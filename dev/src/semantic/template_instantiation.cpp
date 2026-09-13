#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::demand_region(NodeId root)
{
    if (!root || !ast.pending_region(root)) return;
    auto context = ast.nodes.occurrences[root].context;
    ast.instantiate(root,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
}
void Analyzer::instantiate_function(EntityId e)
{
    auto index = entities[e].specialization;
    if (!index || specializations[index].body != FactState::NotStarted) return;
    auto spec = specializations[index];
    auto pattern = templates[entities[spec.pattern].template_info];
    if (!pattern.body) return;
    specializations[index].body = FactState::Active; ++template_bodies;
    auto context = spec.context ? spec.context : ast.new_context();
    auto source = ast.instantiate(pattern.source,context);
    auto declarator = ast.projected(pattern.declarator,context);
    auto body = ast.projected(pattern.body,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    ScopeId environment = specialization_environment(e);
    specializations[index].context = context;
    specializations[index].environment = environment;
    // The source signature already owns typed parameter declarations. Substitute
    // those facts, preserving body cv/array/function forms independently of the
    // adjusted callable type; body demand must not rebuild the signature syntax.
    Index bindings, cache;
    auto frame = substitution_frame(index,pattern.offset,pattern.count);
    attach_template_context(context,frame);
    NodeId parameters = 0, d = pattern.declarator;
    while (d) {
        if (auto p = child(d,syntax::Kind::Parameters)) parameters = p;
        auto nested = child(d,syntax::Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != syntax::Kind::Parameter) continue;
        auto type = facts[p].type;
        if (!type) throw std::logic_error("missing retained template parameter type");
        auto concrete = substitute_type(type,bindings,cache,frame);
        if (!concrete) throw std::runtime_error("invalid instantiated parameter type");
        auto occurrence = ast.projected(p,context);
        facts[occurrence].type = concrete; facts[occurrence].scope = environment;
    }
    function_body({body,declarator,environment,e,source});
    specializations[index].body = FactState::Success;
}
NodeId Analyzer::instantiate_default(EntityId e, unsigned parameter)
{
    auto index = entities[e].specialization;
    auto spec = specializations[index];
    auto context = spec.context ? spec.context : ast.new_context();
    specializations[index].context = context;
    auto pattern = spec.pattern;
    NodeId source = default_arguments[entities[pattern].defaults+parameter];
    NodeId value = ast.instantiate(source,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    auto environment = specialization_environment(e);
    expression(value,environment);
    return value;
}
} }
