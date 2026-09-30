#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
bool Analyzer::access_derives(EntityId derived, EntityId base)
{
    if (derived == base) return true;
    // Access checking must not demand class completion. Completed edge graphs
    // share canonical path/miss facts; open declarations retain local queries.
    return entities[derived].complete ? base_adjustments[base_path(entities[derived].type,base)].edge != 0 :
        class_derives(derived,base);
}
bool Analyzer::privileged_base_path(ScopeId context, EntityId object, EntityId base)
{
    std::vector<EntityId> work(1,object); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto current = work[i];
        if (seen.get(current) || !access_derives(current,base)) continue;
        seen.put(current,1);
        if (privileged(context,current)) return true;
        for (auto b = access_base(current); b; b = bases[b].next) work.push_back(bases[b].base);
    }
    return false;
}
bool Analyzer::accessible_introduction(EntityId e, ScopeId context, EntityId named,
    EntityId introduced, Access level, TypeId object)
{
    if (named != introduced && !base_accessible(named,introduced,context)) return false;
    if (level == Access::Public || privileged(context,introduced)) return true;
    if (level != Access::Protected) return false;
    bool bound = !entities[e].is_static &&
        (entities[e].kind == EntityKind::Variable || entities[e].kind == EntityKind::Function);
    auto actual = object && types[object].kind == TypeKind::Named ? types[object].entity : named;
    if (!object && bound) {
        auto enclosing = scopes[naming_class(context)].entity;
        if (access_derives(enclosing,introduced)) actual = enclosing;
    }
    if (privileged_base_path(context,actual,introduced)) return true;
    if (!bound) for (auto s = context; s; s = scopes[s].parent)
        if (scopes[s].kind == ScopeKind::Class && access_derives(scopes[s].entity,introduced)) return true;
    return false;
}
} }
