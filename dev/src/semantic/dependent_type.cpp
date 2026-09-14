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
TypeId Analyzer::qualified_type(TypeId owner, IdentifierId name, const std::vector<TypeId>& args, bool template_id)
{
    if (dependent_type(owner)) return types.dependent_name(owner,name,args,template_id);
    EntityId member = qualified_type_member(owner,name);
    if (!member || (entities[member].kind != EntityKind::Type && entities[member].kind != EntityKind::Alias))
        throw std::runtime_error("qualified type member not found");
    if (template_id) {
        if (!entities[member].template_info) throw std::runtime_error("member is not a class template");
        return apply_type_template(member,args);
    }
    return source_type(member);
}
TypeId Analyzer::injected_template_type(EntityId e, ScopeId use)
{
    if (!e || entities[e].kind != EntityKind::Type) return 0;
    if (entities[e].class_info && entities[e].template_info) {
        if (!encloses(entities[e].scope,use)) return 0;
        auto head = templates[entities[e].template_info];
        if (head.primary) {
            auto pattern = argument_packs[head.explicit_arguments];
            std::vector<ArgumentId> args(argument_types.begin()+pattern.offset,argument_types.begin()+pattern.offset+pattern.count);
            return entities[specialize_class(head.primary,args)].type;
        }
        auto parameters = head.offset;
        // An out-of-class source overlay owns a parameter slice, independent
        // of the spelling and identity of the primary's original head.
        for (auto scope = use; scope; scope = scopes[scope].parent)
            if (auto source = definition_source_parameters.get(scope)) {
                parameters = source-1; break;
            }
        std::vector<TypeId> args;
        for (unsigned i = 0; i < head.count; ++i) {
            auto parameter = template_parameters[parameters+i]; auto arg = parameter_argument(parameter);
            args.push_back(entities[parameter].parameter_pack ? make_argument_pack({types.compound(TypeKind::PackExpansion,0,arg)}) : arg);
        }
        return entities[specialize_class(e,args)].type;
    }
    auto scope = entities[e].scope, parent = entities[e].owner;
    if (!entities[e].template_pattern || !entities[e].name || !scope ||
        scopes[scope].kind != ScopeKind::Class || scopes[parent].kind != ScopeKind::Class) return 0;
    auto owner = injected_template_type(scopes[parent].entity,use);
    return owner ? types.dependent_name(owner,entities[e].name,{},false) : 0;
}
TypeId Analyzer::type_name(NodeId n, ScopeId s, NodeId last)
{
    if (!last) last = ast[n].last;
    ScopeId owner = ast[n].op == OP_COLON2 ? global : s;
    bool qualified = ast[n].op == OP_COLON2;
    TypeId prefix = 0;
    for (auto p = ast[n].first; p; p = ast[p].next) {
        auto list = child(p,Kind::TemplateArguments);
        if (prefix && dependent_type(prefix)) {
            retain_type_access(p,prefix,s);
            std::vector<TypeId> args;
            for (auto a = ast[list].first; a; a = ast[a].next) {
                auto type = template_argument_node(a,s);
                if (template_type_probe && !type) return 0;
                args.push_back(value_argument(type) ? type : types.signature(type));
            }
            prefix = types.dependent_name(prefix,ast[p].text,args,list);
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
        // Nonterminal injected names still use the bound source class scope
        // so fixed qualified aliases keep their definition-time type facts.
        if (template_type_probe && entities[e].class_info && entities[e].template_info)
            prefix = p == last ? injected_template_type(e,s) : 0;
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
