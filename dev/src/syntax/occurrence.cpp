#include "syntax/ast.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
void Ast::resolve_paren_initializer(NodeId item, NodeId d, NodeId params, NodeId before)
{
    if (nodes.occurrences[d].context || paren_roles.get(nodes.occurrences[d].source))
        throw std::logic_error("declaration ambiguity resolved after publication");
    auto p = nodes[params].first, specs = nodes[p].first;
    auto name = nodes[nodes[specs].first].detail;
    auto index = paren_resolutions.size(); paren_resolutions.push_back({params,before,name});
    NodeId roles[] = {item,d,before,params,p,specs};
    for (unsigned j = 0; j < 6; ++j) paren_roles.put(nodes.occurrences[roles[j]].source,index*8+j);
}
Node Ast::source_view(NodeId id) const
{
    Node result = nodes[id];
    if (auto role = paren_roles.get(nodes.occurrences[id].source)) {
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
    NodeId result = occurrence_index.get((std::uint64_t(context) << 32) | nodes.occurrences[source].source);
    return result; // An edge outside this demanded source region is absent.
}
Node Ast::project_view(NodeId id) const
{
    Node result = paren_roles.empty() ? nodes[id] : source_view(id);
    auto context = nodes.occurrences[id].context;
    if (context) {
        result.first = projected(result.first,context); result.last = projected(result.last,context);
        result.next = projected(result.next,context); result.detail = projected(result.detail,context);
    }
    if (!expanded_first.empty()) {
        if (auto first = expanded_first.get(id)) result.first = first-1;
        if (auto last = expanded_last.get(id)) result.last = last-1;
        if (auto next = expanded_next.get(id)) result.next = next-1;
    }
    return result;
}
void Ast::expanded_children(NodeId parent, const std::vector<NodeId>& children)
{
    expanded_first.put(parent,children.empty() ? 1 : children.front()+1);
    expanded_last.put(parent,children.empty() ? 1 : children.back()+1);
    for (unsigned j = 0; j < children.size(); ++j)
        expanded_next.put(children[j],j+1 == children.size() ? 1 : children[j+1]+1);
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
    IdIndex seen;
    auto defer = [&](NodeId source) {
        if (seen.get(source)) return;
        seen.put(source,1); region_roots.push_back(source);
    };
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto source = work[i];
        if (!source || seen.get(source)) continue;
        seen.put(source,1); region_nodes.push_back(source);
        auto node = source_view(source);
        if (node.detail) work.push_back(node.detail);
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
    for (std::uint32_t j = 0; j < region.count; ++j) {
        auto n = region_nodes[region.begin+j];
        if (projected(n,context)) continue;
        auto id = nodes.occurrence(n,context);
        occurrence_index.put((std::uint64_t(context) << 32) | nodes.occurrences[n].source,id);
    }
    for (std::uint32_t j = 0; j < region.roots_count; ++j) {
        auto n = region_roots[region.roots_begin+j];
        if (projected(n,context)) continue;
        auto id = nodes.occurrence(n,context);
        occurrence_index.put((std::uint64_t(context) << 32) | nodes.occurrences[n].source,id);
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
