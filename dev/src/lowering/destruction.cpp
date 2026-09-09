#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::destroy(EntityId dtor, TypeId t, Value object)
{
    // Callers select an effectful step or a required object/array boundary.
    if (!dtor) return;
    if (sem.types[t].kind == TypeKind::Array) {
        array_destroy(dtor, t, object, false, {}); return;
    }
    Operand args[] = {Operand::symbol(symbol(dtor)), object.operand};
    guarded_call(Instruction(Opcode::Call, IRType::Void), args, 2);
}
void Procedural::destroy_object(EntityId object, EntityId dtor)
{
    TypeId t = sem.entities[object].type;
    if (sem.parameter_cleanup(object)) {
        Value location = address(binding(object));
        Operand arguments[] = {Operand::symbol(symbol(dtor)),location.operand};
        guarded_call(Instruction(Opcode::Call,IRType::Void),arguments,2); return;
    }
    if (sem.types[t].kind == TypeKind::Array) array_destroy(dtor, t, binding(object), false, {});
    else destroy(dtor, t, address(binding(object)));
}
void Procedural::constructor_cleanup(semantic::SubobjectAction action)
{
    if (sem.function_nonthrowing(active_function) || sem.trivial_destructor(action.type)) return;
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
    std::vector<semantic::DestructionAction> actions;
    for (unsigned j = 0; j < m.destruction_count; ++j) {
        auto action = sem.destruction_actions[m.destruction_begin+j];
        if (sem.destructor_needed(action.destructor)) actions.push_back(action);
    }
    // Preserve the small O0 epilogue form, but bound duplicated cleanup work.
    // Larger classes share one unwind block per remaining subobject.
    const unsigned inline_limit = 8;
    bool shared = !emitting_cleanup && actions.size() > inline_limit;
    std::vector<BlockId> suffix;
    if (shared) {
        suffix.resize(actions.size());
        for (unsigned j = 1; j < actions.size(); ++j) suffix[j] = block();
    }
    for (unsigned j = 0; j < actions.size(); ++j) {
        BlockId cleanup, next;
        if (!emitting_cleanup && j+1 < actions.size()) {
            cleanup = shared ? suffix[j+1] : block(); next = block();
            emit(Opcode::EhCleanup,IRType(),{Operand::label(cleanup)});
        }
        destroy_subobject(actions[j]);
        if (cleanup) {
            emit(Opcode::EhEnd,IRType(),{}); jump(next);
            if (!shared) {
                start(cleanup); emitting_cleanup = true;
                for (unsigned k = j+1; k < actions.size(); ++k) destroy_subobject(actions[k]);
                emit(Opcode::EhEnd,IRType(),{}); emit(Opcode::Resume,IRType(),{});
                emitting_cleanup = false;
            }
            start(next);
        }
    }
    if (!shared) return;
    auto end = block(); jump(end); emitting_cleanup = true;
    for (unsigned j = 1; j < actions.size(); ++j) {
        start(suffix[j]); destroy_subobject(actions[j]);
        if (j+1 < actions.size()) jump(suffix[j+1]);
        else { emit(Opcode::EhEnd,IRType(),{}); emit(Opcode::Resume,IRType(),{}); }
    }
    emitting_cleanup = false; start(end);
}
void Procedural::destroy_subobject(const semantic::DestructionAction& action)
{
    if (sem.types[action.type].kind == TypeKind::Array) {
        array_destroy(action.destructor, action.type, Value(Operand::slot(this_slot), IRType::Ptr), true,
            {{action.field ? sem.entities[action.field].member_offset : 0, action.field != 0}}, true);
        return;
    }
    Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
    Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
    Value at = emit(i, {base.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : 0)});
    destroy(action.destructor, action.type, at);
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
            sem.entities[e].definition && sem.destructor_needed(sem.object_destructor(e))) work.push_back(e);
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
    for (auto it = work.rbegin(); it != work.rend(); ++it) {
        BlockId end;
        if (auto guard = reference_guards.get(*it)) {
            Value active = emit(Opcode::Load,IRType::I64,{Operand::symbol(SymbolId(guard))});
            BlockId run = block(); end = block();
            emit(Opcode::Branch,IRType(),{active.operand,Operand::label(run),Operand::label(end)}); start(run);
        }
        destroy_object(*it, sem.object_destructor(*it));
        if (end) { jump(end); start(end); }
    }
    emit(Opcode::Return, IRType(), {}); builder.reset();
}
} }
