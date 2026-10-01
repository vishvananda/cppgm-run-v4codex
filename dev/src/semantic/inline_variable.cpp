#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::variable_expression_type(EntityId e)
{
    auto type = types[entities[e].type];
    if (type.kind == TypeKind::Array && !type.bound && inline_variable_definitions.get(e)) {
        // Completing this type requires the member definition, including its
        // initialization effects, even when the requesting operand is sizeof.
        entities[e].emission |= Entity::Used;
        initialize_inline_variable(e);
        demand_inline_initializer(e);
    }
    return entities[e].type;
}
void Analyzer::initialize_inline_variable(EntityId e)
{
    auto index = inline_variable_definitions.get(e);
    if (!index) return;
    auto def = inline_variable_recipes[index];
    if (def.state == FactState::Success) { ++inline_variable_hits; return; }
    if (def.state == FactState::Active) return; // The declaration is visible in its own initializer.
    if (def.state == FactState::Failure) throw FailedSemanticFact(SemanticFact::InitializerBinding,e,def.declarator);
    inline_variable_recipes[index].state = FactState::Active; ++inline_variable_initializers;
    // A static member initializer is a separate definition. The requesting
    // sizeof, discarded statement, function/default, and invocation site do
    // not become its evaluation context. Inner unevaluated operands still
    // establish their own depth while checking this retained source region.
    struct DefinitionContext {
        Analyzer& sem; unsigned depth, default_fact, initializer; EntityId function;
        SourceInvocation invocation;
        DefinitionContext(Analyzer& s, unsigned id):sem(s),depth(s.unevaluated_depth),
            default_fact(s.active_default_fact),initializer(s.active_inline_initializer),function(s.current_function),invocation(s.source_invocation) {
            s.unevaluated_depth = 1; s.active_default_fact = 0; s.active_inline_initializer = id;
            s.current_function = 0; s.source_invocation = SourceInvocation();
        }
        ~DefinitionContext() {
            sem.unevaluated_depth = depth; sem.active_default_fact = default_fact;
            sem.active_inline_initializer = initializer;
            sem.current_function = function; sem.source_invocation = invocation;
        }
    } context(*this,index);
    auto saved = access_override;
    access_override = entities[e].owner;
    try {
        auto init = entities[e].initializer;
        if (init) {
            auto source = init;
            while (ast[source].kind == Kind::Initializer) source = ast[source].first;
            expand_expression_list(source,def.scope);
        }
        auto type = entities[e].type;
        if (init && types[type].kind == TypeKind::Array && !types[type].bound) {
            type = complete_array_initializer(init,type,def.scope);
            entities[e].type = types.signature(type); facts.edit(def.declarator).type = type;
        }
        if (spec_has(def.specifiers,KW_CONSTEXPR) && !literal_type(type))
            throw std::runtime_error("constexpr variable requires a literal type");
        finish_object_initializer(e,init,def.declarator,def.specifiers,def.scope,def.scope,entities[e].type,false);
        inline_variable_recipes[index].state = FactState::Success;
    } catch (...) {
        access_override = saved; inline_variable_recipes[index].state = FactState::Failure; throw;
    }
    access_override = saved;
}
void Analyzer::demand_inline_initializer(EntityId e)
{
    auto index = inline_variable_definitions.get(e);
    if (!index || inline_variable_recipes[index].state == FactState::Active) return;
    auto def = inline_variable_recipes[index];
    if (def.demand == FactState::Success || def.demand == FactState::Active) return;
    if (def.demand == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::InitializerBinding,e,def.declarator);
    inline_variable_recipes[index].demand = FactState::Active; ++inline_variable_demands;
    auto depth = unevaluated_depth; auto function = current_function;
    auto owner = active_inline_initializer; auto defaults = active_default_fact;
    unevaluated_depth = 0; current_function = 0;
    active_inline_initializer = active_default_fact = 0;
    try {
        for (auto edge = def.dependencies; edge;) {
            auto use = inline_dependencies[edge-1]; edge = use.next;
            switch (use.kind) {
            case DefaultDependencyKind::Member: demand_member(use.target); break;
            case DefaultDependencyKind::Specialization: demand_specialization(use.target); break;
            case DefaultDependencyKind::Storage: demand_template_storage(use.target); break;
            case DefaultDependencyKind::Argument: demand_default_fact(use.target); break;
            }
        }
        inline_variable_recipes[index].demand = FactState::Success;
    } catch (...) {
        unevaluated_depth = depth; current_function = function;
        active_inline_initializer = owner; active_default_fact = defaults;
        inline_variable_recipes[index].demand = FactState::Failure; throw;
    }
    unevaluated_depth = depth; current_function = function;
    active_inline_initializer = owner; active_default_fact = defaults;
}
} }
