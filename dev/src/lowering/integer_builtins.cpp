#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::integer_builtin(NodeId n, semantic::Intrinsic kind)
{
    using I = semantic::Intrinsic;
    auto fact = sem.expression_fact(n);
    auto x = converted(sem.call_argument(fact,0),sem.conversion_fact(fact.conversions));
    auto original = x;
    auto ir = x.ir;
    auto binary = [&](Operation op, Value a, Operand b) { return emit(Opcode::Binary,ir,{a.operand,b},op); };
    auto mask = [&](unsigned byte) {
        unsigned __int128 value = 0;
        for (unsigned j = 0; j < ir.width(); j += 8) value |= (unsigned __int128)(byte) << j;
        auto result = Operand::integer(std::uint64_t(value));
        if (ir.width() == 128) result.integer_high(std::uint64_t(value>>64));
        return result;
    };
    auto popcount = [&](Value v) {
        auto half = binary(Operation::Ushr,v,Operand::integer(1));
        half = binary(Operation::And,half,mask(0x55));
        v = binary(Operation::Sub,v,half.operand);
        half = binary(Operation::Ushr,v,Operand::integer(2));
        half = binary(Operation::And,half,mask(0x33));
        v = binary(Operation::And,v,mask(0x33));
        v = binary(Operation::Add,v,half.operand);
        half = binary(Operation::Ushr,v,Operand::integer(4));
        v = binary(Operation::Add,v,half.operand);
        v = binary(Operation::And,v,mask(0x0f));
        for (unsigned shift = 8; shift < ir.width(); shift *= 2) {
            half = binary(Operation::Ushr,v,Operand::integer(shift));
            v = binary(Operation::Add,v,half.operand);
        }
        return binary(Operation::And,v,Operand::integer(255));
    };
    if (kind == I::Bswap) x = emit(Opcode::Unary,ir,{x.operand},Operation::Bswap);
    else if (kind == I::Clz || kind == I::Clzg) {
        for (unsigned shift = 1; shift < ir.width(); shift *= 2) {
            auto shifted = binary(Operation::Ushr,x,Operand::integer(shift));
            x = binary(Operation::Or,x,shifted.operand);
        }
        auto count = popcount(x);
        x = emit(Opcode::Binary,ir,{Operand::integer(ir.width()),count.operand},Operation::Sub);
    } else if (kind == I::Ctz || kind == I::Ctzg || kind == I::Ffs) {
        auto negative = emit(Opcode::Unary,ir,{x.operand},Operation::Neg);
        x = binary(Operation::And,x,negative.operand);
        x = binary(Operation::Sub,x,Operand::integer(1));
        x = popcount(x);
        if (kind == I::Ffs) {
            x = binary(Operation::Add,x,Operand::integer(1));
            auto nonzero = emit(Opcode::Compare,ir,{original.operand,Operand::integer(0)},Operation::Ne);
            x = binary(Operation::Mul,x,coerce(nonzero,ir).operand);
        }
    } else {
        x = popcount(x);
        if (kind == I::Parity) x = binary(Operation::And,x,Operand::integer(1));
    }
    x = coerce(x,type(fact.type));
    if (fact.argument_count == 2) {
        // Both operands are evaluated once; only the zero result selects the fallback.
        auto fallback = converted(sem.call_argument(fact,1),sem.conversion_fact(fact.conversions+1));
        auto zero = coerce(emit(Opcode::Compare,ir,{original.operand,Operand::integer(0)},Operation::Eq),x.ir);
        auto delta = emit(Opcode::Binary,x.ir,{fallback.operand,x.operand},Operation::Sub);
        delta = emit(Opcode::Binary,x.ir,{delta.operand,zero.operand},Operation::Mul);
        x = emit(Opcode::Binary,x.ir,{x.operand,delta.operand},Operation::Add);
    }
    x.type = fact.type; return x;
}
} }
