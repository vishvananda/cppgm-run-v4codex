#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Access Analyzer::declaration_access(ScopeId s) const
{
    return scopes[s].kind == ScopeKind::Class ? class_facts[entities[scopes[s].entity].class_info].current_access : Access::Public;
}
ScopeId Analyzer::naming_class(ScopeId s) const
{
    for (; s; s = scopes[s].parent) if (scopes[s].kind == ScopeKind::Class) return s;
    return 0;
}
bool Analyzer::class_derives(EntityId derived, EntityId base) const
{
    for (EntityId current = derived; current && entities[current].class_info;) {
        if (current == base) return true;
        auto edge = class_facts[entities[current].class_info].first_base;
        current = edge ? bases[edge].base : 0;
    }
    return false;
}
bool Analyzer::privileged(ScopeId context, EntityId cls) const
{
    for (; context; context = scopes[context].parent) {
        auto scope = scopes[context];
        if (scope.kind == ScopeKind::Class && scope.entity == cls) return true;
        if (scope.entity && friendships.get(key(cls, scope.entity))) return true;
    }
    return false;
}
void Analyzer::check_base_access(TypeId from, TypeId to, ScopeId context)
{
    if (access_override) context = access_override;
    EntityId cls = types[from].entity, target = types[to].entity;
    for (EntityId current = cls; current && current != target;) {
        auto edge = class_facts[entities[current].class_info].first_base;
        if (!edge) throw std::runtime_error("unrelated base conversion");
        Access level = bases[edge].access;
        bool allowed = level == Access::Public || privileged(context, current);
        if (!allowed && level == Access::Protected) {
            for (auto s = context; s; s = scopes[s].parent)
                if (scopes[s].kind == ScopeKind::Class && class_derives(scopes[s].entity, current)) allowed = true;
            for (EntityId d = cls; !allowed && d && d != current;) {
                allowed = privileged(context, d);
                auto b = class_facts[entities[d].class_info].first_base; d = b ? bases[b].base : 0;
            }
        }
        if (!allowed) throw std::runtime_error("inaccessible base conversion");
        current = bases[edge].base;
    }
}
void Analyzer::check_access(EntityId e, ScopeId context, ScopeId naming, TypeId object)
{
    if (!calls || !e || entities[e].kind == EntityKind::Overload || scopes[entities[e].owner].kind != ScopeKind::Class) return;
    if (access_override) context = access_override;
    EntityId owner = scopes[entities[e].owner].entity;
    EntityId named = scopes[naming_class(naming)].entity;
    if (!named || !class_derives(named, owner)) named = owner;
    Access level = entities[e].access;
    EntityId introduced = owner;
    for (EntityId current = named; current;) {
        auto exposed = using_access.get(key(entities[current].scope, e));
        if (exposed) { introduced = current; level = Access(exposed-1); break; }
        if (current == owner) break;
        auto b = class_facts[entities[current].class_info].first_base; current = b ? bases[b].base : 0;
    }
    if (named != introduced) check_base_access(entities[named].type, entities[introduced].type, context);
    if (level == Access::Public || privileged(context, introduced)) return;
    if (level == Access::Protected) {
        bool object_bound = !entities[e].is_static && (entities[e].kind == EntityKind::Variable || entities[e].kind == EntityKind::Function);
        EntityId actual = object && types[object].kind == TypeKind::Named ? types[object].entity : named;
        if (!object && object_bound) {
            EntityId enclosing = scopes[naming_class(context)].entity;
            if (class_derives(enclosing, introduced)) actual = enclosing;
        }
        for (EntityId candidate = actual; candidate && class_derives(candidate, introduced);) {
            if (privileged(context, candidate)) return;
            auto b = class_facts[entities[candidate].class_info].first_base; candidate = b ? bases[b].base : 0;
        }
        if (!object_bound) for (auto s = context; s; s = scopes[s].parent)
            if (scopes[s].kind == ScopeKind::Class && class_derives(scopes[s].entity, introduced)) return;
    }
    throw std::runtime_error("inaccessible class member");
}
} }
