#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Expression Analyzer::fold_expression(NodeId n, ScopeId s)
{
    auto query = expression_query(n,s); auto fact = query_fact(query);
    if (fact.dependent) throw std::runtime_error("unexpanded fold expression");
    auto occurrence = ast.nodes.occurrences[n];
    auto original = type_queries[fold_source_queries.get(occurrence.source)];
    bool left = original.value;
    auto frame = template_type_contexts.get(occurrence.context);
    Index bindings; std::vector<NodeId> operands;
    for (auto child = ast[n].first; child; child = ast[child].next) {
        auto params = source_expansion_parameters(child);
        if (!argument_packs[params].count) { operands.push_back(child); continue; }
        auto count = expansion_count(params,bindings,frame);
        if (count < 0) throw std::runtime_error("fold has unresolved or mismatched packs");
        for (int i = 0; i < count; ++i) {
            auto lane = expansion_frame(frame,params,i);
            auto source = ast.instantiate(child,expansion_context(lane));
            facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
            operands.push_back(source);
        }
    }
    if (operands.empty()) {
        auto result = fact.expression; result.form = ExpressionForm::ConstantQuery;
        facts.edit(n).value = query_value(query); return result;
    }
    // The query owner has selected each operator once. Walk its reduction
    // spine iteratively, then attach evaluated storage/lifetime uses below.
    std::vector<QueryId> operations;
    auto current = query_edges[type_queries[query].offset];
    for (unsigned i = 1; i < operands.size(); ++i) {
        operations.push_back(current);
        auto q = type_queries[current]; current = query_edges[q.offset+(left ? 0 : 1)];
    }
    std::vector<unsigned> leaves;
    for (auto source : operands) {
        expression(source,s);
        FoldStep step; step.source = source; step.operation.result = expressions[source];
        if (!unevaluated_depth) observe_scalar(source,syntax::expression_precedence(ast[n].op) == 2);
        leaves.push_back(fold_steps.size()); fold_steps.push_back(step);
    }
    auto root = left ? leaves.front() : leaves.back();
    for (unsigned i = 1; i < leaves.size(); ++i) {
        auto other = leaves[left ? i : leaves.size()-1-i];
        auto id = operations[operations.size()-i]; auto selected = query_fact(id);
        FoldStep step; step.left = left ? root : other; step.right = left ? other : root;
        auto& op = step.operation; op.op = ast[n].op; op.function = selected.selected; op.result = selected.expression;
        std::vector<Expression> args{fold_steps[step.left].operation.result,fold_steps[step.right].operation.result};
        // Builtin comma retains the right value without operand conversions.
        if (!op.function && (op.op == OP_DOTSTAR || op.op == OP_ARROWSTAR)) {
            op.adjustment = object_uses[op.result.object_use].adjustment;
            size(object_uses[op.result.object_use].type);
        } else if (!op.function && op.op == OP_COMMA) {
            op.result = args[1]; op.result.conversions = op.result.count = 0;
        } else prepare_range_operation(op,args,s,!unevaluated_depth);
        root = fold_steps.size(); fold_steps.push_back(step);
    }
    fold_roots.put(n,root);
    auto result = fold_steps[root].operation.result;
    if (result.form == ExpressionForm::BoundMember) {
        auto use = object_uses[result.object_use]; use.member_pointer = n;
        // This source owns a typed reduction, including both final operands.
        // The callable consumer evaluates that plan once as a bound pair.
        use.adjustment = 0;
        result.object_use = object_uses.size(); object_uses.push_back(use);
    } else result.form = ExpressionForm::Ordinary;
    result.conversions = result.count = 0;
    result.arguments = result.argument_count = 0; result.incoming = 0;
    if (fold_steps[root].operation.temporary) {
        record_object(result,0,0,0);
        object_uses[result.object_use].temporary = fold_steps[root].operation.temporary;
    }
    return result;
}
} }
