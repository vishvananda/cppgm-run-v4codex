#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::constant_field_address(std::uint32_t parent, EntityId field)
{
    if (!parent) return 0;
    auto owner = scopes[entities[field].owner].entity;
    if (types[constant_addresses[parent].type].entity != owner) {
        auto storage = injected_storage(field);
        if (storage && nonstatic_field(storage)) parent = constant_field_address(parent,storage);
    }
    return constant_subobject(parent,entities[field].type,field);
}
std::uint32_t Analyzer::constant_construction_receiver(std::uint32_t receiver, unsigned path)
{
    if (!path) return receiver;
    auto step = construction_storage[path];
    auto parent = constant_construction_receiver(receiver,step.parent);
    return constant_subobject(parent,entities[step.field].type,step.field);
}
void Analyzer::constant_projected_action(ConstantBuilder& builder, const SubobjectAction& action,
    std::uint32_t receiver, Constant value)
{
    // One temporary builder per selected storage path. Prefixes and field
    // addresses are canonical identities; no repeated aggregate copying.
    std::vector<unsigned> missing;
    for (auto path = action.storage; path && !builder.groups_by_path.get(path); path = construction_storage[path].parent)
        missing.push_back(path);
    for (auto it = missing.rbegin(); it != missing.rend(); ++it) {
        auto path = construction_storage[*it];
        ConstantBuildGroup group; group.path = *it;
        group.parent = builder.groups_by_path.get(path.parent);
        auto parent = group.parent ? builder.groups[group.parent-1].address : receiver;
        group.address = constant_subobject(parent,entities[path.field].type,path.field);
        builder.groups.push_back(std::move(group));
        builder.groups_by_path.put(*it,builder.groups.size());
    }
    auto& group = builder.groups[builder.groups_by_path.get(action.storage)-1];
    EvaluatedPart part; part.selector = action.field; part.value = value;
    auto slot = group.slots.get(action.field);
    if (slot) group.parts[slot-1] = part;
    else { group.parts.push_back(part); group.slots.put(action.field,group.parts.size()); }
    auto address = constant_subobject(group.address,action.type,action.field);
    builder.projected_values.push_back(value);
    builder.values_by_address.put(address,builder.projected_values.size());
}
void Analyzer::finish_constant_projections(ConstantBuilder& builder)
{
    // Parents precede children; freeze each completed storage once and append
    // its value to its parent. Temporary builders die with this activation.
    for (auto it = builder.groups.rbegin(); it != builder.groups.rend(); ++it) {
        auto field = construction_storage[it->path].field;
        EvaluatedPart part; part.selector = field;
        part.value = evaluated_object(entities[field].type,it->parts);
        auto& slots = it->parent ? builder.groups[it->parent-1].slots : builder.slots;
        auto& parts = it->parent ? builder.groups[it->parent-1].parts : builder.parts;
        auto slot = slots.get(field);
        if (slot) parts[slot-1] = part;
        else { parts.push_back(part); slots.put(field,parts.size()); }
    }
}
} }
