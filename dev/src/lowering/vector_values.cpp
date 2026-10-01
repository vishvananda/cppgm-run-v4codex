#include "lowering/procedural.h"
#include "support/builtin_registry.h"
namespace cppgm { namespace lowering {
Value Procedural::vector_lane(Value value, Value lane)
{
    auto child = sem.types[value.type].child;
    auto t = sem.types.qualify(child,sem.types[value.type].cv);
    bool boolean = sem.types[child].fundamental == FT_BOOL;
    value.address = true; auto base = address(value);
    auto offset = emit(Opcode::Binary,IRType::I64,{lane.operand,Operand::integer(boolean ? 3 : sem.object_size(t))},boolean ? Operation::Ushr : Operation::Mul);
    base = emit(Opcode::Index,IRType::I8,{base.operand,offset.operand});
    base.type = boolean ? sem.types.qualify(sem.types.fundamental(FT_UNSIGNED_CHAR),sem.types[value.type].cv) : t;
    base.ir = type(base.type); base.address = true; return base;
}
Value Procedural::vector_read(Value value, Value lane)
{
    auto result = load(vector_lane(value,lane));
    auto t = sem.types[value.type].child;
    if (sem.types[t].fundamental != FT_BOOL) return result;
    auto shift = emit(Opcode::Binary,IRType::I64,{lane.operand,Operand::integer(7)},Operation::And);
    result = emit(Opcode::Binary,IRType::U8,{result.operand,shift.operand},Operation::Ushr);
    result = emit(Opcode::Binary,IRType::U8,{result.operand,Operand::integer(1)},Operation::And);
    result.type = t; return result;
}
void Procedural::vector_write(Value value, Value lane, Value source)
{
    auto location = vector_lane(value,lane);
    if (sem.types[sem.types[value.type].child].fundamental == FT_BOOL) {
        auto old = load(location);
        auto shift = emit(Opcode::Binary,IRType::I64,{lane.operand,Operand::integer(7)},Operation::And);
        auto mask = emit(Opcode::Binary,IRType::U8,{Operand::integer(1),shift.operand},Operation::Shl);
        mask = emit(Opcode::Binary,IRType::U8,{mask.operand,Operand::integer(255)},Operation::Xor);
        old = emit(Opcode::Binary,IRType::U8,{old.operand,mask.operand},Operation::And);
        auto bits = emit(Opcode::Binary,IRType::U8,{source.operand,shift.operand},Operation::Shl);
        source = emit(Opcode::Binary,IRType::U8,{old.operand,bits.operand},Operation::Or); source.type = location.type;
    }
    store(source,location);
}
Value Procedural::vector_operation(ETokenType op, Value a, Value b, TypeId result)
{
    Value value(Operand::slot(builder->add_slot(0,type(result))),type(result),result,true);
    zero_object(result,address(value));
    auto count = sem.object_size(a.type)*8;
    auto t = sem.types[a.type]; count = t.kind == TypeKind::ExtVector ? t.bound : t.bound/sem.object_size(t.child);
    auto lane_type = sem.types[result].child;
    bool compare = op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    vector_each(count,[&](Value lane) {
        auto x = vector_read(a,lane), y = vector_read(b,lane);
        auto v = operation(op,x,y,compare ? sem.types.fundamental(FT_BOOL) : lane_type);
        if (compare) { v = convert(v,lane_type); v = emit(Opcode::Unary,type(lane_type),{v.operand},Operation::Neg); v.type = lane_type; }
        vector_write(value,lane,v);
    });
    value.address = false; return value;
}
Value Procedural::builtin_value(NodeId n)
{
    auto node = ast[n]; auto fact = sem.expression_fact(n);
    auto source = converted(node.first,sem.conversion_fact(fact.conversions));
    auto kind = ValueBuiltin(node.flags);
    if (kind == ValueBuiltin::BitCast) {
        Value object(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
        auto slot = builder->add_slot(0,type(source.type));
        Value stored(Operand::slot(slot),type(source.type),source.type,true);
        store(source,stored);
        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(fact.type); copy.alignment = 1;
        emit(copy,{address(stored).operand,address(object).operand}); return load(object);
    }
    auto t = sem.types[source.type]; auto count = t.kind == TypeKind::ExtVector ? t.bound : t.bound/sem.object_size(t.child);
    if (kind == ValueBuiltin::ReduceOr) {
        Value result(Operand::integer(0),type(fact.type),fact.type);
        SlotId accumulator;
        if (count > 8) {
            accumulator = builder->add_slot(0,type(fact.type));
            emit(Opcode::Store,type(fact.type),{result.operand,Operand::slot(accumulator)});
        }
        vector_each(count,[&](Value lane) {
            if (accumulator) result = emit(Opcode::Load,type(fact.type),{Operand::slot(accumulator)});
            auto next = vector_read(source,lane);
            result = emit(Opcode::Binary,type(fact.type),{result.operand,next.operand},Operation::Or); result.type = fact.type;
            if (accumulator) emit(Opcode::Store,type(fact.type),{result.operand,Operand::slot(accumulator)});
        });
        if (accumulator) { result = emit(Opcode::Load,type(fact.type),{Operand::slot(accumulator)}); result.type = fact.type; }
        return result;
    }
    Value result(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
    zero_object(fact.type,address(result));
    vector_each(count,[&](Value lane) {
        vector_write(result,lane,convert(vector_read(source,lane),sem.types[fact.type].child));
    });
    result.address = false; return result;
}
} }
