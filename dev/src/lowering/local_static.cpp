#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::prepare_local_static(EntityId e, bool dynamic)
{
    LocalStatic local{e,SymbolId(),SymbolId(),dynamic};
    auto dtor = sem.object_destructor(e);
    bool destruction = sem.destructor_needed(dtor) || local_static_references.get(e);
    if (dynamic || destruction) {
        Global g; g.symbol = fresh_symbol(p.name(p.symbols[symbols[e].index-1].name)+"__guard"); g.type = IRType::I64;
        g.data.begin = p.data.size(); g.data.count = 1;
        DataItem zero; zero.zero_bytes = 8; p.data.push_back(zero); p.globals.push_back(g);
        auto& symbol = p.symbols[g.symbol.index-1]; symbol.kind = Symbol::GlobalSymbol;
        symbol.entity = p.globals.size(); symbol.metadata.binding = SBM_INTERNAL;
        if (sem.entities[e].thread_local_storage) symbol.metadata.storage = GSM_THREAD_LOCAL;
        local.guard = g.symbol;
    }
    if (destruction) {
        auto void_type = sem.types.fundamental(FT_VOID), ptr = sem.types.compound(TypeKind::Pointer,void_type);
        const auto exit_key = (std::uint64_t(3) << 32) | abi.name(0,"atexit");
        if (!atexit_symbol) atexit_symbol = SymbolId(linkage.external.get(exit_key));
        if (!atexit_symbol) {
            Function f; f.symbol = fresh_symbol("@__cppgm_runtime_atexit"); f.declaration = true;
            FunctionId id(p.functions.size()+1);
            f.signature = signature(sem.types.function(sem.types.fundamental(FT_INT),{ptr},false),id);
            p.functions.push_back(f); atexit_symbol = f.symbol;
            linkage.external.put(exit_key,f.symbol.index);
            auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = Symbol::FunctionSymbol; symbol.entity = id.index;
            symbol.metadata.object = p.intern("atexit"); symbol.metadata.linkage = LLM_C; symbol.metadata.binding = SBM_STRONG;
        }
        Function f; f.symbol = fresh_symbol("@__local_destructor_"+std::to_string(e));
        FunctionId id(p.functions.size()+1); f.signature = signature(sem.types.function(void_type,{},false),id);
        p.functions.push_back(f); local.destructor = f.symbol;
        auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = Symbol::FunctionSymbol; symbol.entity = id.index;
        symbol.metadata.binding = SBM_INTERNAL;
        if (dtor && sem.function_nonthrowing(dtor)) p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
    }
    local_static_index.put(e,local_statics.size()); local_statics.push_back(local);
}
void Procedural::initialize_local_static(EntityId e)
{
    auto local = local_statics[local_static_index.get(e)];
    if (!local.guard) return;
    Value guard = emit(Opcode::Load,IRType::I64,{Operand::symbol(local.guard)});
    Value done = emit(Opcode::Compare,IRType::I64,{guard.operand,Operand::integer(0)},Operation::Ne);
    auto run = block(), end = block();
    emit(Opcode::Branch,IRType(),{done.operand,Operand::label(end),Operand::label(run)}); start(run);
    auto saved_live = live;
    auto entity = sem.entities[e];
    if (local.dynamic) {
        initialized_units = semantic::Index();
        Value location = emit(Opcode::Addr,IRType(),{Operand::symbol(symbols[e])});
        location.type = entity.type; location.address = true;
        begin_full_expression(entity.initializer);
        if (sem.reference_scalar(e)) initialize_reference(e,location);
        else if (entity.initializer) initialize(entity.initializer,entity.type,location);
        else if (sem.types[entity.type].kind == TypeKind::Array)
            array_construct(sem.object_constructor(e),entity.type,location,false,{});
        else construct(sem.object_constructor(e),0,address(location));
        finish_full_expression(saved_live);
    }
    if (local.destructor) emit(Opcode::Call,IRType::I32,{Operand::symbol(atexit_symbol),Operand::symbol(local.destructor)});
    emit(Opcode::Store,IRType::I64,{Operand::integer(1),Operand::symbol(local.guard)});
    jump(end); start(end);
}
void Procedural::emit_local_static_destructors()
{
    for (auto local : local_statics) {
        if (!local.destructor) continue;
        reset_lifetime(0); function = FunctionId(p.symbols[local.destructor.index-1].entity);
        builder.reset(new FunctionBuilder(p,function)); this_slot = SlotId(); start(block());
        if (auto destructor = sem.object_destructor(local.object)) destroy_object(local.object,destructor);
        for (auto id = local_static_references.get(local.object); id; id = local_static_reference_objects[id].next) {
            auto object = local_static_reference_objects[id].object;
            BlockId end;
            if (auto guard = reference_guards.get(object)) {
                auto active = emit(Opcode::Load,IRType::I64,{Operand::symbol(SymbolId(guard))});
                auto run = block(); end = block();
                emit(Opcode::Branch,IRType(),{active.operand,Operand::label(run),Operand::label(end)}); start(run);
            }
            destroy_object(object,sem.object_destructor(object));
            if (end) { jump(end); start(end); }
        }
        emit(Opcode::Return,IRType(),{}); builder.reset();
    }
}
} }
