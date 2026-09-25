#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::first_template_signature(EntityId e, NodeId source, ScopeId environment)
{
    using syntax::Kind;
    auto declarator = [&](NodeId n) {
        if (ast[n].kind == Kind::Function) return ast[ast[n].first].next;
        if (ast[n].kind == Kind::SimpleDeclaration) return ast[ast[child(n,Kind::InitDeclarators)].first].first;
        return child(n,Kind::Declarator);
    };
    auto id = template_first_signature_index.get(e);
    if (!id) {
        TemplateFirstSignature first; first.type = entities[e].type;
        first.environment = templates[entities[e].template_info].environment;
        first.declarator = declarator(entities[e].source);
        auto occurrence = ast.nodes.occurrences[first.declarator];
        if (occurrence.context) {
            // Member-template declarations retain raw source parameters until
            // body demand. Their existing frame maps those source parameters
            // into the concrete enclosing class and this declaration's head.
            first.frame = template_type_contexts.get(occurrence.context);
            first.declarator = template_signature_sources.get(occurrence.source);
            if (!first.frame || !first.declarator) throw std::logic_error("missing first member signature recipe");
        }
        id = template_first_signatures.size(); template_first_signatures.push_back(first);
        template_first_signature_index.put(e,id);
    }
    auto first = template_first_signatures[id];
    Index bindings, cache;
    // Enclosing parameters remain free; only this declaration's head is renamed.
    for (auto s = scopes[first.environment].parent; s; s = scopes[s].parent)
        if (scopes[s].kind == ScopeKind::Template)
            for (auto d = scopes[s].first_decl; d; d = declarations[d].next) {
                auto p = declarations[d].entity;
                if (entities[p].template_parameter) bindings.put(p,parameter_argument(p));
            }
    auto incoming = scopes[environment].first_decl;
    for (auto d = scopes[first.environment].first_decl; d; d = declarations[d].next) {
        auto p = declarations[d].entity;
        if (!entities[p].template_parameter) continue;
        while (incoming && !entities[declarations[incoming].entity].template_parameter) incoming = declarations[incoming].next;
        if (!incoming) throw std::logic_error("redeclaration lacks a template parameter");
        auto arg = parameter_argument(declarations[incoming].entity);
        bindings.put(p,arg);
        incoming = declarations[incoming].next;
    }
    auto type = substitute_type(first.type,bindings,cache);
    if (!type) throw std::logic_error("first template signature cannot be renamed");
    auto current = declarator(source);
    template_first_signatures[id].selected_parameters =
        apply_template_signature(first.declarator,current,type,bindings,first.frame) ? current : 0;
    return type;
}
bool Analyzer::apply_template_signature(NodeId original, NodeId current, TypeId type, const Index& bindings, std::uint32_t original_frame)
{
    using syntax::Kind;
    Index cache;
    auto parameters = [&](NodeId d) {
        NodeId result = 0;
        while (d) {
            if (auto p = child(d,Kind::Parameters)) result = p;
            d = ast[child(d,Kind::NestedDeclarator)].first;
        }
        return result;
    };
    // The common redeclaration has exactly the same substituted semantics.
    // Its already-published raw source facts need no additional traversal.
    if (type == types.signature(facts[current].type)) return false;
    auto occurrence = ast.nodes.occurrences[current];
    auto defining = occurrence.context ? template_signature_sources.get(occurrence.source) : current;
    auto frame = occurrence.context ? template_type_contexts.get(occurrence.context) : 0;
    if (!defining) throw std::logic_error("missing redeclaration parameter recipe");
    auto p = ast[parameters(original)].first, q = ast[parameters(current)].first;
    auto r = ast[parameters(defining)].first;
    for (; p && q && r; p = ast[p].next, q = ast[q].next, r = ast[r].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        auto raw = facts[p].type;
        if (original_frame) { Index empty; raw = substitute_type(raw,empty,cache,original_frame); }
        raw = substitute_type(raw,bindings,cache);
        if (!raw) throw std::logic_error("first raw parameter cannot be renamed");
        auto incoming = facts[r].type;
        if (frame) { Index empty; incoming = substitute_type(incoming,empty,cache,frame); }
        if (!incoming) throw std::logic_error("missing defining raw parameter type");
        // The first declaration owns dependent lookup, while the definition
        // owns top-level cv and the forms discarded by parameter adjustment.
        // Keep its outer array bound (not part of the callable signature).
        auto body = types[types.alias_target(incoming)];
        auto adjusted = types.adjusted(raw);
        if (body.kind == TypeKind::Array || body.kind == TypeKind::DependentArray || body.kind == TypeKind::Function) {
            auto pointer = types[types.alias_target(adjusted)];
            if (pointer.kind != TypeKind::Pointer) throw std::logic_error("inconsistent adjusted parameter");
            raw = body.kind == TypeKind::Function ? pointer.child : types.compound(body.kind,pointer.child,body.bound);
        } else raw = types.qualify(types.unqualified(adjusted),body.cv);
        facts.edit(q).type = raw;
    }
    if (p || q || r) throw std::logic_error("equivalent signatures have different source parameters");
    facts.edit(current).type = type;
    if (!ast.nodes.occurrences[current].context)
        template_type_sources.put(ast.nodes.occurrences[current].source,type+1);
    return true;
}
ScopeId Analyzer::member_template_environment(ScopeId head, ScopeId owner)
{
    if (scopes[head].parent == owner || scopes[owner].kind != ScopeKind::Class) return head;
    // A retained out-of-class definition already has its renamed enclosing
    // heads over this class. Keep that lexical overlay for initializers/bodies.
    if (member_definition_environment && encloses(member_definition_environment,head) &&
        scopes[member_definition_environment].parent == owner) return head;
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
std::uint32_t Analyzer::template_declaration_shape(TypeId type, ScopeId environment, TypeId* result)
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
    if (result) *result = normalized;
    shape.push_back(normalized);
    for (auto& argument : shape) argument = template_signature_shape(argument);
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
                // This map renames a source head; an ellipsis in its retained
                // type owns expansion. Bind the scalar symbol for both type
                // and value packs, not a sequence where a scalar is required.
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
EntityId Analyzer::declare_template_function(ScopeId owner, IdentifierId name, NodeId source, TypeId type, bool constructor, bool conversion)
{
    ScopeId environment = active_template_scope;
    ScopeId scope = owner == environment ? scopes[owner].parent : owner;
    environment = member_template_environment(environment,scope);
    TypeId normalized = 0;
    auto signature = template_declaration_shape(type,environment,&normalized);
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
        if (definition && previous.body && spec_has(ast[source].first,KW_FRIEND) &&
            ast.nodes.occurrences[source].context && !ast.nodes.occurrences[previous.source].context &&
            ast.nodes.occurrences[source].source == ast.nodes.occurrences[previous.source].source &&
            pattern_scope(previous.environment)) {
            // The source definition is a retained recipe. Its first concrete
            // enclosing class publishes the actual namespace definition.
            // A second class producing the same signature is a redefinition.
            previous.body = 0;
        }
        if (definition && previous.body) throw std::runtime_error("template function redefinition");
        if (!previous.body) {
            // Redeclarations may rename parameters. Keep each head immutable;
            // select the defining head as the owner of the retained body.
            entities[e].type = first_template_signature(e,source,environment);
            template_facts(e,environment);
            auto condition = members[entities[e].member_info].explicit_condition;
            if (condition) {
                auto head = templates[entities[e].template_info];
                Index bindings, cache;
                for (unsigned j = 0; j < head.count; ++j)
                    bindings.put(template_parameters[previous.offset+j],parameter_argument(template_parameters[head.offset+j]));
                auto rebound = substitute_query(condition,bindings,cache);
                if (!rebound) throw std::runtime_error("invalid redeclared explicit condition");
                members[entities[e].member_info].explicit_condition = rebound;
            }
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
    if (conversion) {
        if (!entities[e].member_info) {
            entities[e].member_info = members.size(); members.push_back(MemberFacts());
        }
        members[entities[e].member_info].conversion_hiding_target = types[normalized].child;
    }
    if (!constructor) {
        bind(owner,name,e);
        if (owner != scope) bind(scope,name,e);
    }
    return e;
}
} }
