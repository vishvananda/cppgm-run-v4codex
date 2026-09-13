#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Expression Analyzer::template_statement_value(NodeId n, ScopeId s)
{
    if (expressions[n].ready) return expressions[n];
    // Query unsupported/dependent forms conservatively without creating an
    // object, demanding a body or replaying grammar. The query owns its result.
    struct Probe {
        bool& mode; bool previous; unsigned& depth;
        Probe(bool& m, unsigned& d) : mode(m), previous(m), depth(d) { mode = true; ++depth; }
        ~Probe() { mode = previous; --depth; }
    } probe(template_type_probe,unevaluated_depth);
    auto id = expression_query(n,s);
    if (!id) return Expression();
    auto result = query_fact(id);
    return result.dependent ? Expression() : result.expression;
}
void Analyzer::bind_template_condition(NodeId n, ScopeId s, bool is_switch)
{
    auto c = ast[n].first;
    if (!c) return;
    Expression value;
    if (ast[c].kind == Kind::ConditionDeclaration) {
        bind_template_declaration(c,s);
        auto d = ast[ast[c].first].next, entity = facts[d].entity;
        facts.edit(n).entity = entity;
        value.type = value_type(entities[entity].type); value.category = ValueCategory::Lvalue;
    } else {
        if (bind_template_expression(c,s)) return;
        value = template_statement_value(c,s);
    }
    if (!value.type || dependent_type(value.type)) return;
    auto target = condition_target(value,is_switch);
    if (is_switch) switches.back().type = target;
    ++unevaluated_depth;
    try {
        auto conversion = is_switch ? conversion_value(value,target) : boolean_conversion_value(value);
        check_fixed_conversion(value,0,conversion,s);
    } catch (...) { --unevaluated_depth; throw; }
    --unevaluated_depth;
}
void Analyzer::bind_template_return(NodeId n, ScopeId s)
{
    auto c = ast[n].first;
    bool dependent = c && bind_template_expression(c,s);
    if (!return_type || dependent_type(return_type)) return;
    if (!c) {
        if (!fundamental(return_type,FT_VOID)) throw std::runtime_error("missing return value");
        return;
    }
    if (dependent) return;
    auto value = template_statement_value(c,s);
    if (!value.type) return;
    if (fundamental(return_type,FT_VOID)) {
        if (!fundamental(value.type,FT_VOID)) throw std::runtime_error("value returned from void");
        return;
    }
    auto id = c;
    while (ast[id].kind == Kind::Parenthesized) id = ast[id].first;
    auto local = ast[id].kind == Kind::IdExpression ? expressions[id].entity : 0;
    auto function = s;
    while (function && scopes[function].kind != ScopeKind::Function) function = scopes[function].parent;
    bool eligible = class_value(return_type) && local &&
        (entities[local].kind == EntityKind::Variable || entities[local].kind == EntityKind::Parameter) &&
        !entities[local].is_static && !entities[local].external_decl && !(types[entities[local].type].cv & 2) &&
        class_value(entities[local].type) && encloses(function,entities[local].owner);
    ++unevaluated_depth;
    try {
        auto conversion = return_conversion(c,value,return_type,eligible);
        check_fixed_conversion(value,expressions[c].ready ? c : 0,conversion,s);
        if (class_value(return_type)) default_destructor(return_type,s,false);
        // Only fully fixed expression facts can consume this source recipe.
        // Query-only values and per-object lifetime decisions keep their owner.
        if (!ast.nodes.occurrences[c].context &&
            template_fixed_expressions.get(ast.nodes.occurrences[c].source)) {
            template_statement_conversions.put(ast.nodes.occurrences[c].source,conversions.size());
            conversions.push_back(conversion);
            ++statement_conversion_work;
        }
    } catch (...) { --unevaluated_depth; throw; }
    --unevaluated_depth;
}
} }
