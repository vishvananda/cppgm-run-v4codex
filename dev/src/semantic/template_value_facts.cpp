#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::bind_template_size(NodeId node, ScopeId scope)
{
    auto source = ast.nodes.occurrences[node].source;
    if (template_value_queries.get(source)) return true;
    struct Probe {
        bool& mode; unsigned& depth; bool prior;
        Probe(bool& m, unsigned& d) : mode(m), depth(d), prior(m) { mode = true; ++depth; }
        ~Probe() { mode = prior; --depth; }
    } probe(template_type_probe,unevaluated_depth);
    auto query = expression_query(node,scope);
    if (!query) return false; // Local identity or another expression owner is not yet typed.
    template_value_queries.put(source,query); ++template_value_work;
    Expression result; result.type = types.fundamental(FT_UNSIGNED_LONG_INT); result.ready = true;
    expressions[node] = result; facts[node].type = result.type; facts[node].scope = scope;
    template_fixed_expressions.put(source,node); ++template_fixed_work;
    return true;
}
std::uint32_t Analyzer::query_value(QueryId id)
{
    auto slot = query_value_index.get(id);
    if (!slot) {
        slot = query_values.size(); query_values.push_back(QueryValue()); query_value_index.put(id,slot);
    }
    auto state = query_values[slot].state;
    if (state == FactState::Success) return query_values[slot].constant;
    if (state == FactState::Active) throw std::runtime_error("recursive constant query");
    if (state == FactState::Failure) throw std::runtime_error("failed constant query");
    query_values[slot].state = FactState::Active; ++query_value_work;
    try {
        auto fact = query_fact(id);
        if (fact.dependent) throw std::logic_error("dependent query demanded as a concrete value");
        auto query = type_queries[id]; Constant value;
        if (query.kind == QueryKind::Sizeof) {
            auto type = query.type ? query.type : query_fact(query_edges[query.offset]).expression.type;
            value = Constant(types.fundamental(FT_UNSIGNED_LONG_INT),size(type,query.op == KW_ALIGNOF));
        } else throw std::logic_error("missing typed constant query operation");
        auto constant = constants.size(); constants.push_back(value);
        query_values[slot].constant = constant; query_values[slot].state = FactState::Success;
        return constant;
    } catch (...) { query_values[slot].state = FactState::Failure; throw; }
}
bool Analyzer::reuse_template_value(NodeId node, ScopeId scope, Expression& result)
{
    auto occurrence = ast.nodes.occurrences[node];
    auto source = template_value_queries.get(occurrence.source);
    if (!source) return false;
    auto frame = template_type_contexts.get(occurrence.context);
    if (!frame) throw std::logic_error("missing body value substitution frame");
    Index bindings, cache;
    auto query = substitute_query(source,bindings,cache,frame);
    if (!query) throw std::runtime_error("invalid substituted body value query");
    auto value = query_value(query); ++template_value_uses;
    result.type = constants[value].type;
    facts[node].type = result.type; facts[node].scope = scope; facts[node].value = value;
    return true;
}
} }
