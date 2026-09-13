#include "syntax/ast.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
NodeId Ast::projected(NodeId source, std::uint32_t context) const
{
    if (!source || !context) return source;
    NodeId result = occurrence_index.get((std::uint64_t(context) << 32) | nodes.occurrences[source].source);
    return result; // An edge outside this demanded source region is absent.
}
Node Ast::project_view(NodeId id) const
{
    Node result = nodes[id];
    auto context = nodes.occurrences[id].context;
    if (context) {
        result.first = projected(result.first,context); result.last = projected(result.last,context);
        result.next = projected(result.next,context); result.detail = projected(result.detail,context);
    }
    return result;
}
NodeId Ast::instantiate(NodeId root, std::uint32_t context)
{
    struct Region { NodeId source; bool deferred; };
    std::vector<Region> work(1,Region{root,false});
    for (std::size_t i = 0; i < work.size(); ++i) {
        NodeId source = work[i].source;
        if (!source) continue;
        auto id = projected(source,context);
        bool created = !id;
        if (id) {
            if (work[i].deferred || !pending_region(id)) continue;
            deferred_occurrences.put(id,2); ++demanded_regions;
        }
        Node n = nodes[source];
        // Contexts refer to original source graph identities; nested demand
        // retains the enclosing specialization through its semantic environment.
        if (created) {
            id = nodes.occurrence(source,context);
            occurrence_index.put((std::uint64_t(context) << 32) | nodes.occurrences[source].source,id);
            // A function default can be demanded before its containing body.
            // Its root still owns the same region when that body is projected.
            if (!work[i].deferred && n.kind == Kind::DefaultArgument) {
                deferred_occurrences.put(id,2); ++deferred_regions; ++demanded_regions;
            }
        }
        if (work[i].deferred) {
            deferred_occurrences.put(id,1); ++deferred_regions;
            continue;
        }
        if (n.detail) work.push_back({n.detail,false});
        for (NodeId c = n.first; c; c = nodes[c].next) {
            auto kind = nodes[c].kind;
            bool body = (n.kind == Kind::Function || n.kind == Kind::SpecialDefinition) &&
                (kind == Kind::Compound || kind == Kind::FunctionTry || kind == Kind::CtorInitializer);
            work.push_back({c,body || kind == Kind::DefaultArgument});
        }
        if (!created) continue;
        if (auto packing = class_packing.get(source)) class_packing.put(id,packing);
        if (auto attribute = alignment_owners.get(source)) {
            std::uint32_t first = 0, previous = 0;
            for (; attribute; attribute = alignments[attribute].next) {
                auto value = alignments[attribute]; value.next = 0;
                auto index = alignments.size(); alignments.push_back(value);
                if (previous) alignments[previous].next = index; else first = index;
                previous = index; work.push_back({value.operand,false});
            }
            alignment_owners.put(id,first);
        }
    }
    for (auto region : work) {
        auto id = projected(region.source,context);
        for (auto a = alignment_owners.get(id); a; a = alignments[a].next)
            if (nodes.occurrences[alignments[a].operand].context != context)
                alignments[a].operand = projected(alignments[a].operand,context);
    }
    return projected(root,context);
}
} }
