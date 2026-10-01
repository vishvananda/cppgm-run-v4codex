#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
void Procedural::register_inline_temporaries(EntityId e)
{
    // Host TUs guard temporary registration separately from the reference.
    // Register in initialization order, so atexit destroys in reverse order.
    std::vector<EntityId> objects;
    for (auto id = local_static_references.get(e); id; id = local_static_reference_objects[id].next)
        objects.push_back(local_static_reference_objects[id].object);
    for (auto i = objects.rbegin(); i != objects.rend(); ++i) {
        auto object = *i;
        BlockId end;
        if (auto guard = reference_guards.get(object)) {
            auto active = emit(Opcode::Load,IRType::I64,{Operand::symbol(SymbolId(guard))});
            auto run = block(); end = block();
            emit(Opcode::Branch,IRType(),{active.operand,Operand::label(run),Operand::label(end)}); start(run);
        }
        initialize_local_static(object);
        if (end) { jump(end); start(end); }
    }
}
void Procedural::global_initialization()
{
    reset_lifetime(0);
    // Per-TU helpers must not retain a legacy singleton spelling when the
    // program-level scheduler later removes their runtime role.
    Function f; f.symbol = fresh_symbol(linkage.merge ? "@__cppgm_unit_init" : "@__cppgm_init");
    function = FunctionId(p.functions.size()+1);
    auto void_type = sem.types.fundamental(FT_VOID);
    f.signature = signature(sem.types.function(void_type, {}, false), function);
    p.functions.push_back(f);
    linkage.initializers.push_back(function);
    auto& symbol = p.symbols[f.symbol.index-1];
    symbol.kind = Symbol::FunctionSymbol; symbol.entity = function.index;
    symbol.metadata.role = SR_INIT; symbol.metadata.binding = SBM_INTERNAL;
    builder.reset(new FunctionBuilder(p, function)); this_slot = SlotId();
    start(block());
    for (EntityId e : global_initializers) {
        if (sem.entities[e].inline_variable) {
            initialize_local_static(e);
            register_inline_temporaries(e);
            continue;
        }
        SourceInvocationScope invocation(source_invocation,sem.object_source_sites.get(e));
        initialized_units = semantic::Index();
        auto entity = sem.entities[e];
        Value location(Operand::symbol(symbols[e]), type(entity.type), entity.type, true);
        TypeId leaf = entity.type;
        while (sem.types[leaf].kind == TypeKind::Array) leaf = sem.types[leaf].child;
        if (sem.reference_scalar(e)) initialize_reference(e,location);
        else if (entity.initializer && sem.types[leaf].kind == TypeKind::Named && sem.entities[sem.types[leaf].entity].class_info && sem.initializer_plan(entity.initializer, entity.type)) {
            std::vector<InitProjection> path;
            aggregate_initialize(entity.initializer, entity.type, location, false, path);
        } else if (entity.initializer) initialize(entity.initializer, entity.type, location);
        else if (sem.types[entity.type].kind == TypeKind::Array && sem.object_constructor(e)) array_construct(sem.object_constructor(e), entity.type, location, false, {});
        else if (sem.constructor_needed(sem.object_constructor(e))) construct(sem.object_constructor(e), 0, address(location));
        clean_inline(live, 0);
    }
    emit(Opcode::Return, IRType(), {});
    builder.reset();
}
} }
