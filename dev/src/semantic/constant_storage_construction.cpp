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
std::uint32_t Analyzer::constant_construction_receiver(ConstantBuilder& builder,
    std::uint32_t receiver, unsigned selected)
{
    if (!selected) return receiver;
    // One temporary builder per selected storage path. Prefixes and field
    // addresses are canonical identities; no repeated aggregate copying.
    auto& missing = builder.missing_paths; missing.clear();
    for (auto path = selected; path && !builder.groups_by_path.get(path); path = construction_storage[path].parent)
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
    return builder.groups[builder.groups_by_path.get(selected)-1].address;
}
void Analyzer::constant_projected_action(ConstantBuilder& builder, const SubobjectAction& action,
    std::uint32_t receiver, Constant value)
{
    constant_construction_receiver(builder,receiver,action.storage);
    auto& group = builder.groups[builder.groups_by_path.get(action.storage)-1];
    ConstantBuildPart entry; entry.part.selector = action.field; entry.part.value = value;
    builder.projected_parts.push_back(entry);
    auto slot = builder.projected_parts.size();
    if (group.last) builder.projected_parts[group.last-1].next = slot;
    else group.first = slot;
    group.last = slot;
    auto address = constant_subobject(group.address,action.type,action.field);
    builder.values_by_address.put(address,slot);
}
void Analyzer::finish_constant_projections(ConstantBuilder& builder)
{
    // Parents precede children; freeze each completed storage once and append
    // its value to its parent. Temporary builders die with this activation.
    std::vector<EvaluatedPart> parts;
    for (auto it = builder.groups.rbegin(); it != builder.groups.rend(); ++it) {
        auto field = construction_storage[it->path].field;
        parts.clear();
        for (auto p = it->first; p; p = builder.projected_parts[p-1].next)
            parts.push_back(builder.projected_parts[p-1].part);
        EvaluatedPart part; part.selector = field;
        part.value = evaluated_object(entities[field].type,parts);
        if (it->parent) {
            auto& parent = builder.groups[it->parent-1];
            ConstantBuildPart entry; entry.part = part;
            builder.projected_parts.push_back(entry);
            auto slot = builder.projected_parts.size();
            if (parent.last) builder.projected_parts[parent.last-1].next = slot;
            else parent.first = slot;
            parent.last = slot;
        } else {
            auto slot = builder.slots.get(field);
            if (slot) builder.parts[slot-1] = part;
            else { builder.parts.push_back(part); builder.slots.put(field,builder.parts.size()); }
        }
    }
}
} }
