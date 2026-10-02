#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::packed_call(NodeId n)
{
    using P = PackedOp;
    auto fact = sem.expression_fact(n);
    auto d = sem.packed_intrinsic(sem.facts[n].entity);
    auto arg = [&](unsigned j) { return converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j)); };
    auto a = arg(0), b = arg(1);
    Value result(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
    auto c = [&](std::uint64_t x) { return Value(Operand::integer(x),IRType::I64); };
    auto bin = [&](Operation op, Value x, Value y) { return emit(Opcode::Binary,IRType::I64,{x.operand,y.operand},op); };
    auto cmp = [&](Operation op, Value x, Value y) { return coerce(emit(Opcode::Compare,IRType::I64,{x.operand,y.operand},op),IRType::I64); };
    auto choose = [&](Value test, Value yes, Value no) {
        auto mask = bin(Operation::Sub,c(0),test);
        return bin(Operation::Xor,no,bin(Operation::And,mask,bin(Operation::Xor,yes,no)));
    };
    auto clamp = [&](Value x, std::int64_t low, std::int64_t high) {
        x = choose(cmp(Operation::Lt,x,c(low)),c(low),x);
        return choose(cmp(Operation::Gt,x,c(high)),c(high),x);
    };
    auto lane = [&](Value v, unsigned i, bool unsign = false) { return coerce(vector_read(v,c(i)),IRType::I64,unsign); };
    auto put = [&](unsigned i, Value x) {
        auto t = sem.types[fact.type].child; x = coerce(x,type(t)); x.type = t; vector_write(result,i,x);
    };
    bool shift = d.op == P::ShiftLeft || d.op == P::ShiftRight || d.op == P::ShiftArithmetic;
    Value count;
    if (shift) {
        if (d.immediate) count = coerce(b,IRType::I64,true);
        else { b.address = true; count = emit(Opcode::Load,IRType::I64,{address(b).operand}); }
    }
    unsigned lanes = d.bytes/d.lane;
    for (unsigned i = 0; i < d.bytes/d.result_lane; ++i) {
        if (d.op == P::PackSigned || d.op == P::PackUnsigned) {
            auto x = lane(i < lanes ? a : b,i % lanes);
            auto bits = d.result_lane*8;
            put(i,clamp(x,d.op == P::PackUnsigned ? 0 : -(std::int64_t(1)<<(bits-1)),
                (std::int64_t(1)<<(d.op == P::PackUnsigned ? bits : bits-1))-1)); continue;
        }
        if (d.op == P::UnpackLow || d.op == P::UnpackHigh) {
            put(i,lane(i%2 ? b : a,i/2+(d.op == P::UnpackHigh ? lanes/2 : 0))); continue;
        }
        if (d.op == P::MultiplyAdd) {
            put(i,bin(Operation::Add,bin(Operation::Mul,lane(a,i*2),lane(b,i*2)),
                bin(Operation::Mul,lane(a,i*2+1),lane(b,i*2+1)))); continue;
        }
        bool unsign = d.op == P::AddUnsigned || d.op == P::SubUnsigned || d.op == P::ShiftRight;
        auto x = lane(a,i,unsign); Value y = shift ? count : lane(b,i,unsign), v;
        switch (d.op) {
        case P::Add: case P::AddSigned: case P::AddUnsigned: v = bin(Operation::Add,x,y); break;
        case P::Sub: case P::SubSigned: case P::SubUnsigned: v = bin(Operation::Sub,x,y); break;
        case P::MultiplyLow: case P::MultiplyHigh:
            v = bin(Operation::Mul,x,y);
            if (d.op == P::MultiplyHigh) v = bin(Operation::Shr,v,c(d.lane*8));
            break;
        case P::Equal: case P::Greater: v = bin(Operation::Sub,c(0),cmp(d.op == P::Equal ? Operation::Eq : Operation::Gt,x,y)); break;
        case P::And: v = bin(Operation::And,x,y); break;
        case P::AndNot: v = bin(Operation::And,bin(Operation::Xor,x,c(~std::uint64_t(0))),y); break;
        case P::Or: v = bin(Operation::Or,x,y); break;
        case P::Xor: v = bin(Operation::Xor,x,y); break;
        case P::ShiftLeft: case P::ShiftRight: case P::ShiftArithmetic: {
            auto over = cmp(Operation::Uge,y,c(d.lane*8));
            // Clamp first: no LowIR shift ever sees a count outside its width.
            auto safe = choose(over,c(d.lane*8-1),y);
            v = bin(d.op == P::ShiftLeft ? Operation::Shl : d.op == P::ShiftRight ? Operation::Ushr : Operation::Shr,x,safe);
            if (d.op != P::ShiftArithmetic) v = choose(over,c(0),v);
            break;
        }
        default: throw std::logic_error("unhandled packed operation");
        }
        if (d.op == P::AddSigned || d.op == P::SubSigned || d.op == P::AddUnsigned || d.op == P::SubUnsigned) {
            auto bits = d.lane*8;
            v = clamp(v,unsign ? 0 : -(std::int64_t(1)<<(bits-1)),(std::int64_t(1)<<(unsign ? bits : bits-1))-1);
        }
        put(i,v);
    }
    result.address = false; return result;
}
} }
