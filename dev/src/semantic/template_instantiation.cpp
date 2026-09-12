#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
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
    // The declarator's source parameter types retain top-level cv for the body;
    // the canonical callable signature separately owns its adjusted parameter types.
    NodeId specs = ast[source].first;
    declarator = ast.projected(pattern.declarator,context);
    this->declarator(declarator,specifiers(specs,environment),environment);
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
