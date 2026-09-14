#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::member_pointer_value(EntityId member, TypeId target)
{
    if (sem.types[sem.types[target].child].kind != TypeKind::Function)
        return Value(Operand::integer(member ? sem.entities[member].member_offset : ~std::uint64_t(0)),IRType::I64,target);
    SlotId slot = builder->add_slot(0,type(target));
    Value storage(Operand::slot(slot),type(target),target,true);
    Value pointer = address(storage);
    Value function = member ? emit(Opcode::Addr,IRType(),{Operand::symbol(symbol(member))}) : Value(Operand::integer(0),IRType::Ptr);
    emit(Opcode::Store,IRType::Ptr,{function.operand,pointer.operand});
    Value adjustment = emit(Opcode::Index,IRType::I8,{pointer.operand,Operand::integer(8)});
    emit(Opcode::Store,IRType::I64,{Operand::integer(0),adjustment.operand});
    storage.address = false; return storage;
}
Value Procedural::truth_operand(Value value)
{
    return sem.types[value.type].kind == TypeKind::MemberPointer ? convert(value,sem.types.fundamental(FT_BOOL)) : value;
}
void Procedural::member_pointer_data(const semantic::StaticValue& value)
{
    lowir_model::DataItem item; item.type = IRType::Ptr;
    item.kind = value.entity ? lowir_model::DataItem::Address : lowir_model::DataItem::Scalar;
    if (value.entity) item.symbol = symbol(value.entity);
    else item.value = Operand::integer(0);
    p.data.push_back(item);
    item = lowir_model::DataItem(); item.kind = lowir_model::DataItem::Scalar;
    item.type = IRType::I64; item.value = Operand::integer(0); p.data.push_back(item);
}
Value Procedural::member_pointer_equal(Value left, Value right, bool equal)
{
    auto a = member_pointer_address(left), b = member_pointer_address(right);
    auto af = emit(Opcode::Load,IRType::Ptr,{a.operand}), bf = emit(Opcode::Load,IRType::Ptr,{b.operand});
    auto same = emit(Opcode::Compare,IRType::Ptr,{af.operand,bf.operand},Operation::Eq);
    auto null = emit(Opcode::Compare,IRType::Ptr,{af.operand,Operand::integer(0)},Operation::Eq);
    a = emit(Opcode::Index,IRType::I8,{a.operand,Operand::integer(8)});
    b = emit(Opcode::Index,IRType::I8,{b.operand,Operand::integer(8)});
    a = emit(Opcode::Load,IRType::I64,{a.operand}); b = emit(Opcode::Load,IRType::I64,{b.operand});
    auto adjustment = emit(Opcode::Compare,IRType::I64,{a.operand,b.operand},Operation::Eq);
    auto allowed = emit(Opcode::Binary,IRType::I64,{null.operand,adjustment.operand},Operation::Or);
    auto result = emit(Opcode::Binary,IRType::I64,{same.operand,allowed.operand},Operation::And);
    if (!equal) result = emit(Opcode::Compare,IRType::I64,{result.operand,Operand::integer(0)},Operation::Eq);
    return result;
}
Value Procedural::member_pointer_address(Value value)
{
    if (value.address || value.operand.kind == Operand::Slot || value.operand.kind == Operand::Symbol) {
        value.address = true; return address(value);
    }
    if (value.operand.kind == Operand::Temporary && p.values[value.operand.ref-1].type == IRType::Ptr)
        return Value(value.operand,IRType::Ptr,value.type);
    SlotId slot = builder->add_slot(0,type(value.type));
    Value pointer = address(Value(Operand::slot(slot),type(value.type),value.type,true));
    Instruction copy(Opcode::CopyObject); copy.bytes = 16; copy.alignment = 8;
    emit(copy,{value.operand,pointer.operand}); return pointer;
}
Value Procedural::member_pointer_object(const semantic::ObjectUse& use, Value* function)
{
    Value object = expression(use.node,true);
    object = sem.types[sem.expression_fact(use.node).type].kind == TypeKind::Pointer ? load(object) : address(object);
    object = base_projection(object,use.adjustment);
    Value member = load(expression(use.member_pointer));
    if (!function) return emit(Opcode::Index,IRType::I8,{object.operand,member.operand});
    Value pointer = member_pointer_address(member);
    *function = emit(Opcode::Load,IRType::Ptr,{pointer.operand});
    Value at = emit(Opcode::Index,IRType::I8,{pointer.operand,Operand::integer(8)});
    Value adjustment = emit(Opcode::Load,IRType::I64,{at.operand});
    return emit(Opcode::Index,IRType::I8,{object.operand,adjustment.operand});
}
} }
