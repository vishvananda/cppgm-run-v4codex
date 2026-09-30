#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::compose_subobject(std::uint32_t outer, std::uint32_t inner)
{
    if (!outer || subobjects[inner].anchor) return inner;
    if (!inner) return outer;
    auto k = key(outer,inner);
    if (auto old = subobject_composition_index.get(k)) return old;
    auto object = subobjects[outer];
    std::vector<unsigned> edges;
    for (auto path = object.path; path; path = subobject_paths[path].next)
        edges.push_back(subobject_paths[path].edge);
    auto result = inner;
    for (auto i = edges.rbegin(); i != edges.rend(); ++i) result = prefix_subobject(*i,result);
    if (object.anchor) {
        auto composed = subobjects[result]; composed.anchor = object.anchor;
        auto identity = key(composed.anchor,composed.path);
        result = subobject_index.get(identity);
        if (!result) { result = subobjects.size(); subobjects.push_back(composed); subobject_index.put(identity,result); }
    }
    subobject_composition_index.put(k,result); return result;
}
unsigned Analyzer::select_primary_base(EntityId cls)
{
    auto info = entities[cls].class_info;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next)
        if (!bases[b].virtual_base && dynamic_class(bases[b].base)) return b;
    if (!host_abi || !virtual_base_count(cls)) return 0;
    // Preorder graph walk, with each completed class visited once. Edge IDs
    // preserve the virtual anchor even when the selected primary is indirect.
    std::vector<unsigned> pending, order;
    Index visited, virtual_seen, indirect;
    auto push = [&](EntityId e) {
        std::vector<unsigned> edges;
        for (auto b = class_facts[entities[e].class_info].first_base; b; b = bases[b].next) edges.push_back(b);
        pending.insert(pending.end(),edges.rbegin(),edges.rend());
    };
    push(cls);
    while (!pending.empty()) {
        auto b = pending.back(); pending.pop_back();
        auto base = bases[b].base;
        if (bases[b].virtual_base && !virtual_seen.get(base)) {
            virtual_seen.put(base,1); order.push_back(b);
        }
        if (visited.get(base)) continue;
        visited.put(base,1); ++virtual_base_work;
        size(entities[base].type);
        auto primary = class_facts[entities[base].class_info].primary_base;
        if (primary && bases[primary].virtual_base) indirect.put(bases[primary].base,1);
        push(base);
    }
    unsigned fallback = 0;
    for (auto b : order) if (bases[b].virtual_base && class_facts[entities[bases[b].base].class_info].nearly_empty) {
        if (!fallback) fallback = b;
        if (!indirect.get(bases[b].base)) return b;
    }
    return fallback;
}
void Analyzer::complete_virtual_prefix(EntityId cls, VirtualClass& model)
{
    auto primary = class_facts[entities[cls].class_info].primary_base;
    if (primary) {
        const auto& inherited = virtual_class(bases[primary].base);
        model.prefix = bases[primary].virtual_base ? inherited.virtual_prefix : inherited.prefix;
        for (auto& row : model.prefix) if (row.declaration) row.origin = prefix_subobject(primary,row.origin);
    }
    Index base_order;
    auto add_base = [&](EntityId base) {
        if (!base_order.get(base)) { base_order.put(base,1); model.base_order.push_back(base); }
    };
    for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
        if (bases[b].virtual_base) add_base(bases[b].base);
        if (dynamic_class(bases[b].base))
            for (auto base : virtual_class(bases[b].base).base_order) add_base(base);
    }
    Index base_rows;
    for (const auto& row : model.prefix) if (row.base) base_rows.put(row.base,1);
    for (auto base : model.base_order) {
        if (base_rows.get(base)) continue;
        VirtualPrefixRow row; row.base = base; model.prefix.push_back(row); base_rows.put(base,1);
    }
    for (unsigned j = 0; j < model.prefix.size(); ++j) {
        const auto& row = model.prefix[j];
        if (row.base) virtual_base_rows[virtual_base_index.get(key(cls,row.base))-1] = -24-std::int64_t(j)*8;
    }
    model.virtual_prefix = model.prefix;
    Index signatures;
    auto shape = [&](EntityId e) {
        const auto& m = members[entities[e].member_info];
        return key(m.destructor ? 0 : entities[e].name,m.virtual_signature);
    };
    for (const auto& row : model.prefix) if (row.declaration) signatures.put(shape(row.declaration),1);
    for (const auto& slot : model.slots) {
        // Virtual bases have their own vcall region. Their shared primary
        // entries are already present in the inherited prefix.
        if (subobjects[slot.origin].anchor) continue;
        auto k = shape(slot.declaration);
        if (signatures.get(k)) continue;
        signatures.put(k,1);
        VirtualPrefixRow row; row.declaration = slot.declaration; row.origin = slot.origin;
        model.virtual_prefix.push_back(row);
    }
    virtual_base_work += model.prefix.size()+model.virtual_prefix.size();
}
} }
