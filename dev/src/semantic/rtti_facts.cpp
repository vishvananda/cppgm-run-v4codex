#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::record_rtti_type(TypeId type)
{
    if (!type) return;
    type = types.unqualified(type);
    if (rtti_type_demand_index.get(type)) return;
    rtti_type_demand_index.put(type,1); rtti_type_demands.push_back(type);
}
void Analyzer::complete_rtti_class(EntityId cls)
{
    auto info = entities[cls].class_info;
    if (!entities[cls].complete || class_facts[info].rtti_flags != ~0U) return;
    ++rtti_class_work;
    unsigned flags = 0;
    auto first = class_facts[info].first_base;
    for (auto b = first; b; b = bases[b].next) {
        ++rtti_base_work;
        complete_rtti_class(bases[b].base);
        flags |= class_facts[entities[bases[b].base].class_info].rtti_flags;
    }
    // Single-base propagation is constant work. Branching RTTI must also
    // describe repeated nonvirtual types: visit relevant edges once, without
    // expanding repeated subtrees or walking unrelated class registries.
    if (first && bases[first].next) {
        Index seen;
        std::vector<EntityId> pending;
        for (auto b = first; b; b = bases[b].next) pending.push_back(bases[b].base);
        while (!pending.empty()) {
            auto e = pending.back(); pending.pop_back();
            if (seen.get(e)) { flags |= 1; continue; }
            seen.put(e,1);
            for (auto b = class_facts[entities[e].class_info].first_base; b; b = bases[b].next) {
                ++rtti_base_work; pending.push_back(bases[b].base);
            }
        }
    }
    class_facts[info].rtti_flags = flags;
}
void Analyzer::finish_rtti_facts()
{
    // Run after source/body discovery. A previously incomplete class can have
    // acquired its definition; no type-demand miss is cached before that point.
    for (std::size_t i = 0; i < rtti_type_demands.size(); ++i) {
        auto t = types[rtti_type_demands[i]];
        if (t.kind == TypeKind::Named && entities[t.entity].class_info) complete_rtti_class(t.entity);
        if (t.kind == TypeKind::Pointer || t.kind == TypeKind::MemberPointer) record_rtti_type(t.child);
        if (t.kind == TypeKind::MemberPointer) record_rtti_type(entities[t.entity].type);
    }
}
unsigned Analyzer::rtti_class_flags(EntityId cls) const
{
    auto flags = class_facts[entities[cls].class_info].rtti_flags;
    if (flags == ~0U) throw std::logic_error("undemanded class RTTI facts");
    return flags;
}
} }
