#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
QueryId Analyzer::reduce_fold_query(TypeQuery q, const std::vector<QueryId>& operands)
{
    for (auto operand : operands)
        if (type_queries[operand].kind == QueryKind::Expansion) return intern_query(q,operands);
    if (operands.empty()) {
        TypeQuery identity;
        if (q.op == OP_LAND || q.op == OP_LOR) {
            identity.type = types.fundamental(FT_BOOL); identity.value = q.op == OP_LAND;
        } else if (q.op == OP_COMMA) identity.type = types.fundamental(FT_VOID);
        else return intern_query(q,{}); // Structured substitution failure.
        return intern_query(identity,{});
    }
    bool left = q.value;
    auto result = left ? operands.front() : operands.back();
    q.kind = QueryKind::Binary; q.value = 0;
    for (unsigned i = 1; i < operands.size(); ++i) {
        auto operand = operands[left ? i : operands.size()-1-i];
        result = intern_query(q,left ? std::vector<QueryId>{result,operand} : std::vector<QueryId>{operand,result});
        // Complete each step before extending the chain: linear work and no
        // pack-length recursion during type checking.
        query_fact(result);
    }
    TypeQuery paren; paren.kind = QueryKind::Parenthesized;
    return intern_query(paren,{result}); // decltype of a fold is always parenthesized.
}
QueryId Analyzer::fold_query(NodeId n, ScopeId s)
{
    auto occurrence = ast.nodes.occurrences[n];
    if (occurrence.context) {
        auto original = fold_source_queries.get(occurrence.source);
        if (original) {
            Index bindings, cache;
            return substitute_query(original,bindings,cache,template_type_contexts.get(occurrence.context));
        }
    }
    auto node = ast[n]; TypeQuery q; q.kind = QueryKind::Fold;
    q.op = node.op; q.context = s; q.name = q.op == OP_DOTSTAR ? 0 : operator_name(q.op);
    if (q.name && q.op != OP_ASS) {
        auto ordinary = lookup(s,q.name);
        if (function_binding(ordinary)) q.entity = ordinary;
    }
    std::vector<QueryId> operands; unsigned packs = 0;
    for (auto c = node.first; c; c = ast[c].next) {
        auto operand = expression_query(c,s);
        if (!operand) return 0;
        auto params = expansion_parameters(0x80000000U | operand);
        if (argument_packs[params].count) {
            ++packs; q.value = c != node.first || node.flags;
            TypeQuery expansion; expansion.kind = QueryKind::Expansion;
            operand = intern_query(expansion,{operand});
        }
        operands.push_back(operand);
    }
    if (packs != 1) throw std::runtime_error("fold requires exactly one unexpanded pack operand");
    auto result = intern_query(q,operands);
    if (!occurrence.context) fold_source_queries.put(occurrence.source,result);
    return result;
}
} }
