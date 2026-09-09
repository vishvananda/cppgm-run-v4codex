#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using syntax::Kind;
using namespace lowir_model;
bool Procedural::constant_initializer(NodeId n, TypeId t)
{
    if (n && sem.constructor_member(sem.facts[n].entity)) return false;
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    if (target.kind == TypeKind::Named && sem.entities[target.entity].class_info) {
        NodeId c = ast[n].first;
        for (auto d = sem.scopes[sem.entities[target.entity].scope].first_decl; d; d = sem.declarations[d].next) {
            EntityId field = sem.declarations[d].entity;
            if (!sem.nonstatic_field(field)) continue;
            if (!constant_initializer(c, sem.entities[field].type)) return false;
            if (c) c = ast[c].next;
        }
        return true;
    }
    if (target.kind == TypeKind::Array) {
        for (NodeId c = ast[n].first; c; c = ast[c].next)
            if (!constant_initializer(c, target.child)) return false;
        return true;
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
        if (entity.initializer) initialize(entity.initializer, entity.type, location);
        else if (sem.types[entity.type].kind == TypeKind::Array && sem.object_constructor(e)) array_construct(sem.object_constructor(e), entity.type, location, false, {});
        else if (sem.constructor_needed(sem.object_constructor(e))) construct(sem.object_constructor(e), 0, address(location));
        clean_inline(live, 0);
    }
    emit(Opcode::Return, IRType(), {});
    builder.reset();
}
} }
