#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_template_parameters(NodeId n, ScopeId s)
{
    // This checks source spellings against lexical template parameters.
    // Substitution cannot change those spellings. Nested templates own their
    // own check after their complete head is declared, including retained
    // out-of-class heads; projected declarations reuse that source check.
    if (ast.nodes.occurrences[n].context) { ++template_parameter_check_reuses; return; }
    if (ast[n].kind == Kind::Template) return;
    Index parameters;
    for (auto scope = s; scope; scope = scopes[scope].parent) {
        if (scopes[scope].kind != ScopeKind::Template) continue;
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto e = declarations[d].entity;
            if (entities[e].template_parameter && entities[e].name) parameters.put(entities[e].name,entities[e].kind == EntityKind::Type ? 1 : 2);
        }
    }
    struct Work { NodeId node; bool callee; };
    std::vector<Work> work(1,Work{n,false}); Index seen;
    for (std::size_t i = 0; i < work.size(); ++i) {
        auto item = work[i]; auto node = ast[item.node];
        if (!item.node || node.kind == Kind::Template || seen.get(item.node)) continue;
        seen.put(item.node,1);
        ++template_parameter_check_work;
        IdentifierId declared = 0;
        NodeId name = 0;
        if (node.kind == Kind::Declarator) name = decl_name(item.node);
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward || node.kind == Kind::Enum) name = node.detail;
        if (name && ast[name].first == ast[name].last && ast[name].op != OP_COLON2) declared = terminal(name);
        if (node.kind == Kind::Alias || node.kind == Kind::Enumerator) declared = node.text;
        if (node.kind == Kind::UsingDeclaration) declared = terminal(ast[node.first].detail);
        if (declared && parameters.get(declared)) throw std::runtime_error("declaration redeclares template parameter");
        if (node.kind == Kind::IdExpression && !item.callee && ast[node.detail].first == ast[node.detail].last &&
            parameters.get(terminal(node.detail)) == 1) throw std::runtime_error("type template parameter used as a value");
        if (node.detail) work.push_back({node.detail,false});
        for (auto child = node.first; child; child = ast[child].next)
            work.push_back({child,node.kind == Kind::Call && child == node.first});
    }
}
void Analyzer::index_template_members(NodeId n, std::uint32_t path, ScopeId s)
{
    auto add = [&](NodeId d, bool defined) {
        if (!d) return;
        if (template_prototype_sources.get(ast.nodes.occurrences[d].source)) return;
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
    for (auto entry = ast[n].first; entry; entry = ast[entry].next) {
        auto c = entry;
        while (ast[c].kind == Kind::Template) c = ast[ast[c].first].next;
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
        return constant_truth(value);
    }
    return ast[ast[decl_name(d)].last].op == OP_COMPL ? -1 : 0;
}
ScopeId Analyzer::template_signature_owner(TypeId type, EntityId primary)
{
    auto t = types[type];
    if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto spec = specializations[entities[t.entity].specialization];
        auto selected = primary;
        auto primary_head = templates[entities[primary].template_info];
        bool nested = spec.pattern != (primary_head.primary ? primary_head.primary : primary);
        if (nested) {
            if (!encloses(entities[primary].scope,entities[spec.pattern].owner)) return 0;
            selected = spec.pattern;
        }
        auto head = templates[entities[selected].template_info];
        auto args = argument_packs[spec.arguments];
        Index bindings, cache;
        template_signature_bindings(head.environment,primary,bindings);
        if (head.primary) {
            auto pattern = argument_packs[head.explicit_arguments];
            if (args.count != pattern.count) return 0;
            for (unsigned i = 0; i < args.count; ++i)
                if (argument_types[args.offset+i] != substitute_argument(argument_types[pattern.offset+i],bindings,cache)) return 0;
        } else {
            if (args.count != head.count) return 0;
            for (unsigned i = 0; i < args.count; ++i) {
                auto p = template_parameters[head.offset+i];
                auto arg = bindings.get(p);
                if (entities[p].parameter_pack) arg = make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)});
                if (argument_types[args.offset+i] != arg) return 0;
            }
        }
        return entities[selected].scope;
    }
    if (t.kind == TypeKind::DependentName) {
        auto owner = template_signature_owner(t.child,primary);
        auto member = owner ? local(owner,t.entity,Lookup::Qualifier) : 0;
        if (DependentNameKind(t.bound) == DependentNameKind::Application) {
            if (!member || !entities[member].class_info || !entities[member].template_info) return 0;
            std::vector<ArgumentId> arguments(types.parameters.begin()+t.offset,types.parameters.begin()+t.offset+t.count);
            return template_signature_owner(entities[specialize_class(member,arguments)].type,primary);
        }
        if (DependentNameKind(t.bound) == DependentNameKind::Type && member && entities[member].kind == EntityKind::Type) return entities[member].scope;
    }
    return 0;
}
QueryId Analyzer::template_signature_query(QueryId id, EntityId primary, Index& cache)
{
    auto key = (std::uint64_t(1) << 63) | id;
    if (auto known = cache.get(key)) return known;
    cache.put(key,id);
    auto query = type_queries[id];
    // Signature equivalence compares already-bound operands and canonical
    // head ordinals. Access environments belong to the original source query,
    // not this declaration-matching key; these normalized queries are never
    // substituted to establish a concrete declaration's type.
    query.context = 0;
    query.naming = 0;
    if (query.type) query.type = template_member_aliases(query.type,primary,cache);
    std::vector<QueryId> children;
    for (unsigned i = 0; i < query.count; ++i)
        children.push_back(template_signature_query(query_edges[query.offset+i],primary,cache));
    auto result = intern_query(query,children); cache.put(key,result); return result;
}
TypeId Analyzer::template_member_aliases(TypeId type, EntityId primary, Index& cache)
{
    if (!type || !dependent_type(type)) return type;
    if (auto known = cache.get(type)) return known;
    cache.put(type,type);
    auto t = types[type]; TypeId result = type;
    auto child = t.child ? template_member_aliases(t.child,primary,cache) : 0;
    if (t.child && !child) return 0;
    if (t.kind == TypeKind::Decltype) {
        result = types.qualify(types.decltype_type(template_signature_query(t.entity,primary,cache),t.bound),t.cv);
    } else if (t.kind == TypeKind::DependentName) {
        auto owner = template_signature_owner(child,primary);
        auto alias = owner && !t.bound ? local(owner,t.entity,Lookup::Qualifier) : 0;
        if (alias && entities[alias].type && (entities[alias].kind == EntityKind::Alias ||
            (entities[alias].kind == EntityKind::Type && (entities[alias].key == KW_ENUM || entities[alias].template_pattern)))) {
            // Canonical aliases and enum declarations of this current
            // instantiation have source type identity. A different dependent
            // specialization retains its symbolic member qualification.
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
                auto arg = types.parameters[t.offset+i];
                auto value = value_argument(arg) ? arg : template_member_aliases(arg,primary,cache);
                if (!value) return 0;
                args.push_back(value);
            }
            result = types.qualify(types.dependent_name(child,t.entity,args,DependentNameKind(t.bound)),t.cv);
        }
    } else if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto spec = specializations[entities[t.entity].specialization]; auto pack = argument_packs[spec.arguments];
        std::vector<TypeId> args; bool changed = false;
        for (unsigned i = 0; i < pack.count; ++i) {
            auto source = argument_types[pack.offset+i];
            auto value = value_argument(source) ? source : template_member_aliases(source,primary,cache);
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
    Index cache;
    std::vector<ScopeId> heads;
    for (; scope; scope = scopes[scope].parent)
        if (scopes[scope].kind == ScopeKind::Template) heads.push_back(scope);
    unsigned depth = 0;
    for (auto at = heads.rbegin(); at != heads.rend(); ++at) {
        scope = *at;
        if (auto id = definition_source_heads.get(scope)) {
            auto source = template_definition_heads[id]; depth = source.depth;
            for (unsigned i = 0; i < source.count; ++i) {
                auto p = template_parameters[source.parameters+i];
                bindings.put(p,canonical_argument(p,i,bindings,cache,depth));
            }
            ++depth;
        } else if (scopes[scope].kind == ScopeKind::Template) {
            unsigned ordinal = 0;
            for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
                auto parameter = declarations[d].entity;
                if (!entities[parameter].template_parameter) continue;
                bindings.put(parameter,canonical_argument(parameter,ordinal++,bindings,cache,depth));
            }
            if (ordinal) ++depth;
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
    if (!declared) return 0;
    auto signature_of = [&](NodeId node, ScopeId environment) {
        auto type = template_member_signature(facts[node].type,environment,primary,facts[node].scope);
        if (!type) return std::uint32_t(0);
        auto e = facts[node].entity;
        std::uint32_t shape = 0;
        if (entities[e].template_info) {
            Index bindings, cache;
            template_signature_bindings(environment,primary,bindings);
            template_signature_bindings(facts[node].scope,primary,bindings);
            auto head = templates[entities[e].template_info];
            std::vector<ArgumentId> arguments;
            for (unsigned j = 0; j < head.count; ++j) {
                auto p = template_parameters[head.offset+j], arg = bindings.get(p);
                if (!arg) throw std::logic_error("member signature lacks template head identity");
                arguments.push_back(entities[p].parameter_pack ? types.compound(TypeKind::PackExpansion,0,arg) : arg);
            }
            shape = intern_arguments(arguments);
        }
        return intern_arguments({type,shape});
    };
    auto signature = signature_of(d,head);
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
                type = signature_of(prototype.declarator,prototype.environment);
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
        if (types[declared].kind != TypeKind::Function) {
            if (!entities[facts[prototype.declarator].entity].is_static)
                throw std::runtime_error("out-of-class data definition requires a static member");
            return p;
        }
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
