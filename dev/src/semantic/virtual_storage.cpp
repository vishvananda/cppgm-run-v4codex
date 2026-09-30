#include "semantic/analyzer.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace cppgm { namespace semantic {
namespace {
std::uint64_t add(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max()-a) throw std::runtime_error("class size overflow");
    return a+b;
}
}
void Analyzer::layout_host_virtual_bases(EntityId cls, std::uint64_t& cursor, std::uint64_t& alignment)
{
    // Allocation follows the first primary claimant in inheritance preorder,
    // not allocation order. A later-selected primary may itself lose sharing
    // of one of its virtual primaries to an earlier virtual base occurrence.
    struct Visit { EntityId type; unsigned object; };
    std::vector<Visit> pending(1,Visit{cls,0});
    Index visited, claims;
    while (!pending.empty()) {
        auto item = pending.back(); pending.pop_back();
        auto identity = key(item.type,item.object);
        if (visited.get(identity)) continue;
        visited.put(identity,1); ++virtual_base_work;
        if (item.type != cls) size(entities[item.type].type);
        auto info = entities[item.type].class_info;
        auto primary = class_facts[info].primary_base;
        if (primary && bases[primary].virtual_base && !claims.get(bases[primary].base))
            claims.put(bases[primary].base,item.object+1);
        std::vector<Visit> children;
        for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
            if (!dynamic_class(bases[b].base)) continue;
            children.push_back({bases[b].base,compose_subobject(item.object,prefix_subobject(b,0))});
        }
        pending.insert(pending.end(),children.rbegin(),children.rend());
    }
    Index resolved;
    const auto& order = virtual_class(cls).base_order;
    for (auto base : order) {
        if (claims.get(base)) continue;
        size(entities[base].type);
        const auto& facts = class_facts[entities[base].class_info];
        auto remainder = cursor % facts.nonvirtual_alignment;
        auto offset = remainder ? add(cursor,facts.nonvirtual_alignment-remainder) : cursor;
        virtual_base_offsets[virtual_base_index.get(key(cls,base))-1] = offset;
        resolved.put(base,1);
        cursor = add(offset,facts.nonvirtual_size);
        alignment = std::max(alignment,facts.nonvirtual_alignment);
    }
    // A claimant's offset depends only on its virtual anchor and fixed tail.
    // Memoize each anchor and path suffix once; never retry all pending bases.
    Index tails, active;
    auto tail_offset = [&](unsigned path) {
        std::vector<unsigned> pending_paths;
        auto p = path;
        while (p && !tails.get(p)) { pending_paths.push_back(p); p = subobject_paths[p].next; }
        std::uint64_t value = p ? tails.get(p)-1 : 0;
        for (auto i = pending_paths.rbegin(); i != pending_paths.rend(); ++i) {
            value = add(value,bases[subobject_paths[*i].edge].offset);
            tails.put(*i,add(value,1)); ++virtual_base_work;
        }
        return path ? tails.get(path)-1 : 0;
    };
    for (auto base : order) {
        std::vector<EntityId> chain;
        auto current = base;
        while (current && !resolved.get(current)) {
            if (active.get(current)) throw std::logic_error("cyclic virtual primary allocation");
            active.put(current,1); chain.push_back(current);
            auto owner = claims.get(current);
            if (!owner) throw std::logic_error("missing virtual primary claimant");
            current = subobjects[owner-1].anchor;
        }
        for (auto i = chain.rbegin(); i != chain.rend(); ++i) {
            auto object = subobjects[claims.get(*i)-1];
            auto offset = add(object.anchor ? virtual_base_offset(cls,object.anchor) : 0,tail_offset(object.path));
            virtual_base_offsets[virtual_base_index.get(key(cls,*i))-1] = offset;
            resolved.put(*i,1); ++virtual_base_work;
        }
    }
}
} }
