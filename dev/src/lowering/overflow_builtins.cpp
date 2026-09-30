#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::overflow_builtin(NodeId n, semantic::Intrinsic kind)
{
    using I = semantic::Intrinsic;
    auto fact = sem.expression_fact(n);
    auto arg = [&](unsigned j) { return converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j)); };
    auto a = arg(0), b = arg(1), destination = arg(2);
    auto target = sem.types[destination.type].child;
    auto wide = IRType(IRType::I128);
    auto binary = [&](Operation op, Value x, Value y) { return emit(Opcode::Binary,wide,{x.operand,y.operand},op); };
    auto constant = [&](std::uint64_t bits) { return Value(Operand::integer(bits),wide); };
    auto zero = constant(0), one = constant(1);
    auto compare = [&](Operation op, Value x, Value y) { return coerce(emit(Opcode::Compare,wide,{x.operand,y.operand},op),wide); };
    auto negative = [&](Value x) {
        if (sem.unsigned_type(x.type)) return zero;
        return coerce(emit(Opcode::Compare,x.ir,{x.operand,Operand::integer(0)},Operation::Lt),wide);
    };
    auto sa = negative(a), sb = negative(b);
    auto raw_a = coerce(a,wide,sem.unsigned_type(a.type)), raw_b = coerce(b,wide,sem.unsigned_type(b.type));
    auto high_a = binary(Operation::Sub,zero,sa), high_b = binary(Operation::Sub,zero,sb);
    Value low, high;
    // A signed 256-bit pair holds every mathematical operation on <=128-bit
    // inputs. Width <=64 multiplication needs only one 128-bit product.
    if (kind == I::AddOverflow) {
        low = binary(Operation::Add,raw_a,raw_b);
        high = binary(Operation::Add,binary(Operation::Add,high_a,high_b),compare(Operation::Ult,low,raw_a));
    } else if (kind == I::SubOverflow) {
        low = binary(Operation::Sub,raw_a,raw_b);
        high = binary(Operation::Sub,binary(Operation::Sub,high_a,high_b),compare(Operation::Ult,raw_a,raw_b));
    } else {
        auto magnitude_a = binary(Operation::Add,binary(Operation::Xor,raw_a,high_a),sa);
        auto magnitude_b = binary(Operation::Add,binary(Operation::Xor,raw_b,high_b),sb);
        high = zero;
        if (a.ir.width() <= 64 && b.ir.width() <= 64) low = binary(Operation::Mul,magnitude_a,magnitude_b);
        else {
            auto mask = constant(~std::uint64_t(0));
            auto al = binary(Operation::And,magnitude_a,mask), bl = binary(Operation::And,magnitude_b,mask);
            auto ah = binary(Operation::Ushr,magnitude_a,constant(64)), bh = binary(Operation::Ushr,magnitude_b,constant(64));
            auto p00 = binary(Operation::Mul,al,bl), p01 = binary(Operation::Mul,al,bh);
            auto p10 = binary(Operation::Mul,ah,bl), p11 = binary(Operation::Mul,ah,bh);
            low = binary(Operation::Add,p00,binary(Operation::Shl,p01,constant(64)));
            auto carry = compare(Operation::Ult,low,p00), first = low;
            low = binary(Operation::Add,low,binary(Operation::Shl,p10,constant(64)));
            carry = binary(Operation::Add,carry,compare(Operation::Ult,low,first));
            high = binary(Operation::Add,p11,binary(Operation::Ushr,p01,constant(64)));
            high = binary(Operation::Add,high,binary(Operation::Ushr,p10,constant(64)));
            high = binary(Operation::Add,high,carry);
        }
        auto sign = binary(Operation::Xor,sa,sb), mask = binary(Operation::Sub,zero,sign);
        low = binary(Operation::Add,binary(Operation::Xor,low,mask),sign);
        high = binary(Operation::Add,binary(Operation::Xor,high,mask),binary(Operation::And,sign,compare(Operation::Eq,low,zero)));
    }
    auto stored = coerce(low,type(target)); stored.type = target;
    Instruction write(Opcode::Store,stored.ir); write.is_volatile = sem.types[target].cv & 2;
    emit(write,{stored.operand,destination.operand});
    auto expected = coerce(stored,wide,sem.unsigned_type(target));
    auto expected_high = binary(Operation::Sub,zero,negative(stored));
    auto mismatch = binary(Operation::Or,compare(Operation::Ne,low,expected),compare(Operation::Ne,high,expected_high));
    auto result = coerce(mismatch,type(fact.type)); result.type = fact.type; return result;
}
} }
