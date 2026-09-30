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
std::uint32_t Analyzer::access_base(EntityId entity) const
{
    auto info = entities[entity].class_info;
    auto edge = info ? class_facts[info].first_base : 0;
    return edge ? edge : template_pattern_bases.get(entity);
}
bool Analyzer::class_derives(EntityId derived, EntityId base) const
{
    std::vector<EntityId> work(1,derived); Index seen;
    for (unsigned i = 0; i < work.size(); ++i) {
        auto current = work[i];
        if (current == base) return true;
        if (seen.get(current)) continue;
        seen.put(current,1);
        for (auto edge = access_base(current); edge; edge = bases[edge].next) work.push_back(bases[edge].base);
    }
    return false;
}
bool Analyzer::privileged(ScopeId context, EntityId cls) const
{
    for (; context; context = scopes[context].parent) {
        auto scope = scopes[context];
        if (scope.kind == ScopeKind::Class && scope.entity == cls) return true;
        if (scope.entity && friendships.get(key(cls, scope.entity))) return true;
        // A friend template grants access to each specialization, including
        // explicitly specialized definitions. The grant belongs to its
        // canonical declaration; it is not copied to every instantiation.
        if (scope.entity && entities[scope.entity].specialization &&
            friendships.get(key(cls,specializations[entities[scope.entity].specialization].pattern))) return true;
    }
    return false;
}
void Analyzer::check_base_access(TypeId from, TypeId to, ScopeId context)
{
    check_base_entity_access(types[from].entity,types[to].entity,context);
}
void Analyzer::check_base_entity_access(EntityId cls, EntityId target, ScopeId context)
{
    if (!base_accessible(cls,target,context)) throw std::runtime_error("inaccessible base conversion");
}
bool Analyzer::base_accessible(EntityId cls, EntityId target, ScopeId context)
{
    if (explicit_instantiation_naming) return true;
    if (access_override) context = access_override;
    if (cls == target) return true;
    std::vector<EntityId> work(1,cls); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto current = work[i];
        if (current == target) return true;
        if (seen.get(current)) continue;
        seen.put(current,1);
        for (auto edge = access_base(current); edge; edge = bases[edge].next) {
            if (!access_derives(bases[edge].base,target)) continue;
            auto level = bases[edge].access;
            bool allowed = level == Access::Public || privileged(context,current);
            if (!allowed && level == Access::Protected) {
                for (auto s = context; s; s = scopes[s].parent)
                    if (scopes[s].kind == ScopeKind::Class && access_derives(scopes[s].entity,current)) allowed = true;
                if (!allowed) allowed = privileged_base_path(context,cls,current);
            }
            if (allowed) work.push_back(bases[edge].base);
        }
    }
    return false;
}
void Analyzer::check_access(EntityId e, ScopeId context, ScopeId naming, TypeId object)
{
    if (!accessible(e,context,naming,object)) throw std::runtime_error("inaccessible class member");
}
unsigned Analyzer::using_member_access(ScopeId scope, EntityId member) const
{
    // Exposure belongs to the declaration named by using, and applies to its
    // specializations without copying access entries into every instance.
    for (auto e = member; e;) {
        if (auto access = using_access.get(key(scope,e))) return access;
        auto spec = entities[e].specialization;
        if (!spec) break;
        e = specializations[spec].pattern;
    }
    return 0;
}
bool Analyzer::accessible(EntityId e, ScopeId context, ScopeId naming, TypeId object)
{
    if (explicit_instantiation_naming) return true;
    if (!calls || !e || entities[e].kind == EntityKind::Overload || scopes[entities[e].owner].kind != ScopeKind::Class) return true;
    if (access_override) context = access_override;
    EntityId owner = scopes[entities[e].owner].entity;
    if (injected_class_owners.get(owner)) {
        // The enclosing declaration owns access through inheritance. Anonymous
        // storage is a member, not an additional base on that access path.
        if (!accessible(owner,context,naming,object)) return false;
        return accessible_introduction(e,context,owner,owner,entities[e].access,object);
    }
    EntityId named = scopes[naming_class(naming)].entity;
    if (!named || !access_derives(named, owner)) named = owner;
    if (named == owner) return accessible_introduction(e,context,named,owner,entities[e].access,object);
    std::vector<EntityId> work(1,named); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto current = work[i];
        if (seen.get(current)) continue;
        seen.put(current,1);
        auto exposed = using_member_access(entities[current].scope,e);
        if (exposed || current == owner) {
            if (accessible_introduction(e,context,named,current,exposed ? Access(exposed-1) : entities[e].access,object)) return true;
            continue;
        }
        for (auto b = access_base(current); b; b = bases[b].next)
            if (access_derives(bases[b].base,owner)) work.push_back(bases[b].base);
    }
    return false;
}
} }
