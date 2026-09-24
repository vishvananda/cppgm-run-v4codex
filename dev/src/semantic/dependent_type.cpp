#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::qualified_type_member(TypeId owner, IdentifierId name)
{
    if (types[owner].kind != TypeKind::Named) throw std::runtime_error("type qualifier is not a class");
    auto cls = types[owner].entity;
    auto k = key(cls,name);
    if (auto member = qualified_type_members.get(k)) return member;
    complete_class(cls);
    auto member = lookup(entities[cls].scope,name,Lookup::Ordinary,true);
    // In-progress classes can still introduce members that hide a base name.
    // Only completed classes publish immutable selected-member facts.
    if (member && entities[cls].complete) qualified_type_members.put(k,member);
    return member;
}
TypeId Analyzer::qualified_type(TypeId owner, IdentifierId name, const std::vector<TypeId>& args, DependentNameKind kind)
{
    if (dependent_type(owner)) return types.dependent_name(owner,name,args,kind);
    if (types[owner].kind != TypeKind::Named) {
        if (template_type_probe) return 0;
        throw std::runtime_error("type qualifier is not a class");
    }
    EntityId member = qualified_type_member(owner,name);
    if (!member || (entities[member].kind != EntityKind::Type && entities[member].kind != EntityKind::Alias)) {
        if (template_type_probe) return 0;
        throw std::runtime_error("qualified type member not found");
    }
    bool member_template = entities[member].template_info;
    if (member_template != (kind != DependentNameKind::Type)) {
        if (template_type_probe) return 0;
        throw std::runtime_error("qualified member has the wrong type/template category");
    }
    if (kind == DependentNameKind::Application) {
        return apply_type_template(member,args);
    }
    // A dependent template-template argument names the member template entity,
    // whereas a template-id applies its alias body to an argument tuple.
    if (kind == DependentNameKind::Template) return types.named(member);
    return source_type(member);
}
TypeId Analyzer::injected_template_type(EntityId e, ScopeId use)
{
    if (!e || entities[e].kind != EntityKind::Type) return 0;
    if (entities[e].class_info && entities[e].template_info) {
        if (!encloses(entities[e].scope,use)) return 0;
        auto head = templates[entities[e].template_info];
        auto parameters = head.offset;
        // An out-of-class source overlay owns a parameter slice, independent
        // of the spelling and identity of the primary's original head.
        for (auto scope = use; scope; scope = scopes[scope].parent)
            if (auto source = definition_source_heads.get(scope)) {
                auto defined = template_definition_heads[source];
                if (defined.pattern == e) { parameters = defined.parameters; break; }
            }
        // The entity fixes head width and partial shape; the immutable
        // parameter slice distinguishes every renamed source environment.
        auto identity = key(e,parameters);
        if (auto type = injected_type_facts.get(identity)) return type;
        std::vector<TypeId> args;
        if (head.primary) {
            auto pattern = argument_packs[head.explicit_arguments];
            Index bindings, cache;
            if (parameters != head.offset) for (unsigned j = 0; j < head.count; ++j) {
                auto p = template_parameters[parameters+j]; auto arg = parameter_argument(p);
                // This is a source-head renaming. The retained expansion
                // already owns its ellipsis; the binding is its scalar pattern.
                bindings.put(template_parameters[head.offset+j],arg);
            }
            for (unsigned j = 0; j < pattern.count; ++j) {
                auto arg = argument_types[pattern.offset+j];
                args.push_back(parameters == head.offset ? arg : substitute_argument(arg,bindings,cache));
            }
            auto type = entities[specialize_class(head.primary,args)].type;
            injected_type_facts.put(identity,type); return type;
        }
        for (unsigned i = 0; i < head.count; ++i) {
            auto parameter = template_parameters[parameters+i]; auto arg = parameter_argument(parameter);
            args.push_back(entities[parameter].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
        }
        auto type = entities[specialize_class(e,args)].type;
        injected_type_facts.put(identity,type); return type;
    }
    auto scope = entities[e].scope, parent = entities[e].owner;
    if (!entities[e].template_pattern || !entities[e].name || !scope ||
        scopes[scope].kind != ScopeKind::Class || scopes[parent].kind != ScopeKind::Class) return 0;
    auto owner = injected_template_type(scopes[parent].entity,use);
    return owner ? types.dependent_name(owner,entities[e].name,{},DependentNameKind::Type) : 0;
}
ScopeId Analyzer::current_instantiation_scope(TypeId type, ScopeId use)
{
    for (auto scope = use; scope; scope = scopes[scope].parent) {
        if (scopes[scope].kind != ScopeKind::Class) continue;
        auto entity = scopes[scope].entity;
        if (types.unqualified(type) == entities[entity].type ||
            types.unqualified(type) == injected_template_type(entity,use)) return scope;
    }
    return 0;
}
void Analyzer::resolve_parenthesized_declaration(NodeId declaration, ScopeId scope)
{
    if (ast.nodes.occurrences[declaration].context) return;
    auto kind = ast[declaration].kind;
    if ((kind != Kind::SimpleDeclaration && kind != Kind::Function) || !(ast[declaration].flags & 1)) return;
    auto items = child(declaration,Kind::InitDeclarators);
    auto item = ast[items].first;
    do {
        auto d = kind == Kind::Function ? ast[ast[declaration].first].next : ast[item].first;
        auto params = child(d,Kind::Parameters), p = ast[params].first;
        auto specs = ast[p].first, spec = ast[specs].first, name = ast[spec].detail;
        if (p && !ast[p].next && ast[p].kind == Kind::Parameter && !ast[specs].next &&
            spec && ast[spec].kind == Kind::DeclSpecifier && !ast[spec].next && !(ast[spec].flags & 1) && name &&
            ast[name].first != ast[name].last) {
            auto previous = ast[name].first;
            while (ast[previous].next != ast[name].last) previous = ast[previous].next;
            auto binding = bind_template_name(name,scope,previous);
            if (binding.dependent) {
                auto type = type_name(name,scope,previous);
                auto current = current_instantiation_scope(type,scope);
                auto member = current ? lookup(current,terminal(name),Lookup::Ordinary,true) : 0;
                bool known_type = member && (entities[member].kind == EntityKind::Type || entities[member].kind == EntityKind::Alias);
                if (dependent_type(type) && !known_type) {
                    if (kind == Kind::Function || ast[d].next || ast[params].next || spec_has(ast[declaration].first,KW_TYPEDEF))
                        throw std::runtime_error("dependent name does not declare a parameter type");
                    // Resolve this single parsed ambiguity before publishing
                    // semantic facts. Reuse its name and delimiter wrappers;
                    // neither grammar nor a specialization is parsed again.
                    auto before = ast[d].first;
                    while (before && ast[before].next != params) before = ast[before].next;
                    if (!before) throw std::logic_error("parenthesized declaration has no declarator-id");
                    if (ast[before].kind != Kind::Identifier)
                        throw std::runtime_error("dependent name does not declare a pointed-to function parameter type");
                    ast.resolve_paren_initializer(item,d,params,before);
                }
            }
        }
        item = ast[item].next;
    } while (item);
}
TypeId Analyzer::type_name(NodeId n, ScopeId s, NodeId last, bool require_typename, bool template_name)
{
    if (!last) last = ast[n].last;
    ScopeId owner = ast[n].op == OP_COLON2 ? global : s;
    bool qualified = ast[n].op == OP_COLON2;
    TypeId prefix = 0;
    for (auto p = ast[n].first; p; p = ast[p].next) {
        auto list = child(p,Kind::TemplateArguments);
        if (prefix && dependent_type(prefix)) {
            auto current = current_instantiation_scope(prefix,s);
            // Known members of the current instantiation are looked up at the
            // definition. Other dependent qualifiers retain a substitution path.
            if (current && (lookup(current,ast[p].text,Lookup::Ordinary,true) ||
                !template_pattern_open_bases.get(scopes[current].entity))) {
                // A known nondependent base member is looked up again in the
                // instantiation context when a dependent base can add a
                // conflicting name ([temp.dep.type]/7).
                if (template_pattern_open_bases.get(scopes[current].entity)) retain_type_access(p,prefix,s);
                owner = current; qualified = true; prefix = 0;
            }
        }
        if (prefix && dependent_type(prefix)) {
            if (!ast.nodes.occurrences[n].context) {
                if (list && !(ast[p].flags & 1)) throw std::runtime_error("dependent member template requires template");
                if (p == last && require_typename) throw std::runtime_error("dependent qualified type requires typename");
            }
            retain_type_access(p,prefix,s);
            std::vector<TypeId> args;
            for (auto a = ast[list].first; a; a = ast[a].next) {
                auto type = template_argument_node(a,s);
                if (template_type_probe && !type) return 0;
                args.push_back(value_argument(type) ? type : types.signature(type));
            }
            auto kind = list ? DependentNameKind::Application :
                template_name && p == last ? DependentNameKind::Template : DependentNameKind::Type;
            prefix = types.dependent_name(prefix,ast[p].text,args,kind);
            if (p == last) return prefix;
            continue;
        }
        if (ast[ast[p].detail].kind == Kind::Decltype) {
            prefix = expression_type(ast[ast[p].detail].first,s,true);
            if (template_type_probe && !prefix) return 0;
            if (p == last) return prefix;
            if (dependent_type(prefix)) continue;
            if (types[prefix].kind != TypeKind::Named) throw std::runtime_error("decltype qualifier is not a class");
            complete_class(types[prefix].entity);
            owner = entities[types[prefix].entity].scope; qualified = true; continue;
        }
        auto e = lookup(owner,ast[p].text,p == last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
        auto template_target = template_entity(e);
        if (list && template_target && entities[template_target].kind == EntityKind::Alias) {
            check_access(template_target,s,owner);
            std::vector<ArgumentId> args;
            for (auto a = ast[list].first; a; a = ast[a].next)
                append_template_argument(a,s,template_argument_node(a,s),args);
            prefix = specialize_alias(template_target,args);
            if (p == last) return prefix;
            if (dependent_type(prefix)) continue;
            if (types[prefix].kind != TypeKind::Named) throw std::runtime_error("alias qualifier is not a class");
            complete_class(types[prefix].entity); owner = entities[types[prefix].entity].scope; qualified = true; continue;
        }
        auto instance = class_template_name(p,e,s);
        if (template_type_probe && e && !instance) return 0;
        e = instance;
        if (!e) throw std::runtime_error("type name is not visible");
        check_access(e,s,owner);
        bool type = entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias;
        prefix = type ? source_type(e) : 0;
        // Member class identities have symbolic current-instantiation types.
        // Retain the current-instantiation type for a dependent base member.
        // The next component uses the source scope only if lookup establishes
        // that member now; otherwise it remains a dependent qualified path.
        if (entities[e].class_info && entities[e].template_info && encloses(entities[e].scope,s))
            prefix = injected_template_type(e,s);
        else if (template_type_probe && !prefix && p == last) prefix = injected_template_type(e,s);
        if (p == last) {
            if (!type) throw std::runtime_error("type name denotes a value");
            if (last == ast[n].last) facts.edit(n).entity = e;
            return prefix;
        }
        if (prefix && dependent_type(prefix)) continue;
        if (prefix && types[prefix].kind == TypeKind::Named) complete_class(types[prefix].entity);
        owner = target(e); qualified = true;
        if (template_type_probe && !owner && type && !prefix) return 0;
        if (!owner) throw std::runtime_error("type qualifier has no scope");
    }
    return prefix;
}
} }
