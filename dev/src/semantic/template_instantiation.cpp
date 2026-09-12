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
    specializations[index].body = FactState::Active;
    auto context = ast.new_context();
    auto source = ast.instantiate(pattern.source,context);
    auto declarator = ast.projected(pattern.declarator,context);
    auto body = ast.projected(pattern.body,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    ScopeId environment = make_scope(ScopeKind::Template,scopes[pattern.environment].parent);
    auto pack = argument_packs[spec.arguments];
    for (unsigned j = 0; j < pack.count; ++j) {
        auto parameter = template_parameters[pattern.offset+j];
        auto alias = make_entity(EntityKind::Alias,environment,entities[parameter].name,0);
        entities[alias].type = argument_types[pack.offset+j];
        bind(environment,entities[alias].name,alias);
    }
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
} }
