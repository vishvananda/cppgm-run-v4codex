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
        std::vector<std::uint32_t> pending;
        for (auto b = first; b; b = bases[b].next) pending.push_back(b);
        while (!pending.empty()) {
            auto edge = bases[pending.back()]; pending.pop_back();
            auto e = edge.base;
            unsigned bit = edge.virtual_base ? 2 : 1, prior = seen.get(e);
            if (prior) {
                if (edge.virtual_base && (prior & 2)) flags |= 2;
                else flags |= 1;
                // Repeating an ordinary subtree repeats each of its virtual
                // anchors too. Summaries avoid expanding identical subtrees.
                if (!edge.virtual_base && (prior & 1) && virtual_base_count(e)) flags |= 2;
            }
            if (prior & bit) continue;
            seen.put(e,prior|bit);
            for (auto b = class_facts[entities[e].class_info].first_base; b; b = bases[b].next) {
                ++rtti_base_work; pending.push_back(b);
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
