#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
bool Procedural::constant_initializer(NodeId n, TypeId t)
{
    if (auto plan = sem.initializer_plan(n, t)) return constant_plan(plan);
    if (n && sem.constructor_member(sem.facts[n].entity)) return false;
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    if (!n && sem.value_constructor(t)) return false;
    if ((target.kind == TypeKind::Named && sem.entities[target.entity].class_info) || target.kind == TypeKind::Array) {
        if (n) throw std::logic_error("missing static aggregate initializer plan");
        return true; // Uninitialized static storage is zero-initialized.
    }
    return sem.static_value(n, t).kind != semantic::StaticValue::Invalid;
}
void Procedural::global_initialization()
{
    reset_lifetime(0);
    Function f; f.symbol = fresh_symbol("@__cppgm_init");
    function = FunctionId(p.functions.size()+1);
    auto void_type = sem.types.fundamental(FT_VOID);
    f.signature = signature(sem.types.function(void_type, {}, false), function);
    p.functions.push_back(f);
    auto& symbol = p.symbols[f.symbol.index-1];
    symbol.kind = Symbol::FunctionSymbol; symbol.entity = function.index;
    symbol.metadata.role = SR_INIT; symbol.metadata.binding = SBM_INTERNAL;
    builder.reset(new FunctionBuilder(p, function)); this_slot = SlotId();
    start(block());
    for (EntityId e : global_initializers) {
        initialized_units = semantic::Index();
        auto entity = sem.entities[e];
        Value location(Operand::symbol(symbols[e]), type(entity.type), entity.type, true);
        TypeId leaf = entity.type;
        while (sem.types[leaf].kind == TypeKind::Array) leaf = sem.types[leaf].child;
        if (entity.initializer && sem.types[leaf].kind == TypeKind::Named && sem.entities[sem.types[leaf].entity].class_info && sem.initializer_plan(entity.initializer, entity.type)) {
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
