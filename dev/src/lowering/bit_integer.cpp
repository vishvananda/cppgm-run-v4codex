#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::normalize_bit_integer(Value v)
{
    if (!v.type || v.address || v.normalized_integer == v.type) return v;
    auto t = sem.types[v.type];
    if (t.kind != TypeKind::Fundamental || !semantic::bit_integer_kind(t.fundamental)) return v;
    unsigned precision = t.bound, excess = v.ir.width()-precision;
    if (!excess) { v.normalized_integer = v.type; return v; }
    bool unsign = sem.unsigned_type(v.type);
    auto target = v.type;
    if (v.operand.kind == Operand::Integer) {
        unsigned __int128 bits = v.operand.data.integer;
        if (v.ir == IRType::I128) bits |= (unsigned __int128)(v.operand.integer_high()) << 64;
        auto mask = ((unsigned __int128)(1)<<precision)-1;
        bits &= mask;
        if (!unsign && (bits & ((unsigned __int128)(1)<<(precision-1)))) bits |= ~mask;
        v.operand = Operand::integer(std::uint64_t(bits));
        if (v.ir == IRType::I128) v.operand.integer_high(std::uint64_t(bits>>64));
        v.normalized_integer = target; return v;
    }
    v = emit(Opcode::Binary,v.ir,{v.operand,Operand::integer(excess)},Operation::Shl);
    v = emit(Opcode::Binary,v.ir,{v.operand,Operand::integer(excess)},unsign ? Operation::Ushr : Operation::Shr);
    v.type = v.normalized_integer = target; return v;
}
} }
