#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
bool Analyzer::inherit_using(NodeId name, ScopeId scope)
{
    if (ast[name].first == ast[name].last) return false;
    ScopeId owner = name_owner(name, scope);
    if (scopes[owner].kind != ScopeKind::Class || terminal(name) != scopes[owner].name) return false;
    EntityId cls = scopes[scope].entity, base = scopes[owner].entity;
    auto info = entities[cls].class_info;
    bool direct = false;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) direct |= bases[b].base == base;
    if (!direct) throw std::runtime_error("inherited constructor must name a direct base");
    if (class_facts[info].inherited_base) throw std::runtime_error("repeated constructor using-declaration");
    class_facts[info].inherited_base = base;
    return true;
}
void Analyzer::inherited_constructors(EntityId cls)
{
    auto info = entities[cls].class_info;
    EntityId base = class_facts[info].inherited_base;
    if (!base) return;
    // Class completion sees every locally declared signature, including ones
    // after the using-declaration. An inherited signature never replaces it.
    for (EntityId target : candidates(class_facts[entities[base].class_info].constructor)) {
        if (!target || !types[entities[target].type].count) continue;
        EntityId e = declare_function(entities[cls].scope, entities[cls].name, 0, entities[target].type, true);
        if (entities[e].member_info) continue;
        member_facts(e);
        MemberFacts source = members[entities[target].member_info];
        auto m = entities[e].member_info;
        members[m].synthetic = members[m].constructor = true;
        members[m].inherited_constructor = target;
        members[m].explicit_constructor = source.explicit_constructor;
        members[m].deleted = source.deleted;
        entities[e].inline_function = true;
        entities[e].defaults = entities[target].defaults;
        entities[e].exception_spec = entities[target].exception_spec;
        class_facts[info].constructor = merge_lookup(class_facts[info].constructor, e);
    }
}
} }
