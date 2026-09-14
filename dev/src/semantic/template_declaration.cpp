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
std::uint32_t Analyzer::template_declaration_shape(TypeId type, ScopeId environment)
{
    Index bindings, cache;
    std::vector<TypeId> shape;
    // A nested declaration's owner already fixes enclosing parameter identity.
    // Normalize only this head; outer parameters are free variables of its
    // source signature, not failed substitutions.
    for (auto s = scopes[environment].parent; s; s = scopes[s].parent) {
        if (scopes[s].kind != ScopeKind::Template) continue;
        for (auto d = scopes[s].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (entities[p].template_parameter) {
                auto arg = parameter_argument(p);
                bindings.put(p,entities[p].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
            }
        }
    }
    unsigned count = 0;
    for (auto d = scopes[environment].first_decl; d; d = declarations[d].next) {
        EntityId parameter = declarations[d].entity;
        if (!entities[parameter].template_parameter) continue;
        auto argument = canonical_argument(parameter,count,bindings,cache);
        bindings.put(parameter,argument);
        shape.push_back(entities[parameter].parameter_pack ? types.compound(TypeKind::PackExpansion,0,argument) : argument); ++count;
    }
    TypeId normalized = substitute_type(type,bindings,cache);
    if (!normalized) throw std::runtime_error("invalid template declaration shape");
    shape.push_back(normalized);
    return intern_arguments(shape);
}
bool Analyzer::equivalent_alias_template(EntityId e, TypeId type, ScopeId environment)
{
    if (entities[e].kind != EntityKind::Alias || !entities[e].template_info) return false;
    auto head = entities[e].template_info;
    auto prior = alias_declaration_shapes.get(head);
    if (!prior) {
        prior = template_declaration_shape(entities[e].type,templates[head].environment);
        alias_declaration_shapes.put(head,prior);
    }
    return prior == template_declaration_shape(type,environment);
}
void Analyzer::merge_template_defaults(EntityId e, ScopeId incoming, ScopeId previous)
{
    auto has_defaults = [&](ScopeId scope) {
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (entities[p].template_parameter && (entities[p].initializer || template_default_types.get(p))) return true;
        }
        return false;
    };
    if (!has_defaults(incoming) && !has_defaults(previous)) return;
    auto selected = templates[entities[e].template_info];
    auto parameters = [&](ScopeId s) {
        std::vector<EntityId> result;
        for (auto d = scopes[s].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (entities[p].template_parameter) result.push_back(p);
        }
        return result;
    };
    auto current = parameters(incoming), old = parameters(previous);
    if (current.size() != selected.count || (previous && old.size() != selected.count))
        throw std::logic_error("template default head mismatch");
    Index old_bindings, new_bindings, old_cache, new_cache;
    for (auto s = scopes[incoming].parent; s; s = scopes[s].parent) {
        if (scopes[s].kind != ScopeKind::Template) continue;
        for (auto d = scopes[s].first_decl; d; d = declarations[d].next) {
            auto p = declarations[d].entity;
            if (entities[p].template_parameter) {
                auto arg = parameter_argument(p);
                if (entities[p].parameter_pack) arg = make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)});
                old_bindings.put(p,arg); new_bindings.put(p,arg);
            }
        }
    }
    for (unsigned j = 0; j < selected.count; ++j) {
        auto p = template_parameters[selected.offset+j];
        new_bindings.put(current[j],parameter_argument(p));
        if (previous) old_bindings.put(old[j],parameter_argument(p));
    }
    auto value = [&](EntityId p, ScopeId scope) {
        auto known = template_default_types.get(p);
        if (!known && entities[p].initializer) {
            known = template_argument_node(ast[entities[p].initializer].first,scope);
            template_default_types.put(p,known);
        }
        return known;
    };
    for (unsigned j = 0; j < selected.count; ++j) {
        auto before = previous ? value(old[j],previous) : 0;
        auto after = value(current[j],incoming);
        if (before && after && old[j] != current[j]) throw std::runtime_error("duplicate template default argument");
        auto result = after ? substitute_argument(after,new_bindings,new_cache) :
            before ? substitute_argument(before,old_bindings,old_cache) : 0;
        if ((before || after) && !result) throw std::runtime_error("invalid redeclared template default");
        if (result) template_default_types.put(template_parameters[selected.offset+j],result);
    }
}
EntityId Analyzer::declare_template_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor)
{
    ScopeId environment = active_template_scope;
    ScopeId scope = owner == environment ? scopes[owner].parent : owner;
    environment = member_template_environment(environment,scope);
    auto signature = template_declaration_shape(type,environment);
    auto family = template_families.get(key(scope,name));
    auto e = family ? template_signatures.get(key(family,signature)) : 0;
    if (e) {
        auto previous = templates[entities[e].template_info];
        bool definition = ast[source].kind == syntax::Kind::Function || ast[source].kind == syntax::Kind::SpecialDefinition;
        if (source == explicit_specialization_source && scopes[scope].kind == ScopeKind::Class &&
            entities[scopes[scope].entity].specialization && !entities[scopes[scope].entity].explicit_specialization &&
            !entities[e].explicit_specialization) {
            select_explicit_specialization(e,source);
            previous.body = 0;
        }
        if (definition && previous.body) throw std::runtime_error("template function redefinition");
        if (!previous.body) {
            // Redeclarations may rename parameters. Keep each head immutable;
            // select the defining head as the owner of the retained body.
            entities[e].type = type;
            template_facts(e,environment);
        }
        merge_template_defaults(e,environment,previous.environment);
    } else {
        e = make_entity(EntityKind::Function,scope,name,source);
        entities[e].type = type;
        template_facts(e,environment);
        merge_template_defaults(e,environment);
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
