#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::member_pointer_value(EntityId member, TypeId target)
{
    if (!member) throw std::logic_error("missing member pointer target");
    if (sem.types[sem.types[target].child].kind != TypeKind::Function)
        return Value(Operand::integer(sem.entities[member].member_offset),IRType::I64,target);
    SlotId slot = builder->add_slot(0,type(target));
    Value storage(Operand::slot(slot),type(target),target,true);
    Value pointer = address(storage);
    Value function = emit(Opcode::Addr,IRType(),{Operand::symbol(symbol(member))});
    emit(Opcode::Store,IRType::Ptr,{function.operand,pointer.operand});
    Value adjustment = emit(Opcode::Index,IRType::I8,{pointer.operand,Operand::integer(8)});
    emit(Opcode::Store,IRType::I64,{Operand::integer(0),adjustment.operand});
    storage.address = false; return storage;
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
