#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::complete_virtual_bases(EntityId cls)
{
    auto info = entities[cls].class_info;
    auto begin = virtual_bases.size();
    auto add = [&](EntityId base) {
        ++virtual_base_work;
        auto k = key(cls,base);
        if (virtual_base_index.get(k)) return;
        virtual_base_index.put(k,virtual_bases.size()+1);
        virtual_bases.push_back(base); virtual_base_offsets.push_back(0); virtual_base_rows.push_back(0);
    };
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
        auto inherited = class_facts[entities[bases[b].base].class_info];
        for (unsigned j = 0; j < inherited.virtual_bases_count; ++j)
            add(virtual_bases[inherited.virtual_bases_begin+j]);
        if (bases[b].virtual_base) add(bases[b].base);
    }
    class_facts[info].virtual_bases_begin = begin;
    class_facts[info].virtual_bases_count = virtual_bases.size()-begin;
}
std::uint64_t Analyzer::virtual_base_offset(EntityId cls, EntityId base) const
{
    auto id = virtual_base_index.get(key(cls,base));
    if (!id) throw std::logic_error("missing virtual-base layout identity");
    return virtual_base_offsets[id-1];
}
std::int64_t Analyzer::virtual_base_row(EntityId cls, EntityId base) const
{
    auto id = virtual_base_index.get(key(cls,base));
    if (!id) throw std::logic_error("missing virtual-base row identity");
    if (host_abi) {
        if (!virtual_base_rows[id-1]) throw std::logic_error("unpublished virtual-base prefix row");
        return virtual_base_rows[id-1];
    }
    auto position = id-1-class_facts[entities[cls].class_info].virtual_bases_begin;
    return -24-std::int64_t(position)*8;
}
std::uint32_t Analyzer::prefix_subobject(std::uint32_t b, std::uint32_t id)
{
    ++subobject_work;
    auto object = subobjects[id];
    if (object.anchor) return id;
    if (bases[b].virtual_base) object.anchor = bases[b].base;
    else {
        auto k = key(b,object.path); auto path = subobject_path_index.get(k);
        if (!path) {
            path = subobject_paths.size(); SubobjectPath p; p.edge = b; p.next = object.path;
            subobject_paths.push_back(p); subobject_path_index.put(k,path);
        }
        object.path = path;
    }
    auto k = key(object.anchor,object.path); auto known = subobject_index.get(k);
    if (known) return known;
    auto result = subobjects.size(); subobjects.push_back(object);
    subobject_index.put(k,result); return result;
}
bool Analyzer::contains_subobject(EntityId outer, std::uint32_t from, std::uint32_t to)
{
    ++final_overrider_work;
    if (from == to) return true;
    auto a = subobjects[from], b = subobjects[to];
    // Every virtual anchor reachable from this occurrence denotes the same
    // subobject, even when its paths begin in distinct nonvirtual occurrences.
    if (b.anchor && virtual_base_index.get(key(outer,b.anchor))) return true;
    if (a.anchor != b.anchor) return false;
    auto p = a.path, q = b.path;
    while (p && q) {
        ++final_overrider_work;
        if (subobject_paths[p].edge != subobject_paths[q].edge) return false;
        p = subobject_paths[p].next; q = subobject_paths[q].next;
    }
    return !p;
}
void Analyzer::resolve_final_overriders(EntityId cls, VirtualClass& model)
{
    if (!class_facts[entities[cls].class_info].virtual_bases_count) return;
    Index winners, containment;
    auto dominates = [&](const VirtualSlot& a, const VirtualSlot& b) {
        // The class-local occurrence pair includes the receiver type: each
        // occurrence has exactly one class. Reuse this completed graph fact
        // across unrelated virtual signatures on the same receiver pair.
        auto k = key(a.implementation,b.implementation);
        if (auto known = containment.get(k)) return known == 2;
        bool result = contains_subobject(scopes[entities[a.function].owner].entity,a.implementation,b.implementation);
        containment.put(k,result ? 2 : 1); return result;
    };
    // A maximum, if one exists, survives this tournament. A second linear
    // pass proves it dominates every required candidate. Distinct occurrences
    // of the same member declaration are still distinct final overriders.
    for (unsigned i = 0; i < model.slots.size(); ++i) {
        const auto& slot = model.slots[i];
        auto k = key(slot.declaration,slot.origin); auto old = winners.get(k);
        ++final_overrider_work;
        if (!old || dominates(slot,model.slots[old-1])) winners.put(k,i+1);
    }
    for (const auto& slot : model.slots) {
        const auto& winner = model.slots[winners.get(key(slot.declaration,slot.origin))-1];
        if (!dominates(winner,slot)) throw std::runtime_error("virtual base has no unique final overrider");
    }
    for (auto& slot : model.slots) {
        auto winner = model.slots[winners.get(key(slot.declaration,slot.origin))-1];
        slot.function = winner.function; slot.receiver = winner.receiver;
        slot.implementation = winner.implementation;
    }
}
} }
