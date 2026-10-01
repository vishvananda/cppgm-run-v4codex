#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::deduce_return(NodeId n, ScopeId s)
{
    auto e = current_function, pattern = placeholder_returns.get(e);
    auto source = ast[n].first;
    if (ast[source].kind == syntax::Kind::BracedInit)
        throw std::runtime_error("cannot deduce return type from braced list");
    Expression value;
    if (source) value = expression(source,s);
    else value.type = types.fundamental(FT_VOID);
    TypeId deduction = 0;
    auto result = deduce_placeholder(pattern,value,deduction);
    auto f = types[entities[e].type];
    if (!placeholder_type(f.child) && f.child != result)
        throw std::runtime_error("inconsistent deduced return types");
    if (placeholder_type(f.child)) {
        ++return_deductions;
        std::vector<TypeId> parameters(types.parameters.begin()+f.offset,types.parameters.begin()+f.offset+f.count);
        entities[e].type = types.function(result,parameters,f.variadic,f.cv,f.ref);
        if (entities[e].member_info) member_facts(e);
        facts.edit(entities[e].definition).type = entities[e].type;
    }
    return_type = result;
}
void Analyzer::finish_deduced_return(EntityId e)
{
    if (!placeholder_returns.get(e)) return;
    if (placeholder_type(types[entities[e].type].child)) deduce_return(0,entities[e].scope);
    check_constexpr_signature(e);
}
void Analyzer::require_deduced_return(EntityId e)
{
    if (placeholder_returns.get(e)) ++return_deduction_uses;
    if (!placeholder_type(types[entities[e].type].child)) return;
    if (entities[e].template_info || entities[e].template_pattern) return;
    if (entities[e].body_state == FactState::Active)
        throw std::runtime_error("function used before return deduction");
    if (entities[e].body_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::FunctionDefinition,e,entities[e].source);
    struct DeclarationOnly {
        unsigned& depth; bool active;
        DeclarationOnly(unsigned& d, bool a):depth(d),active(a) { if (active) ++depth; }
        ~DeclarationOnly() { if (active) --depth; }
    } declaration_only(unevaluated_depth,discarded_statement());
    // Return deduction is a semantic demand even in an unevaluated context.
    // The ordinary body state owns recursion/failure, and dormant use edges
    // preserve the distinction between checking a body and emitting it.
    auto saved_probe = template_type_probe, saved_immediate = immediate_query_probe;
    template_type_probe = immediate_query_probe = false;
    try {
        if (entities[e].specialization) instantiate_function(e);
        else if (entities[e].member_info) {
            instantiate_member_definition(e);
            auto m = members[entities[e].member_info];
            if (m.body && !entities[e].definition)
                function_body({m.body,m.declarator,m.body_environment ? m.body_environment : entities[e].owner,e,m.source});
        }
    } catch (...) {
        template_type_probe = saved_probe; immediate_query_probe = saved_immediate; throw;
    }
    template_type_probe = saved_probe; immediate_query_probe = saved_immediate;
    if (placeholder_type(types[entities[e].type].child))
        throw std::runtime_error("return deduction requires a visible definition");
}
} }
