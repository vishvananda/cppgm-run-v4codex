#include "syntax/parser.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
std::string Parser::assembly_string()
{
    if (in.peek().kind != PostTokenKind::literal) throw std::runtime_error("asm requires a string literal");
    auto token = in.take(); const auto& value = ast.literals[token.literal];
    if (value.kind != LiteralKind::string || value.type != FT_CHAR || value.suffix || !value.bytes)
        throw std::runtime_error("asm requires a narrow string");
    std::string result(ast.literal_bytes.data()+value.offset,value.bytes-1);
    if (result.find('\0') != std::string::npos) throw std::runtime_error("null in asm string");
    return result;
}
NodeId Parser::assembly()
{
    auto result = leaf(Kind::Assembly);
    while (in.is("volatile") || in.is("__volatile") || in.is("__volatile__") || in.is("inline") || in.is("__inline__")) {
        ast[result].flags |= 1; in.take();
    }
    in.require("("); auto source = assembly_string();
    AsmPlan plan; std::vector<AsmOperandName> operands;
    unsigned section = 0;
    // The C++ lexer combines adjacent colons into ::. Both tokens contribute
    // separators here, including the hosted spelling "" ::: "memory".
    unsigned pending = 0;
    auto colon = [&]() {
        if (pending) { --pending; return true; }
        if (in.eat(":")) return true;
        if (in.eat("::")) { pending = 1; return true; }
        return false;
    };
    while (colon()) {
        plan.extended = true;
        if (++section > 3) throw std::runtime_error("asm goto is not supported");
        if (pending || in.is(":") || in.is("::") || in.is(")")) continue;
        do {
            if (section == 3) {
                auto clobber = assembly_string();
                if (clobber == "memory") plan.memory = true;
                else if (clobber != "cc") throw std::runtime_error("unsupported asm register clobber");
                continue;
            }
            AsmOperandName operand;
            if (in.eat("[")) {
                if (!identifier()) throw std::runtime_error("invalid asm operand name");
                auto spelling = ids.spelling(in.take().text);
                operand.name.assign(spelling.data,spelling.size); in.require("]");
                for (const auto& prior : operands) if (prior.name == operand.name) throw std::runtime_error("duplicate asm operand name");
            }
            auto constraint = assembly_string(); unsigned i = 0;
            if (section == 1) {
                if (constraint.empty() || (constraint[0] != '=' && constraint[0] != '+')) throw std::runtime_error("asm output requires = or +");
                operand.flags = AsmOutput | (constraint[0] == '+' ? AsmRead : 0); ++i;
                if (i < constraint.size() && constraint[i] == '&') { operand.flags |= AsmEarly; ++i; }
            } else operand.flags = AsmRead;
            if (section == 2 && !constraint.empty() && (constraint[0] == '[' || (constraint[0] >= '0' && constraint[0] <= '9'))) {
                unsigned match = 0;
                if (constraint[0] == '[' && constraint.back() == ']') {
                    auto name = constraint.substr(1,constraint.size()-2); match = operands.size();
                    for (unsigned j = 0; j < operands.size(); ++j) if (operands[j].name == name) match = j;
                    i = constraint.size();
                } else {
                    for (; i < constraint.size() && constraint[i] >= '0' && constraint[i] <= '9'; ++i) {
                        match = match*10+constraint[i]-'0';
                        if (match >= operands.size()) throw std::runtime_error("invalid asm matching constraint");
                    }
                }
                if (match >= operands.size() || !(operands[match].flags & AsmOutput) || !(operands[match].flags & AsmRegister)) throw std::runtime_error("asm match requires register output");
                for (const auto& prior : operands) if (prior.match == match+1) throw std::runtime_error("duplicate matching input");
                operand.match = match+1; operand.flags |= AsmRegister;
            } else for (; i < constraint.size(); ++i) {
                if (constraint[i] == 'r' || constraint[i] == 'q') operand.flags |= AsmRegister;
                else if (constraint[i] == 'm') operand.flags |= AsmMemory;
                else if (section == 2 && (constraint[i] == 'i' || constraint[i] == 'n')) operand.flags |= AsmImmediate;
                else throw std::runtime_error("unsupported asm constraint");
            }
            if (i != constraint.size() || !(operand.flags & (AsmMemory|AsmRegister|AsmImmediate))) throw std::runtime_error("invalid asm constraint");
            // Choosing memory for a mixed register/memory alternative is legal.
            if (operand.flags & AsmMemory) operand.flags &= ~AsmRegister;
            if (operand.flags & AsmRegister) operand.flags &= ~AsmImmediate;
            auto node = make(Kind::AssemblyOperand); ast[node].flags = operand.flags; ast[node].literal = operand.match;
            in.require("("); ast.append(node,expression()); in.require(")"); ast.append(result,node);
            operands.push_back(operand);
            if (operands.size() > 30) throw std::runtime_error("too many asm operands");
        } while (in.eat(","));
    }
    in.require(")"); in.require(";");
    plan.begin = ast.assembly_instructions.size();
    parse_assembly_template(source,operands,ast.assembly_instructions);
    plan.count = ast.assembly_instructions.size()-plan.begin;
    ast[result].literal = ast.assemblies.size(); ast.assemblies.push_back(plan);
    return result;
}
} }
