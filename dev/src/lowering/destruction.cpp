#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::destroy(EntityId dtor, TypeId t, Value object)
{
    if (!sem.destructor_needed(dtor)) return;
    auto target = sem.types[t];
    if (target.kind == TypeKind::Array) {
        for (std::uint64_t j = target.bound; j; --j) {
            Value at = emit(Opcode::Index, IRType::I8, {object.operand, Operand::integer((j-1)*sem.object_size(target.child))});
            destroy(dtor, target.child, at);
        }
        return;
    }
    Operand args[] = {Operand::symbol(symbol(dtor)), object.operand};
    guarded_call(Instruction(Opcode::Call, IRType::Void), args, 2);
}
void Procedural::destroy_object(EntityId object, EntityId dtor)
{
    destroy(dtor, sem.entities[object].type, address(binding(object)));
}
void Procedural::constructor_cleanup(semantic::SubobjectAction action)
{
    if (!sem.destructor_needed(sem.type_destructor(action.type))) return;
    BlockId handler = block(); constructed_subobjects.push_back({action, handler});
    emit(Opcode::EhCleanup, IRType(), {Operand::label(handler)});
}
void Procedural::finish_constructor_handlers()
{
    for (std::size_t j = 0; j < constructed_subobjects.size(); ++j) emit(Opcode::EhEnd, IRType(), {});
}
void Procedural::destructor_prologue(EntityId e)
{
    auto m = sem.member_fact(e);
    bool needed = false;
    for (unsigned j = 0; j < m.destruction_count; ++j)
        needed |= sem.destructor_needed(sem.destruction_actions[m.destruction_begin+j].destructor);
    if (!needed) return;
    destructor_handler = block(); destructor_end = block();
    emit(Opcode::EhCleanup, IRType(), {Operand::label(destructor_handler)});
}
void Procedural::destroy_subobjects(EntityId e)
{
    auto m = sem.member_fact(e);
    for (unsigned j = 0; j < m.destruction_count; ++j) {
        auto action = sem.destruction_actions[m.destruction_begin+j];
        if (!sem.destructor_needed(action.destructor)) continue;
        Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        Value at = emit(i, {base.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : 0)});
        destroy(action.destructor, action.type, at);
    }
}
void Procedural::destructor_finish(EntityId e)
{
    if (!ended) {
        emit(Opcode::EhEnd, IRType(), {});
        destroy_subobjects(e); jump(destructor_end);
    }
    if (destructor_epilogue) { start(destructor_epilogue); destroy_subobjects(e); jump(destructor_end); }
    start(destructor_handler);
    emitting_cleanup = true; live = 0;
    destroy_subobjects(e);
    emit(Opcode::EhEnd, IRType(), {}); emit(Opcode::Resume, IRType(), {});
    emitting_cleanup = false;
    start(destructor_end); emit(Opcode::Return, IRType(), {});
}
void Procedural::global_finalization()
{
    std::vector<EntityId> work;
    for (EntityId e = 1; e < sem.entities.size(); ++e)
        if (symbols[e] && sem.entities[e].kind == semantic::EntityKind::Variable &&
            !sem.entities[e].external_decl && sem.destructor_needed(sem.object_destructor(e))) work.push_back(e);
    if (work.empty()) return;
    reset_lifetime(0);
    Function f; f.symbol = fresh_symbol("@__cppgm_fini");
    function = FunctionId(p.functions.size()+1);
    auto void_type = sem.types.fundamental(FT_VOID);
    f.signature = signature(sem.types.function(void_type, {}, false), function);
    p.functions.push_back(f);
    auto& symbol = p.symbols[f.symbol.index-1];
    symbol.kind = Symbol::FunctionSymbol; symbol.entity = function.index;
    symbol.metadata.role = SR_FINI; symbol.metadata.binding = SBM_INTERNAL;
    builder.reset(new FunctionBuilder(p, function)); this_slot = SlotId();
    start(block());
    for (auto it = work.rbegin(); it != work.rend(); ++it) destroy_object(*it, sem.object_destructor(*it));
    emit(Opcode::Return, IRType(), {}); builder.reset();
}
} }
