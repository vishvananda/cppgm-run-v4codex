#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
QueryId Analyzer::cast_query(NodeId n, ScopeId scope)
{
    auto node = ast[n]; auto first = node.first;
    if (node.op != OP_LPAREN && node.op != KW_STATIC_CAST &&
        node.op != KW_CONST_CAST && node.op != KW_REINTERPET_CAST && node.op != KW_DYNAMIC_CAST) {
        if (template_type_probe) return 0;
        throw std::runtime_error("cast is not supported in a constant type query");
    }
    TypeQuery query; query.kind = QueryKind::Cast; query.op = node.op;
    query.type = type_id(first,scope); query.context = scope;
    auto operand = ast.next(first); auto child = expression_query(operand,scope);
    if (node.op == OP_LPAREN && ast.kind(operand) == syntax::Kind::BracedInit)
        query.value = 1; // Source compound literal, distinct from semantic list formation.
    if (template_type_probe) {
        if (!child) return 0;
        if (!query.value) {
            if (!query.type) return 0;
            auto fact = query_fact(child);
            if (!fact.expression.type && !fact.dependent) return 0;
        }
    }
    return intern_query(query,{child});
}
} }
