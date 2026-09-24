#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::explicit_specifier(EntityId e, NodeId source, ScopeId scope)
{
    auto specs = child(source,syntax::Kind::MemberSpecifiers);
    for (auto spec = ast[specs].first; spec; spec = ast[spec].next) {
        if (ast[spec].op != KW_EXPLICIT) continue;
        auto member = entities[e].member_info;
        if (!ast[spec].first) { members[member].explicit_constructor = true; continue; }
        auto operand = ast[spec].first;
        if (definitions && !ast.nodes.occurrences[operand].context)
            bind_template_expression(operand,scope);
        auto query = expression_query(operand,scope);
        if (!query) throw std::logic_error("missing explicit condition query");
        bool dependent = query_fact(query).dependent;
        members[member].explicit_condition = dependent ? query : 0;
        if (!dependent)
            members[member].explicit_constructor = explicit_condition_value(query);
    }
}
bool Analyzer::explicit_condition_value(QueryId query)
{
    auto conversion = boolean_conversion_value(query_fact(query).expression);
    if (!conversion.valid()) throw std::runtime_error("invalid explicit condition conversion");
    auto value = constant_query_conversion(query,conversion);
    if (!value.valid)
        throw std::runtime_error("explicit condition is not a constant boolean");
    return constant_truth(value);
}

} }
