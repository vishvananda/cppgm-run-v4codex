#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::transfer_array(const semantic::TransferAction& action, Value source, Value target, bool assignment, bool member_root)
{
    // Bound static expansion across all dimensions, matching ordinary array
    // construction. Larger transfers use counted loops with constant-size IR.
    std::uint64_t elements = 1;
    TypeId leaf = action.type;
    while (sem.types[leaf].kind == TypeKind::Array) { elements *= sem.types[leaf].bound; leaf = sem.types[leaf].child; }
    std::vector<InitProjection> path;
    if (member_root) path.push_back({action.field ? sem.entities[action.field].member_offset : 0, action.field != 0, action.field});
    auto one = [&](Operand index) {
        Value dst = array_element(target, member_root, path, index, sem.object_size(leaf));
        Value src = array_element(source, member_root, path, index, sem.object_size(leaf));
        auto element = action; element.type = leaf;
        transfer_action(element, src, dst, assignment);
    };
    if (elements <= 8) { for (std::uint64_t j = 0; j < elements; ++j) one(Operand::integer(j)); return; }
    SlotId cursor = builder->add_slot(0, IRType::I64);
    emit(Opcode::Store, IRType::I64, {Operand::integer(0), Operand::slot(cursor)});
    BlockId test = block(), body = block(), end = block(); jump(test); start(test);
    Value index = emit(Opcode::Load, IRType::I64, {Operand::slot(cursor)});
    Value more = emit(Opcode::Compare, IRType::I64, {index.operand, Operand::integer(elements)}, Operation::Ult);
    emit(Opcode::Branch, IRType(), {more.operand, Operand::label(body), Operand::label(end)});
    start(body); one(index.operand);
    Value next = emit(Opcode::Binary, IRType::I64, {index.operand, Operand::integer(1)}, Operation::Add);
    emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(cursor)}); jump(test); start(end);
}
void Procedural::transfer_action(const semantic::TransferAction& action, Value source, Value target, bool assignment)
{
    if (sem.types[action.type].kind == TypeKind::Array) { transfer_array(action, source, target, assignment); return; }
    if (action.function) {
        if (!assignment && sem.direct_transfer(action.function)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(action.type); copy.alignment = sem.object_alignment(action.type);
            emit(copy, {source.operand, target.operand});
        } else {
            Operand args[] = {Operand::symbol(symbol(action.function, !action.field)), target.operand, source.operand};
            guarded_call(Instruction(Opcode::Call, assignment ? type(sem.types[sem.entities[action.function].type].child) : IRType::Void), args, 3);
        }
        return;
    }
    TypeId t = action.kind == semantic::TransferAction::Reference ? sem.types.compound(TypeKind::Pointer, sem.types.fundamental(FT_VOID)) : action.type;
    Value src(source.operand, type(t), t, true), dst(target.operand, type(t), t, true);
    if (action.kind == semantic::TransferAction::Scalar && sem.field_fact(action.field).bit_field) src.bit_field = dst.bit_field = action.field;
    store(load(src), dst);
}
void Procedural::transfer_body(EntityId e)
{
    auto m = sem.member_fact(e);
    bool assignment = !m.constructor;
    SlotId other = objects[m.transfer_parameter];
    auto project = [&](SlotId slot, const semantic::TransferAction& action) {
        Value root = emit(Opcode::Load, IRType::Ptr, {Operand::slot(slot)});
        Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        return emit(i, {root.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : 0)});
    };
    for (unsigned j = 0; j < m.transfer_count; ++j) {
        auto action = sem.transfers[m.transfer_begin+j];
        if (action.kind == semantic::TransferAction::Empty) continue;
        if (action.kind == semantic::TransferAction::Storage) {
            Value dst = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
            Value src = emit(Opcode::Load, IRType::Ptr, {Operand::slot(other)});
            Instruction copy(Opcode::CopyObject); copy.bytes = action.bytes; copy.alignment = action.alignment;
            emit(copy, {src.operand, dst.operand});
        } else if (sem.types[action.type].kind == TypeKind::Array) {
            transfer_array(action, Value(Operand::slot(other), IRType::Ptr), Value(Operand::slot(this_slot), IRType::Ptr), assignment, true);
        } else if (action.function) {
            Value dst = project(this_slot, action), src = project(other, action);
            transfer_action(action, src, dst, assignment);
        } else if (!assignment && action.kind != semantic::TransferAction::Unit) {
            Value dst = project(this_slot, action), src = project(other, action);
            transfer_action(action, src, dst, assignment);
        } else {
            Value src = project(other, action);
            TypeId t = action.kind == semantic::TransferAction::Reference ? sem.types.compound(TypeKind::Pointer, sem.types.fundamental(FT_VOID)) : action.type;
            src.type = t; src.address = true;
            if (action.kind == semantic::TransferAction::Scalar && sem.field_fact(action.field).bit_field) src.bit_field = action.field;
            Value value = load(src), dst = project(this_slot, action); dst.type = t; dst.address = true; dst.bit_field = src.bit_field;
            store(value, dst);
        }
    }
    if (assignment) {
        Value object = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        emit(Opcode::Return, IRType::Ptr, {object.operand});
    }
}
} }
