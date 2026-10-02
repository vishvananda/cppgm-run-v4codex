#include "syntax/ast.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
namespace {
std::size_t occurrence_hash(NodeId source)
{
    std::uint32_t x = source;
    x ^= x >> 16; x *= 0x7feb352dU;
    x ^= x >> 15; x *= 0x846ca68bU;
    return x ^ (x >> 16);
}
}
std::size_t NodePool::index_bytes() const
{
    auto bytes = indexes.capacity()*sizeof(ContextIndex) + recent.capacity()*sizeof(NodeId);
    for (const auto& index : indexes) bytes += index.slots.capacity()*sizeof(NodeId);
    return bytes;
}
NodeId NodePool::projected(NodeId source, std::uint32_t context) const
{
    source = occurrences[source].source;
    auto known = recent[source];
    if (known && occurrences[known].context == context) return known;
    if (context >= indexes.size()) return 0;
    const auto& index = indexes[context].slots;
    if (index.empty()) return 0;
    auto mask = index.size()-1;
    auto slot = occurrence_hash(source)&mask;
    while (auto id = index[slot]) {
        const auto& entry = occurrences[id];
        if (entry.source == source) {
            recent[source] = id; return id;
        }
        slot = (slot+1)&mask;
    }
    return 0;
}
void NodePool::reserve_occurrences(std::uint32_t context, std::size_t additional)
{
    if (indexes.size() <= context) indexes.resize(context+1);
    auto& map = indexes[context];
    auto& index = map.slots;
    auto capacity = index.empty() ? 8 : index.size();
    while ((map.used+additional)*2 >= capacity) capacity *= 2;
    if (capacity == index.size()) return;
    std::vector<NodeId> old;
    old.swap(index); index.resize(capacity);
    auto mask = capacity-1;
    for (auto id : old) if (id) {
        auto slot = occurrence_hash(occurrences[id].source)&mask;
        while (index[slot]) slot = (slot+1)&mask;
        index[slot] = id;
    }
}
NodeId NodePool::occurrence(NodeId source, std::uint32_t context)
{
    source = occurrences[source].source;
    auto known = recent[source];
    if (known && occurrences[known].context == context) return known;
    if (indexes.size() <= context) indexes.resize(context+1);
    auto& map = indexes[context];
    auto& index = map.slots;
    auto hash = occurrence_hash(source);
    auto slot = index.empty() ? 0 : hash & (index.size()-1);
    while (!index.empty() && index[slot]) {
        auto id = index[slot];
        const auto& entry = occurrences[id];
        if (entry.source == source) {
            recent[source] = id; return id;
        }
        slot = (slot+1)&(index.size()-1);
    }
    if (index.empty() || (map.used+1)*2 >= index.size()) {
        reserve_occurrences(context,1);
        auto mask = index.size()-1;
        slot = hash & mask;
        while (index[slot]) slot = (slot+1)&mask;
    }
    NodeId id = occurrences.size();
    occurrences.push_back({source,context});
    index[slot] = id;
    ++map.used;
    recent[source] = id;
    return id;
}
void Ast::resolve_source_node(NodeId id, Node node)
{
    if (nodes.occurrences[id].context)
        throw std::logic_error("source grammar interpretation during instantiation");
    auto source = nodes.occurrences[id].source;
    if (source/64 < published_source.size() && (published_source[source/64] & (std::uint64_t(1) << (source%64))))
        throw std::logic_error("source grammar interpretation after publication");
    nodes[id] = node;
}
void Ast::resolve_paren_initializer(NodeId item, NodeId d, NodeId params, NodeId before)
{
    if (nodes.occurrences[d].context || paren_role(nodes.occurrences[d].source))
        throw std::logic_error("declaration ambiguity resolved after publication");
    auto p = nodes[params].first, specs = nodes[p].first;
    auto name = nodes[nodes[specs].first].detail;
    auto index = paren_resolutions.size(); paren_resolutions.push_back({params,before,name});
    NodeId roles[] = {item,d,before,params,p,specs};
    for (unsigned j = 0; j < 6; ++j) {
        auto source = nodes.occurrences[roles[j]].source;
        if (paren_roles.size() <= source) paren_roles.resize(source+1);
        paren_roles[source] = index*8+j;
    }
}
Node Ast::source_view(NodeId id) const
{
    Node result = nodes[id];
    if (auto role = paren_role(nodes.occurrences[id].source)) {
        auto r = paren_resolutions[role/8];
        switch (role%8) {
        case 0: result.last = r.parameters; break;
        case 1: result.last = r.before; result.next = r.parameters; break;
        case 2: result.next = 0; break;
        case 3: result.kind = Kind::Initializer; break;
        case 4: result.kind = Kind::ParenInitializer; break;
        case 5: result.kind = Kind::IdExpression; result.detail = r.name; result.first = result.last = 0; break;
        }
    }
    return result;
}
NodeId Ast::projected(NodeId source, std::uint32_t context) const
{
    if (!source || !context) return source;
    return nodes.projected(source,context); // An edge outside this demanded source region is absent.
}
Node Ast::project_view(NodeId id) const
{
    Node result = paren_roles.empty() ? nodes[id] : source_view(id);
    auto context = nodes.occurrences[id].context;
    if (context) {
        result.first = projected(result.first,context); result.last = projected(result.last,context);
        result.next = projected(result.next,context); result.detail = projected(result.detail,context);
    }
    if (has_expansion(id)) {
        if (auto first = expanded_first.get(id)) result.first = first-1;
        if (auto last = expanded_last.get(id)) result.last = last-1;
        if (auto next = expanded_next.get(id)) result.next = next-1;
    }
    return result;
}
NodeId Ast::edge(NodeId id, unsigned which) const
{
    // Traversals needing one edge do not project the other three.
    if (which != 3 && has_expansion(id)) {
        auto expanded = which == 0 ? expanded_first.get(id) : which == 1 ? expanded_last.get(id) : expanded_next.get(id);
        if (expanded) return expanded-1;
    }
    auto node = paren_roles.empty() ? nodes[id] : source_view(id);
    auto source = which == 0 ? node.first : which == 1 ? node.last : which == 2 ? node.next : node.detail;
    return projected(source,nodes.occurrences[id].context);
}
void Ast::expanded_children(NodeId parent, const std::vector<NodeId>& children)
{
    auto mark = [&](NodeId id) {
        if (expanded_nodes.size() <= id/64) expanded_nodes.resize(id/64+1);
        expanded_nodes[id/64] |= std::uint64_t(1) << (id%64);
    };
    mark(parent);
    expanded_first.put(parent,children.empty() ? 1 : children.front()+1);
    expanded_last.put(parent,children.empty() ? 1 : children.back()+1);
    for (unsigned j = 0; j < children.size(); ++j) {
        mark(children[j]);
        expanded_next.put(children[j],j+1 == children.size() ? 1 : children[j+1]+1);
    }
}
std::uint32_t Ast::source_region(NodeId root)
{
    auto key = nodes.occurrences[root].source;
    if (auto known = source_region_index.get(key)) return known-1;
    SourceRegion region{std::uint32_t(region_nodes.size()),0,
        std::uint32_t(region_roots.size()),0,std::uint32_t(region_metadata.size()),0};
    // Source topology is immutable after source ambiguity resolution. This cache retains IDs and
    // attribute references, never a second syntax tree or semantic decisions.
    auto& work = projection_work; work.clear(); work.push_back(root);
    IdIndex seen, template_classes;
    auto defer = [&](NodeId source) {
        if (seen.get(source)) return;
        seen.put(source,1); region_roots.push_back(source);
    };
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto source = work[i];
        if (!source || seen.get(source)) continue;
        seen.put(source,1); region_nodes.push_back(source);
        auto identity = nodes.occurrences[source].source;
        if (published_source.size() <= identity/64) published_source.resize(identity/64+1);
        published_source[identity/64] |= std::uint64_t(1) << (identity%64);
        auto node = source_view(source);
        if (node.detail) work.push_back(node.detail);
        if (node.kind == Kind::Template)
            template_classes.put(source_view(node.first).next,1);
        if (node.kind == Kind::Class && node.detail && source != root && !template_classes.get(source)) {
            // A nested class declaration needs its name and class-key, but
            // no concrete occurrences of its bases, attributes or members.
            // Demand of this same root later projects its definition once.
            work.push_back(node.first);
            region_roots.push_back(source);
            continue;
        }
        bool function = node.kind == Kind::Function || node.kind == Kind::SpecialDefinition;
        if (function || node.kind == Kind::Parameter) {
            for (auto c = node.first; c; c = source_view(c).next) {
                auto kind = source_view(c).kind;
                bool body = function && (kind == Kind::Compound || kind == Kind::FunctionTry || kind == Kind::CtorInitializer);
                if (body || kind == Kind::DefaultArgument) defer(c);
                else work.push_back(c);
            }
        } else for (auto c = node.first; c; c = source_view(c).next) work.push_back(c);
        auto packing = class_packing.get(source), alignment = alignment_owners.get(source);
        if (packing || alignment) region_metadata.push_back({source,packing,alignment});
        for (auto a = alignment; a; a = alignments[a].next) work.push_back(alignments[a].operand);
    }
    region.count = region_nodes.size()-region.begin;
    region.roots_count = region_roots.size()-region.roots_begin;
    region.metadata_count = region_metadata.size()-region.metadata_begin;
    auto id = source_regions.size(); source_regions.push_back(region);
    source_region_index.put(key,id+1); return id;
}
NodeId Ast::instantiate(NodeId root, std::uint32_t context)
{
    if (!root || !context) return root;
    auto existing = projected(root,context);
    auto pending = existing ? deferred_occurrences.get(existing) : 0;
    if (existing && pending <= 1) return existing;
    auto source = pending > 1 ? pending-1 : root;
    auto region = source_regions[source_region(source)];
    // One demanded region arrives as a batch. Avoid repeatedly growing and
    // rehashing this context's index while publishing its known source IDs.
    nodes.reserve_occurrences(context,std::size_t(region.count)+region.roots_count);
    for (std::uint32_t j = 0; j < region.count; ++j) {
        auto n = region_nodes[region.begin+j];
        nodes.occurrence(n,context);
    }
    for (std::uint32_t j = 0; j < region.roots_count; ++j) {
        auto n = region_roots[region.roots_begin+j];
        auto id = projected(n,context);
        if (id && deferred_occurrences.get(id)) continue;
        if (!id) {
            id = nodes.occurrence(n,context);
        }
        // Parsed source IDs exclude the reserved zero and maximum IDs.
        deferred_occurrences.put(id,n+1); ++deferred_regions;
    }
    for (std::uint32_t j = 0; j < region.metadata_count; ++j) {
        auto metadata = region_metadata[region.metadata_begin+j];
        auto id = projected(metadata.source,context);
        if (metadata.packing) class_packing.put(id,metadata.packing);
        if (!metadata.alignment || alignment_owners.get(id)) continue;
        std::uint32_t first = 0, previous = 0;
        for (auto a = metadata.alignment; a; a = alignments[a].next) {
            auto value = alignments[a]; value.next = 0;
            value.operand = projected(value.operand,context);
            auto index = alignments.size(); alignments.push_back(value);
            if (previous) alignments[previous].next = index; else first = index;
            previous = index;
        }
        alignment_owners.put(id,first);
    }
    auto id = projected(root,context);
    if (pending > 1) { deferred_occurrences.put(id,1); ++demanded_regions; }
    else if (nodes[source].kind == Kind::DefaultArgument) {
        // A default can be demanded before its containing function body.
        deferred_occurrences.put(id,1); ++deferred_regions; ++demanded_regions;
    }
    return id;
}
} }
