#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::member_pointer_value(EntityId member, TypeId target, std::int64_t adjustment)
{
    if (!member) { Value null(Operand::integer(0),type(target),target); null.member_zero_adjustment = true; return null; }
    Value result;
    if (sem.types[sem.types[target].child].kind != TypeKind::Function)
        result = emit(Opcode::Const,IRType::I64,{Operand::integer(sem.entities[member].member_offset+1+adjustment)});
    else {
        result = emit(Opcode::Addr,IRType(),{Operand::symbol(member_function_symbol(member))});
        result = emit(Opcode::Copy,IRType::I64,{result.operand});
        result = coerce(result,IRType::I128,true,true);
        if (adjustment) {
            auto high = coerce(Value(Operand::integer(adjustment),IRType::I64),IRType::I128,true,true);
            high = emit(Opcode::Binary,IRType::I128,{high.operand,Operand::integer(64)},Operation::Shl);
            result = emit(Opcode::Binary,IRType::I128,{result.operand,high.operand},Operation::Or);
        }
    }
    result.type = target; result.member_zero_adjustment = !adjustment; return result;
}
Value Procedural::truth_operand(Value value)
{
    if (sem.types[value.type].kind == TypeKind::MemberPointer && value.ir == IRType::I128 && !value.member_zero_adjustment)
        return coerce(value,IRType::I64,true,true);
    return value;
}
void Procedural::member_pointer_data(const semantic::StaticValue& value)
{
    lowir_model::DataItem item; item.type = IRType::Ptr;
    item.kind = value.entity ? lowir_model::DataItem::Address : lowir_model::DataItem::Scalar;
    if (value.entity) item.symbol = member_function_symbol(value.entity);
    else item.value = Operand::integer(0);
    p.data.push_back(item);
    item = lowir_model::DataItem(); item.kind = lowir_model::DataItem::Scalar;
    item.type = IRType::I64; item.value = Operand::integer(value.addend); p.data.push_back(item);
}
Value Procedural::member_pointer_conversion(Value value, const semantic::Conversion& conversion)
{
    auto offset = sem.base_adjustments[conversion.adjustment].total;
    value.type = conversion.target;
    if (!offset) return value;
    // Preserve canonical null even when adding a nonzero base displacement.
    auto slot = builder->add_slot(0,value.ir);
    auto test = emit(Opcode::Compare,value.ir,{value.operand,Operand::integer(0)},Operation::Eq);
    auto null = block(), adjust = block(), end = block();
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(null),Operand::label(adjust)});
    start(null); emit(Opcode::Store,value.ir,{Operand::integer(0),Operand::slot(slot)}); jump(end);
    start(adjust);
    Value delta(Operand::integer(offset),IRType::I64);
    if (value.ir == IRType::I128) {
        delta = coerce(delta,IRType::I128,true,true);
        delta = emit(Opcode::Binary,IRType::I128,{delta.operand,Operand::integer(64)},Operation::Shl);
    }
    auto result = emit(Opcode::Binary,value.ir,{value.operand,delta.operand},Operation::Add);
    emit(Opcode::Store,value.ir,{result.operand,Operand::slot(slot)}); jump(end);
    start(end); result = emit(Opcode::Load,value.ir,{Operand::slot(slot)});
    result.type = conversion.target; return result;
}
Value Procedural::member_pointer_object(const semantic::ObjectUse& use, Value* function)
{
    Value object = expression(use.node,true);
    if (use.invoke_dereference) object = address(range_operation(sem.invoke_dereference(use),{object}));
    else object = sem.types[sem.expression_fact(use.node).type].kind == TypeKind::Pointer ? load(object) : address(object);
    object = base_projection(object,use.adjustment);
    if (function && use.member_target) {
        auto target = sem.member_target_value(use.member_target);
        if (target.addend) object = emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(target.addend)});
        return object;
    }
    Value member = load(expression(use.member_pointer));
    if (!function) {
        auto offset = emit(Opcode::Binary,IRType::I64,{member.operand,Operand::integer(1)},Operation::Sub);
        Instruction index(Opcode::Index,IRType::I8); index.projection = ir_model::IPK_FIELD;
        return emit(index,{object.operand,offset.operand});
    }
    *function = coerce(member,IRType::I64,true,true);
    *function = emit(Opcode::Copy,IRType::Ptr,{function->operand});
    if (!member.member_zero_adjustment) {
        auto high = emit(Opcode::Binary,IRType::I128,{member.operand,Operand::integer(64)},Operation::Shr);
        auto adjustment = coerce(high,IRType::I64,true,true);
        object = emit(Opcode::Index,IRType::I8,{object.operand,adjustment.operand});
    }
    return object;
}
} }
