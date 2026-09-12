#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
EntityId Analyzer::declare_template_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type)
{
    ScopeId environment = active_template_scope;
    ScopeId scope = owner == environment ? scopes[owner].parent : owner;
    Index bindings, cache;
    std::vector<TypeId> shape;
    unsigned count = 0;
    for (auto d = scopes[environment].first_decl; d; d = declarations[d].next) {
        EntityId parameter = declarations[d].entity;
        if (!entities[parameter].template_parameter) continue;
        if (count == canonical_parameters.size()) {
            auto e = make_entity(EntityKind::Type,0,0,0);
            entities[e].template_parameter = true; entities[e].type = types.named(e);
            canonical_parameters.push_back(entities[e].type);
        }
        bindings.put(parameter,canonical_parameters[count]);
        shape.push_back(canonical_parameters[count++]);
    }
    TypeId normalized = substitute_type(type,bindings,cache);
    if (!normalized) throw std::runtime_error("invalid function template declaration");
    shape.push_back(normalized);
    auto signature = intern_arguments(shape);
    auto family = template_families.get(key(scope,name));
    auto e = family ? template_signatures.get(key(family,signature)) : 0;
    if (e) {
        auto& previous = templates[entities[e].template_info];
        bool definition = ast[source].kind == syntax::Kind::Function;
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
    bind(owner,name,e);
    if (owner != scope) bind(scope,name,e);
    return e;
}
} }
