#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
IRType Procedural::atomic_representation(TypeId t)
{
    auto bytes = sem.object_size(t);
    if (!bytes || bytes > 16 || (bytes & (bytes-1))) throw std::runtime_error("unsupported atomic object width");
    return bytes == 1 ? IRType::U8 : bytes == 2 ? IRType::U16 : bytes == 4 ? IRType::U32 : bytes == 8 ? IRType::I64 : IRType::I128;
}
Value Procedural::atomic_bits(Value value, IRType raw)
{
    if (value.ir.kind() == IRType::Object) {
        if (value.ir.bytes() == raw.bytes()) return emit(Opcode::Load,raw,{value.operand});
        auto slot = Operand::slot(builder->add_slot(0,raw));
        emit(Opcode::Store,raw,{Operand::integer(0),slot});
        Instruction copy(Opcode::CopyObject); copy.bytes = value.ir.bytes(); copy.alignment = 1;
        emit(copy,{address(Value(value.operand,value.ir,0,true)).operand,address(Value(slot,raw,0,true)).operand});
        return emit(Opcode::Load,raw,{slot});
    }
    if (value.ir.floating()) {
        auto slot = Operand::slot(builder->add_slot(0,raw));
        emit(Opcode::Store,raw,{Operand::integer(0),slot});
        emit(Opcode::Store,value.ir,{value.operand,slot});
        return emit(Opcode::Load,raw,{slot});
    }
    return coerce(value,raw,sem.unsigned_type(value.type));
}
Value Procedural::atomic_value(Value bits, TypeId target)
{
    auto ir = type(target);
    if (ir.kind() == IRType::Object || ir.floating()) {
        auto slot = Operand::slot(builder->add_slot(0,ir));
        atomic_extract(bits,slot,target);
        if (ir.kind() == IRType::Object) return Value(slot,ir,target);
        bits = emit(Opcode::Load,ir,{slot});
    } else bits = coerce(bits,ir);
    bits.type = target; return bits;
}
void Procedural::atomic_extract(Value bits, Operand destination, TypeId target)
{
    auto bytes = sem.object_size(target);
    if (bytes == bits.ir.bytes()) { emit(Opcode::Store,bits.ir,{bits.operand,destination}); return; }
    auto slot = Operand::slot(builder->add_slot(0,bits.ir));
    emit(Opcode::Store,bits.ir,{bits.operand,slot});
    Instruction copy(Opcode::CopyObject); copy.bytes = bytes; copy.alignment = 1;
    emit(copy,{address(Value(slot,bits.ir,0,true)).operand,address(Value(destination,type(target),0,true)).operand});
}
void Procedural::atomic_padding(TypeId t, Value destination)
{
    if (!(sem.types[t].cv & 4)) return;
    auto payload = sem.object_size(sem.types.non_atomic(sem.types.unqualified(t))), total = sem.object_size(t);
    if (payload < total) zero_padding(destination,payload,total-payload,1);
}
Value Procedural::atomic_update(Value location, Value rhs, ETokenType op, TypeId computation, bool postfix)
{
    auto raw = atomic_representation(location.type);
    auto target = sem.types.non_atomic(sem.types.unqualified(location.type));
    auto expected = Operand::slot(builder->add_slot(0,raw));
    auto pointer = address(location).operand;
    auto initial = emit(Opcode::AtomicLoad,raw,{pointer,Operand::integer(5)});
    emit(Opcode::Store,raw,{initial.operand,expected});
    auto expected_pointer = address(Value(expected,raw,0,true)).operand;
    auto retry = block(), done = block(); jump(retry); start(retry);
    auto prior = emit(Opcode::Load,raw,{expected});
    auto old = atomic_value(prior,target);
    auto lhs = convert(old,computation);
    if (op != OP_LSHIFT && op != OP_RSHIFT && lhs.ir != IRType::Ptr && rhs.ir != IRType::Ptr) rhs = convert(rhs,computation);
    auto next = convert(operation(op,lhs,rhs,computation),target);
    auto bits = atomic_bits(next,raw);
    auto success = emit(Opcode::AtomicCompareExchange,raw,{pointer,expected_pointer,bits.operand,Operand::integer(5),Operand::integer(5)});
    emit(Opcode::Branch,IRType(),{success.operand,Operand::label(done),Operand::label(retry)});
    start(done); return postfix ? old : location;
}
} }
