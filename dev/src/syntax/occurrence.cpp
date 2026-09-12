#include "syntax/ast.h"
#include <stdexcept>
namespace cppgm { namespace syntax {
NodeId Ast::projected(NodeId source, std::uint32_t context) const
{
    if (!source || !context) return source;
    NodeId result = occurrence_index.get((std::uint64_t(context) << 32) | source);
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
    std::vector<NodeId> work(1,root);
    for (std::size_t i = 0; i < work.size(); ++i) {
        NodeId source = work[i];
        if (!source || projected(source,context)) continue;
        Node n = view(source);
        // Contexts refer to original source graph identities; nested demand
        // retains the enclosing specialization through its semantic environment.
        if (nodes.occurrences[source].context) throw std::logic_error("nested occurrence requires source pattern");
        NodeId id = nodes.occurrence(source,context);
        occurrence_index.put((std::uint64_t(context) << 32) | source,id);
        if (n.detail) work.push_back(n.detail);
        for (NodeId c = n.first; c; c = nodes[c].next) work.push_back(c);
        if (auto packing = class_packing.get(source)) class_packing.put(id,packing);
        if (auto attribute = alignment_owners.get(source)) {
            std::uint32_t first = 0, previous = 0;
            for (; attribute; attribute = alignments[attribute].next) {
                auto value = alignments[attribute]; value.next = 0;
                auto index = alignments.size(); alignments.push_back(value);
                if (previous) alignments[previous].next = index; else first = index;
                previous = index; work.push_back(value.operand);
            }
            alignment_owners.put(id,first);
        }
    }
    for (NodeId source : work) {
        auto id = projected(source,context);
        for (auto a = alignment_owners.get(id); a; a = alignments[a].next)
            if (nodes.occurrences[alignments[a].operand].context == 0)
                alignments[a].operand = projected(alignments[a].operand,context);
    }
    return projected(root,context);
}
} }
