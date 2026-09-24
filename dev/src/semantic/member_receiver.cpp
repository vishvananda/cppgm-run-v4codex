#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::record_member_receiver(Expression& result, NodeId node, TypeId object,
    EntityId selected, ScopeId naming, bool qualified, ScopeId context, bool layout)
{
    auto owner = scopes[entities[selected].owner].entity;
    auto qualifier = qualified && naming && scopes[naming].kind == ScopeKind::Class ? scopes[naming].entity : 0;
    // The qualified naming class selects the subobject before the declaration's
    // own base path. The direct object-to-owner path can be ambiguous even when
    // these two paths are individually unambiguous.
    auto path = [&](TypeId from, EntityId to) { return layout ? base_steps(from,to) : base_path(from,to); };
    auto first = qualifier ? path(object,qualifier) : 0;
    auto from = qualifier ? entities[qualifier].type : object;
    auto tail = path(from,owner);
    if ((first && (!base_adjustments[first].edge || base_adjustments[first].ambiguous)) ||
        (tail && (!base_adjustments[tail].edge || base_adjustments[tail].ambiguous)))
        throw std::runtime_error("invalid selected member subobject path");
    if (qualifier) check_base_access(object,from,context);
    auto type = entities[selected].kind == EntityKind::Function ? types.parameters[types[call_type(selected)].offset] :
        types.compound(TypeKind::Pointer,entities[owner].type);
    record_object(result,node,type,tail);
    auto& use = object_uses[result.object_use];
    use.qualifier_adjustment = first;
    if (!qualified) use.virtual_slot = members[entities[selected].member_info].virtual_slot;
}
} }
