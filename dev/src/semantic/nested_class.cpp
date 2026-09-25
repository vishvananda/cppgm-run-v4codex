#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::complete_nested_class(EntityId e)
{
    auto id = entities[e].class_info;
    auto info = class_facts[id];
    if (info.definition_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::ClassDefinition,e,info.definition_source);
    if (info.definition_state != FactState::NotStarted) return;
    class_facts[id].definition_state = FactState::Active;
    auto saved_template = active_template_scope, saved_depth = class_depth;
    auto saved_bodies = bodies.size(), saved_defaults = declaration_defaults.size();
    auto saved_exceptions = declaration_exceptions.size();
    try {
        active_template_scope = 0;
        demand_region(info.definition_source);
        ++nested_class_definitions;
        class_attributes(info.definition_source,info.definition_scope,e,true);
        define_class(info.definition_source,info.definition_scope,e,entities[e].owner,false,false);
        class_facts[id].definition_state = FactState::Success;
        active_template_scope = saved_template;
    } catch (...) {
        class_facts[id].definition_state = FactState::Failure;
        entities[e].complete = false;
        active_template_scope = saved_template; class_depth = saved_depth;
        bodies.resize(saved_bodies); declaration_defaults.resize(saved_defaults);
        declaration_exceptions.resize(saved_exceptions);
        throw;
    }
}
} }
