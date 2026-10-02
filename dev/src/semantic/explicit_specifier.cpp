#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::explicit_specifier(EntityId e, NodeId source, ScopeId scope)
{
    auto specs = child(source,syntax::Kind::MemberSpecifiers);
    for (auto spec = ast.first(specs); spec; spec = ast.next(spec)) {
        if (ast.op(spec) != KW_EXPLICIT) continue;
        auto member = entities[e].member_info;
        if (!ast.first(spec)) { members[member].explicit_constructor = true; continue; }
        auto operand = ast.first(spec);
        if (definitions && !ast.nodes.occurrences[operand].context)
            bind_template_expression(operand,scope);
        auto query = expression_query(operand,scope);
        if (!query) throw std::logic_error("missing explicit condition query");
        bool dependent = query_fact(query).dependent;
        members[member].explicit_condition = dependent ? query : 0;
        if (!dependent)
            members[member].explicit_constructor = explicit_condition_value(query,scope);
    }
}
Constant Analyzer::explicit_condition(QueryId query, ScopeId scope)
{
    EvaluationScope evaluation(*this,true);
    auto conversion = boolean_conversion_value(query_fact(query).expression);
    if (!valid_fixed_conversion(query_fact(query).expression,0,conversion,scope)) return Constant();
    return constant_query_conversion(query,conversion);
}
bool Analyzer::explicit_condition_value(QueryId query, ScopeId scope)
{
    auto value = explicit_condition(query,scope);
    if (!value.valid)
        throw std::runtime_error("explicit condition is not a constant boolean");
    return constant_truth(value);
}

} }
