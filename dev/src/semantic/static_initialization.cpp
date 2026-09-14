#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::constant_initializer(NodeId n, TypeId t, bool local)
{
    if (NodeId source = class_initialization(n,t).source) {
        auto value = expression_fact(source);
        return empty_value(t) && value.form == ExpressionForm::Construction && !value.argument_count &&
            !constructor_needed(facts[source].entity);
    }
    if (auto plan = initializer_plan(n, t)) return constant_plan(plan,local);
    if (n && constructor_member(facts[n].entity)) return false;
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = types[t];
    if (!n && value_constructor(t)) return false;
    if ((target.kind == TypeKind::Named && entities[target.entity].class_info) || target.kind == TypeKind::Array) {
        if (n) throw std::logic_error("missing static aggregate initializer plan");
        return true; // Uninitialized static storage is zero-initialized.
    }
    return static_value(n, t).kind != StaticValue::Invalid;
}
bool Analyzer::constant_plan(std::uint32_t plan, bool local)
{
    auto k = key(plan,local);
    if (auto known = static_plan_facts.get(k)) return known == 2;
    ++static_plan_work;
    auto action = initializers[plan];
    bool valid = false;
    if (action.kind == InitKind::Constructor) valid =
        (!local || entities[facts[action.source].entity].constexpr_function) &&
        !class_initialization(action.source,action.type).source && constant_construction(action.source,action.type).valid;
    else if (action.kind == InitKind::Value) valid = !value_constructor(action.type);
    else if (action.kind == InitKind::String) valid = true;
    else if (action.kind == InitKind::Scalar) valid = static_value(action.source,action.type).kind != StaticValue::Invalid;
    else if (action.kind == InitKind::Group) {
        valid = true;
        for (auto child = action.first; valid && child; child = initializers[child].next) valid = constant_plan(child,local);
    }
    static_plan_facts.put(k,valid ? 2 : 1); return valid;
}
bool Analyzer::local_static(EntityId e) const
{
    auto entity = entities[e]; auto scope = scopes[entity.owner].kind;
    return entity.kind == EntityKind::Variable && entity.is_static &&
        scope != ScopeKind::Namespace && scope != ScopeKind::Class && !static_temporary(e).object;
}
bool Analyzer::static_initialization(EntityId e)
{
    // Queried after declaration checking. The immutable initializer, selected
    // constructor and completed layout are owned by this declaration.
    if (auto known = static_initialization_facts.get(e)) return known == 2;
    ++static_initialization_work;
    auto entity = entities[e]; auto t = entity.type;
    bool constant = !entity.initializer || constant_initializer(entity.initializer,t,local_static(e));
    if (!entity.initializer && class_value(t)) constant = (local_static(e) || empty_value(t)) && !constructor_needed(object_constructor(e));
    if (!entity.initializer && types[t].kind == TypeKind::Array) constant = !constructor_needed(object_constructor(e));
    if (static_vptr(e)) constant = true;
    static_initialization_facts.put(e,constant ? 2 : 1); return constant;
}
} }
