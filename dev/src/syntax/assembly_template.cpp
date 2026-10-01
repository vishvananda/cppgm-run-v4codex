#include "syntax/assembly.h"
#include <cctype>
#include <stdexcept>
namespace cppgm { namespace syntax {
namespace {
struct TemplateParser {
    const std::string& text;
    const std::vector<AsmOperandName>& operands;
    std::size_t pos = 0;
    TemplateParser(const std::string& s, const std::vector<AsmOperandName>& o) : text(s), operands(o) {}
    bool end() const { return pos == text.size(); }
    void space() { while (!end() && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\r')) ++pos; }
    void separators() { while (!end() && (std::isspace((unsigned char)text[pos]) || text[pos] == ';')) ++pos; }
    bool eat(char c) { if (!end() && text[pos] == c) { ++pos; return true; } return false; }
    std::string word() {
        space(); auto begin = pos;
        while (!end() && (std::isalnum((unsigned char)text[pos]) || text[pos] == '_')) ++pos;
        return text.substr(begin,pos-begin);
    }
    unsigned operand() {
        space(); if (!eat('%')) throw std::runtime_error("asm requires an operand reference");
        unsigned ordinal = operands.size();
        if (eat('[')) {
            auto name = word(); if (!eat(']')) throw std::runtime_error("invalid named asm operand");
            for (unsigned i = 0; i < operands.size(); ++i) if (operands[i].name == name) ordinal = i;
        } else {
            auto number = word(); ordinal = 0;
            if (number.empty()) throw std::runtime_error("missing asm operand ordinal");
            for (char c : number) {
                if (c < '0' || c > '9' || ordinal > 30) throw std::runtime_error("unsupported asm operand modifier");
                ordinal = ordinal*10+c-'0';
            }
        }
        if (ordinal >= operands.size()) throw std::runtime_error("unknown asm operand");
        return operands[ordinal].match ? operands[ordinal].match-1 : ordinal;
    }
    std::uint64_t immediate() {
        space(); bool negative = eat('-'); if (!negative) eat('+');
        unsigned base = 10; if (pos+2 <= text.size() && text[pos] == '0' && text[pos+1] == 'x') { pos += 2; base = 16; }
        std::uint64_t value = 0; unsigned digits = 0;
        while (!end()) {
            char c = text[pos]; unsigned d = c >= '0' && c <= '9' ? c-'0' : c >= 'a' && c <= 'f' ? c-'a'+10 : 99;
            if (d >= base) break;
            if (value > (~std::uint64_t(0)-d)/base) throw std::runtime_error("asm immediate overflow");
            value = value*base+d; ++digits; ++pos;
        }
        if (!digits) throw std::runtime_error("invalid asm immediate");
        return negative ? -value : value;
    }
};
}
void parse_assembly_template(const std::string& source, const std::vector<AsmOperandName>& operands,
    std::vector<AsmInstruction>& instructions)
{
    TemplateParser p(source,operands);
    bool initialized[30] = {};
    for (unsigned i = 0; i < operands.size(); ++i) {
        initialized[i] = operands[i].flags & AsmRead;
        if (operands[i].match) initialized[operands[i].match-1] = true;
    }
    p.separators();
    while (!p.end()) {
        AsmInstruction instruction; auto mnemonic = p.word();
        if (mnemonic == "lock") { instruction.locked = true; p.separators(); mnemonic = p.word(); }
        if (mnemonic == "rep" || mnemonic == "repe") {
            p.separators(); if (p.word() != "nop") throw std::runtime_error("unsupported asm repeat prefix");
            mnemonic = "pause";
        }
        struct Entry { const char* name; AsmOp op; unsigned arity; };
        static const Entry entries[] = {
            {"nop",AsmOp::Nop,0},{"pause",AsmOp::Pause,0},{"mfence",AsmOp::Fence,0},
            {"mov",AsmOp::Move,2},{"add",AsmOp::Add,2},{"sub",AsmOp::Sub,2},
            {"and",AsmOp::And,2},{"or",AsmOp::Or,2},{"xor",AsmOp::Xor,2},
            {"inc",AsmOp::Inc,1},{"dec",AsmOp::Dec,1},{"not",AsmOp::Not,1},
            {"neg",AsmOp::Neg,1},{"bswap",AsmOp::Bswap,1},{"xchg",AsmOp::Exchange,2},{"xadd",AsmOp::Xadd,2}
        };
        const Entry* found = nullptr;
        for (const auto& e : entries) {
            if (mnemonic == e.name) { found = &e; break; }
            if (e.arity && mnemonic.size() == std::string(e.name).size()+1 && mnemonic.compare(0,mnemonic.size()-1,e.name) == 0) {
                char suffix = mnemonic.back();
                instruction.width = suffix == 'b' ? 1 : suffix == 'w' ? 2 : suffix == 'l' ? 4 : suffix == 'q' ? 8 : 0;
                if (instruction.width) { found = &e; break; }
            }
        }
        if (!found) throw std::runtime_error("unsupported asm instruction: " + mnemonic);
        instruction.op = found->op;
        if (found->arity == 2) {
            p.space(); instruction.immediate = p.eat('$');
            if (instruction.immediate) instruction.value = p.immediate();
            else instruction.source = p.operand();
            p.space(); if (!p.eat(',')) throw std::runtime_error("asm requires two operands");
        }
        if (found->arity) {
            instruction.destination = p.operand();
            if (instruction.op == AsmOp::Exchange && !instruction.immediate && (operands[instruction.source].flags & AsmMemory))
                std::swap(instruction.destination,instruction.source);
            auto d = instruction.destination;
            if (!(operands[d].flags & AsmOutput)) throw std::runtime_error("asm instruction writes an input");
            if (instruction.op != AsmOp::Move && !initialized[d]) throw std::runtime_error("asm reads an uninitialized output");
            if (found->arity == 2 && !instruction.immediate && !initialized[instruction.source]) throw std::runtime_error("asm reads an uninitialized source");
            if (found->arity == 2 && !instruction.immediate && (operands[d].flags & AsmMemory) && (operands[instruction.source].flags & AsmMemory)) throw std::runtime_error("asm has two memory operands");
            if (instruction.op == AsmOp::Exchange || instruction.op == AsmOp::Xadd) {
                if (instruction.immediate || !(operands[instruction.source].flags & AsmOutput) || !(operands[instruction.source].flags & AsmRegister)) throw std::runtime_error("asm exchange requires a read/write register source");
            }
            if (instruction.op == AsmOp::Bswap && !(operands[d].flags & AsmRegister)) throw std::runtime_error("bswap requires a register");
            initialized[d] = true;
        }
        if (instruction.locked && (!found->arity || !(operands[instruction.destination].flags & AsmMemory) || instruction.op == AsmOp::Move || instruction.op == AsmOp::Bswap)) throw std::runtime_error("invalid asm lock prefix");
        p.space();
        if (!p.end() && source[p.pos] != ';' && source[p.pos] != '\n') throw std::runtime_error("trailing asm operands");
        instructions.push_back(instruction); p.separators();
    }
    for (unsigned i = 0; i < operands.size(); ++i)
        if ((operands[i].flags & AsmOutput) && !initialized[i]) throw std::runtime_error("asm output is never initialized");
}
} }
