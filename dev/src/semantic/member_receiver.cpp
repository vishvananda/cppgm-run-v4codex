#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::record_member_receiver(Expression& result, NodeId node, TypeId object,
    EntityId selected, ScopeId naming, bool qualified, ScopeId context)
{
    auto owner = scopes[entities[selected].owner].entity;
    auto qualifier = qualified && naming && scopes[naming].kind == ScopeKind::Class ? scopes[naming].entity : 0;
    // The qualified naming class selects the subobject before the declaration's
    // own base path. The direct object-to-owner path can be ambiguous even when
    // these two paths are individually unambiguous.
    auto first = qualifier ? base_steps(object,qualifier) : 0;
    auto from = qualifier ? entities[qualifier].type : object;
    auto tail = base_steps(from,owner);
    if (qualifier) check_base_access(object,from,context);
    record_object(result,node,types.parameters[types[call_type(selected)].offset],tail);
    auto& use = object_uses[result.object_use];
    use.qualifier_adjustment = first;
    if (!qualified) use.virtual_slot = members[entities[selected].member_info].virtual_slot;
}
} }
