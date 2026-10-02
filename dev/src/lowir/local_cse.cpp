#include "lowir/folding.h"
#include <algorithm>
namespace lowir_model {
namespace {
std::uint64_t mix(std::uint64_t h, std::uint64_t n) {
    n ^= n>>30; n *= 0xbf58476d1ce4e5b9ULL; n ^= n>>27; n *= 0x94d049bb133111ebULL;
    return (h^(n^(n>>31)))*1099511628211ULL;
}
std::uint64_t key(const Program& p, const Instruction& i) {
    std::uint64_t h = mix(unsigned(i.opcode),unsigned(i.operation));
    h = mix(h,i.type.kind()); h = mix(h,i.type.width()); h = mix(h,i.type.alignment());
    h = mix(h,unsigned(i.projection));
    for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
        auto a = p.operands[n]; h = mix(h,a.kind);
        if (a.kind == Operand::Integer) { h = mix(h,a.data.integer); h = mix(h,a.integer_high()); }
        else h = mix(h,a.ref);
    }
    return h;
}
bool equal(const Program& p, const Instruction& a, const Instruction& b) {
    if (a.opcode != b.opcode || a.operation != b.operation || a.type != b.type ||
        a.source_type != b.source_type || a.projection != b.projection || a.operands.count != b.operands.count) return false;
    for (unsigned n = 0; n < a.operands.count; ++n)
        if (!same_scalar(p.operands[a.operands.begin+n],p.operands[b.operands.begin+n])) return false;
    return true;
}
bool less(const Program& p, Operand a, Operand b) {
    if (a.literal() != b.literal()) return !a.literal();
    if (a.kind == Operand::Temporary && b.kind == Operand::Temporary &&
        p.values[a.ref-1].definition != p.values[b.ref-1].definition)
        return p.values[a.ref-1].definition < p.values[b.ref-1].definition;
    if (a.kind != b.kind) return a.kind < b.kind;
    if (a.kind != Operand::Integer) return a.ref < b.ref;
    return a.integer_high() != b.integer_high() ? a.integer_high() < b.integer_high() : a.data.integer < b.data.integer;
}
void canonicalize(Program& p, Instruction& i) {
    if (i.operands.count != 2) return;
    bool commute = i.operation == Operation::Add || i.operation == Operation::Mul || i.operation == Operation::And ||
        i.operation == Operation::Or || i.operation == Operation::Xor || i.operation == Operation::Eq || i.operation == Operation::Ne;
    bool swap = false;
    if (i.operation == Operation::Gt) { i.operation = Operation::Lt; swap = true; }
    if (i.operation == Operation::Ge) { i.operation = Operation::Le; swap = true; }
    if (i.operation == Operation::Ugt) { i.operation = Operation::Ult; swap = true; }
    if (i.operation == Operation::Uge) { i.operation = Operation::Ule; swap = true; }
    auto& a = p.operands[i.operands.begin]; auto& b = p.operands[i.operands.begin+1];
    if (swap || (commute && less(p,b,a))) std::swap(a,b);
}
}
void eliminate_local_expressions(Program& p, std::uint64_t& work)
{
    std::vector<unsigned> definitions(p.values.size()+1);
    for (const auto& a : p.parameters) ++definitions[a.value.index];
    for (const auto& i : p.instructions) if (i.destination) ++definitions[i.destination.index];
    for (auto bid : p.block_order) {
        auto r = p.blocks[bid.index-1].instructions;
        unsigned size = 8; while (size < 2*r.count) size *= 2;
        std::vector<unsigned> table(size); std::uint64_t budget = 16*(std::uint64_t(r.count)+1);
        for (unsigned n = r.begin; n < r.end() && budget; ++n) {
            auto& i = p.instructions[n]; ++work; --budget;
            bool eligible = i.opcode == Opcode::Addr || i.opcode == Opcode::Index ||
                ((i.opcode == Opcode::Binary || i.opcode == Opcode::Unary || i.opcode == Opcode::Compare || i.opcode == Opcode::Convert) && discardable(i));
            if (!eligible || definitions[i.destination.index] != 1) continue;
            for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) {
                auto a = p.operands[j];
                eligible &= a.kind != Operand::Floating && (a.kind != Operand::Temporary || definitions[a.ref] == 1);
            }
            if (!eligible) continue;
            canonicalize(p,i); unsigned bucket = key(p,i)&(size-1);
            while (table[bucket] && budget) {
                --budget; ++work;
                const auto& previous = p.instructions[table[bucket]-1];
                if (equal(p,i,previous)) {
                    p.operands[i.operands.begin] = Operand::value(previous.destination);
                    i.type = i.result_type(); i.opcode = Opcode::Copy; i.source_type = Type();
                    i.operation = Operation::None; i.operands.count = 1; break;
                }
                bucket = (bucket+1)&(size-1);
            }
            if (!table[bucket]) table[bucket] = n+1;
        }
    }
}
}
