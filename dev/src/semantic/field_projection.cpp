#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
FieldProjection Analyzer::field_projection(EntityId field, TypeId receiver)
{
    while (types[receiver].kind == TypeKind::Pointer || types[receiver].kind == TypeKind::LRef ||
        types[receiver].kind == TypeKind::RRef) receiver = types[receiver].child;
    auto owner = types[receiver].entity;
    auto identity = key(field,owner);
    if (auto prior = field_projection_index.get(identity)) return field_projections[prior];
    auto declared = scopes[entities[field].owner].entity;
    // Layout and storage ownership are established semantic facts. This query
    // only combines their offsets; it cannot demand declarations or bodies.
    if (class_facts[entities[declared].class_info].layout_state != FactState::Success)
        throw std::logic_error("field projection requires completed layout");
    FieldProjection result;
    if (declared != owner) {
        auto storage = injected_storage(field);
        if (storage && nonstatic_field(storage)) result = field_projection(storage,receiver);
        else result.object = storage;
    }
    result.offset += entities[field].member_offset;
    field_projection_index.put(identity,field_projections.size());
    field_projections.push_back(result);
    return result;
}
} }
