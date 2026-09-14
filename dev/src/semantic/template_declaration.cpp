#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
ScopeId Analyzer::member_template_environment(ScopeId head, ScopeId owner)
{
    if (scopes[head].parent == owner || scopes[owner].kind != ScopeKind::Class) return head;
    auto k = key(head,owner);
    if (auto previous = member_template_environments.get(k)) return previous;
    // The return type preceding the qualified declarator uses the lexical
    // head. Its parameters and body see the class beneath the same parameters.
    auto environment = make_scope(ScopeKind::Template,owner);
    for (auto d = scopes[head].first_decl; d; d = declarations[d].next) {
        auto p = declarations[d].entity;
        if (entities[p].template_parameter) {
            bind(environment,entities[p].name,p);
            record(environment,p,0,entities[p].type,entities[p].kind);
        }
    }
    member_template_environments.put(k,environment); return environment;
}
EntityId Analyzer::declare_template_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor)
{
    ScopeId environment = active_template_scope;
    ScopeId scope = owner == environment ? scopes[owner].parent : owner;
    environment = member_template_environment(environment,scope);
    Index bindings, cache;
    std::vector<TypeId> shape;
    unsigned count = 0;
    for (auto d = scopes[environment].first_decl; d; d = declarations[d].next) {
        EntityId parameter = declarations[d].entity;
        if (!entities[parameter].template_parameter) continue;
        auto argument = canonical_argument(parameter,count,bindings,cache);
        bindings.put(parameter,argument);
        shape.push_back(entities[parameter].parameter_pack ? types.compound(TypeKind::PackExpansion,0,argument) : argument); ++count;
    }
    TypeId normalized = substitute_type(type,bindings,cache);
    if (!normalized) throw std::runtime_error("invalid function template declaration");
    shape.push_back(normalized);
    auto signature = intern_arguments(shape);
    auto family = template_families.get(key(scope,name));
    auto e = family ? template_signatures.get(key(family,signature)) : 0;
    if (e) {
        auto& previous = templates[entities[e].template_info];
        bool definition = ast[source].kind == syntax::Kind::Function || ast[source].kind == syntax::Kind::SpecialDefinition;
        if (definition && previous.body) throw std::runtime_error("template function redefinition");
        if (!previous.body) {
            // Redeclarations may rename parameters. Keep each head immutable;
            // select the defining head as the owner of the retained body.
            entities[e].type = type;
            template_facts(e,environment);
        }
    } else {
        e = make_entity(EntityKind::Function,scope,name,source);
        entities[e].type = type;
        template_facts(e,environment);
        if (!family) { family = e; template_families.put(key(scope,name),family); }
        template_signatures.put(key(family,signature),e);
    }
    if (!constructor) {
        bind(owner,name,e);
        if (owner != scope) bind(scope,name,e);
    }
    return e;
}
} }
