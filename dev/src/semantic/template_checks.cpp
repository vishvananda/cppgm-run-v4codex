#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_template_parameters(NodeId n, ScopeId s)
{
    Index parameters;
    for (auto scope = s; scope; scope = scopes[scope].parent) {
        if (scopes[scope].kind != ScopeKind::Template) continue;
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto e = declarations[d].entity;
            if (entities[e].template_parameter && entities[e].name) parameters.put(entities[e].name,1);
        }
    }
    struct Work { NodeId node; bool callee; };
    std::vector<Work> work(1,Work{n,false}); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto item = work[i]; auto node = ast[item.node];
        if (!item.node || seen.get(item.node)) continue;
        seen.put(item.node,1);
        IdentifierId declared = 0;
        NodeId name = 0;
        if (node.kind == Kind::Declarator) name = decl_name(item.node);
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward || node.kind == Kind::Enum) name = node.detail;
        if (name && ast[name].first == ast[name].last && ast[name].op != OP_COLON2) declared = terminal(name);
        if (node.kind == Kind::Alias || node.kind == Kind::Enumerator) declared = node.text;
        if (node.kind == Kind::UsingDeclaration) declared = terminal(ast[node.first].detail);
        if (declared && parameters.get(declared)) throw std::runtime_error("declaration redeclares template parameter");
        if (node.kind == Kind::IdExpression && !item.callee && ast[node.detail].first == ast[node.detail].last &&
            parameters.get(terminal(node.detail))) throw std::runtime_error("type template parameter used as a value");
        if (node.detail) work.push_back({node.detail,false});
        for (auto child = node.first; child; child = ast[child].next)
            work.push_back({child,node.kind == Kind::Call && child == node.first});
    }
}
void Analyzer::index_template_members(NodeId n, std::uint32_t path, ScopeId s)
{
    auto add = [&](NodeId d, bool defined) {
        if (!d) return;
        auto name = decl_name(d), member = terminal(name);
        if (ast[ast[name].last].op == OP_COMPL) {
            auto text = ids.spelling(ast[ast[ast[name].last].first].text);
            std::string spelling = "~" + std::string(text.data,text.size);
            member = ids.intern(TextView(spelling.data(),spelling.size()));
        }
        auto k = key(path,member);
        TemplatePrototype prototype{d,s,template_prototype_index.get(k),defined};
        template_prototype_sources.put(ast.nodes.occurrences[d].source,template_prototypes.size());
        template_prototype_index.put(k,template_prototypes.size()); template_prototypes.push_back(prototype);
    };
    for (auto c = ast[n].first; c; c = ast[c].next) {
        if (spec_has(ast[c].first,KW_FRIEND)) continue;
        if (ast[c].kind == Kind::Class) {
            index_template_members(c,definition_path(path,terminal(ast[c].detail)),s); continue;
        }
        if (ast[c].kind == Kind::SimpleDeclaration) {
            auto first = ast[child(c,Kind::InitDeclarators)].first;
            for (auto spec = ast[ast[c].first].first; spec; spec = ast[spec].next)
                if (ast[spec].kind == Kind::Class) {
                    auto name = ast[spec].detail ? terminal(ast[spec].detail) : terminal(decl_name(ast[first].first));
                    if (name) index_template_members(spec,definition_path(path,name),s);
                }
            for (auto item = first; item; item = ast[item].next) {
                auto d = ast[item].first;
                add(d,child(ast[d].next,Kind::SpecialInitializer));
            }
        } else add(ast[c].kind == Kind::Function ? ast[ast[c].first].next : child(c,Kind::Declarator),
            ast[c].kind == Kind::Function || ast[c].kind == Kind::SpecialDefinition || child(child(c,Kind::Initializer),Kind::SpecialInitializer));
    }
}
int Analyzer::template_exception(NodeId d, ScopeId s)
{
    for (auto c = ast[d].first; c; c = ast[c].next) {
        if (ast[c].kind != Kind::FunctionQualifier || ast[c].op != KW_NOEXCEPT) continue;
        if (!ast[c].first) return 1;
        // Names in this fact may need class declaration facts or substitution.
        // Keep that dependency instead of evaluating in the wrong environment.
        std::vector<NodeId> work(1,ast[c].first);
        for (std::size_t j = 0; j < work.size(); ++j) {
            auto n = ast[work[j]];
            if (n.kind == Kind::IdExpression || n.kind == Kind::TypeId) return -1;
            for (auto child = n.first; child; child = ast[child].next) work.push_back(child);
        }
        auto value = evaluate(ast[c].first,s);
        if (!value.valid) throw std::runtime_error("nonconstant template exception specification");
        return value.bits != 0;
    }
    return ast[ast[decl_name(d)].last].op == OP_COMPL ? -1 : 0;
}
ScopeId Analyzer::template_signature_owner(TypeId type, EntityId primary)
{
    auto t = types[type];
    if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto spec = specializations[entities[t.entity].specialization];
        if (spec.pattern != primary) return 0;
        auto args = argument_packs[spec.arguments];
        if (args.count != templates[entities[primary].template_info].count || args.count > canonical_parameters.size()) return 0;
        for (unsigned i = 0; i < args.count; ++i)
            if (argument_types[args.offset+i] != canonical_parameters[i]) return 0;
        return entities[primary].scope;
    }
    if (t.kind == TypeKind::DependentName && !t.bound) {
        auto owner = template_signature_owner(t.child,primary);
        auto member = owner ? local(owner,t.entity,Lookup::Qualifier) : 0;
        if (member && entities[member].kind == EntityKind::Type) return entities[member].scope;
    }
    return 0;
}
TypeId Analyzer::template_member_aliases(TypeId type, EntityId primary, Index& cache)
{
    if (!type || !dependent_type(type)) return type;
    if (auto known = cache.get(type)) return known;
    cache.put(type,type);
    auto t = types[type]; TypeId result = type;
    auto child = t.child ? template_member_aliases(t.child,primary,cache) : 0;
    if (t.child && !child) return 0;
    if (t.kind == TypeKind::DependentName) {
        auto owner = template_signature_owner(child,primary);
        auto alias = owner && !t.bound ? local(owner,t.entity,Lookup::Qualifier) : 0;
        if (alias && entities[alias].kind == EntityKind::Alias && entities[alias].type) {
            // Only aliases of this current instantiation are expanded. A
            // different dependent specialization retains its symbolic member.
            Index bindings, substitution;
            template_signature_bindings(entities[alias].owner,primary,bindings);
            auto value = substitute_type(entities[alias].type,bindings,substitution);
            if (!value) return 0;
            value = template_member_aliases(value,primary,cache);
            if (!value) return 0;
            result = types.qualify(value,t.cv);
        } else if (alias && alias == scopes[owner].entity) {
            // A qualified injected-class-name denotes its current owner.
            result = types.qualify(child,t.cv);
        } else {
            std::vector<TypeId> args;
            for (unsigned i = 0; i < t.count; ++i) {
                auto value = template_member_aliases(types.parameters[t.offset+i],primary,cache);
                if (!value) return 0;
                args.push_back(value);
            }
            result = types.qualify(types.dependent_name(child,t.entity,args,t.bound),t.cv);
        }
    } else if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto spec = specializations[entities[t.entity].specialization]; auto pack = argument_packs[spec.arguments];
        std::vector<TypeId> args; bool changed = false;
        for (unsigned i = 0; i < pack.count; ++i) {
            auto source = argument_types[pack.offset+i];
            auto value = template_member_aliases(source,primary,cache);
            if (!value) return 0;
            args.push_back(value); changed |= value != source;
        }
        if (changed) result = types.qualify(entities[specialize_class(spec.pattern,args)].type,t.cv);
    } else if (t.kind == TypeKind::Function) {
        std::vector<TypeId> params;
        for (unsigned i = 0; i < t.count; ++i) {
            auto parameter = template_member_aliases(types.parameters[t.offset+i],primary,cache);
            if (!parameter) return 0;
            params.push_back(parameter);
        }
        result = types.signature(types.function(child,params,t.variadic,t.cv,t.ref));
    } else if (t.child) {
        result = t.kind == TypeKind::MemberPointer ? types.member_pointer(t.entity,child) : types.compound(t.kind,child,t.bound);
        result = types.qualify(result,t.cv);
    }
    cache.put(type,result); return result;
}
void Analyzer::template_signature_bindings(ScopeId scope, EntityId primary, Index& bindings)
{
    auto head = templates[entities[primary].template_info];
    for (; scope; scope = scopes[scope].parent) {
        if (auto parameters = definition_source_parameters.get(scope)) {
            for (unsigned i = 0; i < head.count; ++i)
                bindings.put(template_parameters[parameters-1+i],canonical_parameters[i]);
        } else if (scopes[scope].kind == ScopeKind::Template) {
            for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
                auto parameter = declarations[d].entity;
                if (!entities[parameter].template_parameter) continue;
                auto ordinal = parameter_ordinals.get(parameter);
                if (ordinal && ordinal <= head.count)
                    bindings.put(parameter,canonical_parameters[ordinal-1]);
            }
        }
    }
}
TypeId Analyzer::template_member_signature(TypeId type, ScopeId head, EntityId primary, ScopeId owner)
{
    if (!type || !dependent_type(type)) return type;
    auto count = templates[entities[primary].template_info].count;
    while (canonical_parameters.size() < count) {
        auto e = make_entity(EntityKind::Type,0,0,0);
        entities[e].template_parameter = true; entities[e].type = types.named(e);
        canonical_parameters.push_back(entities[e].type);
    }
    Index bindings, cache;
    template_signature_bindings(head,primary,bindings);
    template_signature_bindings(owner,primary,bindings);
    auto result = substitute_type(type,bindings,cache);
    Index aliases; return template_member_aliases(result,primary,aliases);
}
std::uint32_t Analyzer::check_template_member_definition(NodeId d, std::uint32_t path, IdentifierId name, ScopeId head, EntityId primary)
{
    auto declared = facts[d].type;
    if (!declared || types[declared].kind != TypeKind::Function) return 0;
    auto signature = template_member_signature(declared,head,primary,facts[d].scope);
    if (!signature) return 0;
    ++definition_signature_requests;
    auto group = template_prototype_index.get(key(path,name));
    bool unresolved = false;
    if (!template_signature_groups.get(group)) {
        for (auto p = group; p; p = template_prototypes[p].next) {
            ++definition_signature_work;
            auto prototype = template_prototypes[p];
            auto type = prototype.signature;
            if (!type) {
                type = template_member_signature(facts[prototype.declarator].type,prototype.environment,primary,facts[prototype.declarator].scope);
                template_prototypes[p].signature = type;
            }
            if (!type) { unresolved = true; continue; }
            template_signature_index.put(key(group,type),p);
        }
        // A source bucket is immutable once all prototype types are known.
        // An unresolved type cannot publish a negative signature result.
        if (!unresolved) template_signature_groups.put(group,1);
    }
    if (auto p = template_signature_index.get(key(group,signature))) {
        auto prototype = template_prototypes[p];
        if (prototype.inline_definition) throw std::runtime_error("template member was already defined in its class");
        auto current = template_exception(d,head);
        auto previous = template_exception(prototype.declarator,prototype.environment);
        if (current >= 0 && previous >= 0 && current != previous)
            throw std::runtime_error("conflicting template member exception specifications");
        return p;
    }
    // Special-member and not-yet-established declaration types retain their
    // own checking owner. A complete ordinary signature must name a member.
    if (!unresolved) throw std::runtime_error("template definition does not match a declared member");
    return 0;
}
} }
