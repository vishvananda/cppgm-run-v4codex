#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::qualified_type(TypeId owner, IdentifierId name, const std::vector<TypeId>& args, bool template_id)
{
    if (dependent_type(owner)) return types.dependent_name(owner,name,args,template_id);
    if (types[owner].kind != TypeKind::Named) throw std::runtime_error("type qualifier is not a class");
    EntityId cls = types[owner].entity;
    complete_class(cls);
    EntityId member = lookup(entities[cls].scope,name,Lookup::Ordinary,true);
    if (!member || (entities[member].kind != EntityKind::Type && entities[member].kind != EntityKind::Alias))
        throw std::runtime_error("qualified type member not found");
    if (template_id) {
        if (!entities[member].template_info) throw std::runtime_error("member is not a class template");
        member = specialize_class(member,args);
    }
    return source_type(member);
}
TypeId Analyzer::type_name(NodeId n, ScopeId s)
{
    ScopeId owner = ast[n].op == OP_COLON2 ? global : s;
    bool qualified = ast[n].op == OP_COLON2;
    TypeId prefix = 0;
    for (auto p = ast[n].first; p; p = ast[p].next) {
        auto list = child(p,Kind::TemplateArguments);
        if (prefix && dependent_type(prefix)) {
            std::vector<TypeId> args;
            for (auto a = ast[list].first; a; a = ast[a].next) {
                if (ast[a].kind != Kind::TypeId) throw std::runtime_error("type template argument required");
                auto type = type_id(a,s);
                if (template_type_probe && !type) return 0;
                args.push_back(types.signature(type));
            }
            prefix = types.dependent_name(prefix,ast[p].text,args,list);
            continue;
        }
        if (ast[ast[p].detail].kind == Kind::Decltype) {
            prefix = expression_type(ast[ast[p].detail].first,s,true);
            if (template_type_probe && !prefix) return 0;
            if (dependent_type(prefix)) continue;
            if (types[prefix].kind != TypeKind::Named) throw std::runtime_error("decltype qualifier is not a class");
            complete_class(types[prefix].entity);
            owner = entities[types[prefix].entity].scope; qualified = true; continue;
        }
        auto e = lookup(owner,ast[p].text,p == ast[n].last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
        auto instance = class_template_name(p,e,s);
        if (template_type_probe && e && !instance) return 0;
        e = instance;
        if (!e) throw std::runtime_error("type name is not visible");
        check_access(e,s,owner);
        bool type = entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias;
        prefix = type ? source_type(e) : 0;
        // A local pattern class/enum or the primary's injected name has no
        // concrete declaration type yet. Qualified aliases can still supply
        // a canonical symbolic type through their already bound class scope.
        if (template_type_probe && entities[e].class_info && entities[e].template_info) prefix = 0;
        if (p == ast[n].last) {
            if (!type) throw std::runtime_error("type name denotes a value");
            facts[n].entity = e;
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
