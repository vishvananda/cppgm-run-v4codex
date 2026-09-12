#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
ScopeId Analyzer::bind_template_class(NodeId n, ScopeId parent, EntityId entity, std::vector<Body>* deferred)
{
    auto source = ast.nodes.occurrences[n].source;
    if (auto scope = template_class_bindings.get(source)) return scope;
    auto name = ast[n].detail;
    if (!entity) entity = pattern_declaration(EntityKind::Type,parent,terminal(name),n,true);
    template_pattern_entities.put(entity,2);
    auto cs = entities[entity].scope;
    if (!cs) {
        cs = make_scope(ScopeKind::Class,parent,entities[entity].name,entity,false);
        entities[entity].scope = cs;
    }
    if (scopes[cs].parent != parent) {
        scopes[cs].parent = parent; scopes[cs].depth = scopes[parent].depth+1;
        auto jump = scopes[parent].jump, grand = scopes[jump].jump;
        scopes[cs].jump = scopes[parent].depth-scopes[jump].depth == scopes[jump].depth-scopes[grand].depth ? grand : parent;
    }
    template_pattern_scopes.put(cs,1); template_class_bindings.put(source,cs);
    bind(cs,entities[entity].name,entity);
    auto list = child(n,Kind::Bases);
    for (auto b = ast[list].first; b; b = ast[b].next) {
        auto name = ast[child(b,Kind::BaseName)].detail;
        auto binding = bind_template_name(name,parent);
        template_base_dependence.put(ast.nodes.occurrences[b].source,binding.dependent ? 2 : 1);
        if (binding.dependent) continue;
        auto base = binding.entity;
        if (base && entities[base].kind == EntityKind::Alias) base = types[entities[base].type].entity;
        if (base && entities[base].class_info) complete_class(base);
        if (!base || !target(base)) throw std::runtime_error("invalid fixed pattern base");
        auto access = child(b,Kind::Access);
        auto level = access ? (ast[access].op == KW_PRIVATE ? Access::Private :
            ast[access].op == KW_PROTECTED ? Access::Protected : Access::Public) :
            ast[ast[n].first].op == KW_CLASS ? Access::Private : Access::Public;
        // Definition-time access consumes the fixed base edge without asking
        // for a concrete specialization's layout or giving a local pattern a type.
        bases.push_back({base,template_pattern_bases.get(entity),level});
        template_pattern_bases.put(entity,bases.size()-1);
        add_edge(cs,target(base));
    }
    std::vector<Body> bodies;
    for (auto c = ast[n].first; c; c = ast[c].next) bind_template_declaration(c,cs,deferred ? deferred : &bodies);
    for (auto body : bodies) bind_template_body(body);
    return cs;
}
void Analyzer::bind_template_declaration(NodeId n, ScopeId s, std::vector<Body>* deferred)
{
    auto node = ast[n];
    if (node.kind == Kind::Class || node.kind == Kind::ClassForward) {
        auto cs = bind_template_class(n,s,0,deferred);
        if (!node.detail) for (auto d = scopes[cs].first_decl; d; d = declarations[d].next)
            bind(s,entities[declarations[d].entity].name,declarations[d].entity);
        return;
    }
    if (node.kind == Kind::Enum) {
        auto e = pattern_declaration(EntityKind::Type,s,terminal(node.detail),n,false);
        auto es = make_scope(ScopeKind::Enum,s,entities[e].name,e,false); entities[e].scope = es;
        bool scoped = child(n,Kind::EnumKey);
        for (auto c = node.first; c; c = ast[c].next) if (ast[c].kind == Kind::Enumerator) {
            auto value = pattern_declaration(EntityKind::Enumerator,es,ast[c].text,c,bind_template_expression(ast[c].first,es));
            if (!scoped) bind(s,ast[c].text,value);
        }
        return;
    }
    if (node.kind == Kind::UsingDirective || node.kind == Kind::NamespaceAlias) {
        auto e = resolve(ast[node.first].detail,s,Lookup::Namespace);
        if (!e) throw std::runtime_error("unknown pattern namespace");
        if (node.kind == Kind::UsingDirective) add_edge(s,target(e));
        else { auto alias = pattern_declaration(EntityKind::NamespaceAlias,s,node.text,n,false); entities[alias].scope = target(e); }
        return;
    }
    if (node.kind == Kind::UsingDeclaration) {
        auto name = ast[node.first].detail; auto value = bind_template_name(name,s);
        if (value.dependent) {
            // A repeated terminal qualifier is an inherited-constructor using;
            // it does not introduce an ordinary name into the derived scope.
            auto previous = ast[name].first;
            while (ast[previous].next && ast[previous].next != ast[name].last) previous = ast[previous].next;
            if (ast[previous].text != terminal(name)) pattern_declaration(EntityKind::Function,s,terminal(name),n,true);
        }
        else if (value.entity) bind(s,terminal(name),value.entity);
        else throw std::runtime_error("unbound pattern using-declaration");
        return;
    }
    if (node.kind == Kind::Alias) {
        auto dep = bind_template_expression(node.first,s);
        auto e = pattern_declaration(EntityKind::Alias,s,node.text,n,dep);
        if (!dep) { entities[e].type = type_id(node.first,s); facts[n].type = entities[e].type; }
        return;
    }
    if (node.kind == Kind::StaticAssert) { bind_template_expression(node.first,s); return; }
    if (node.kind == Kind::SimpleDeclaration || node.kind == Kind::Function || node.kind == Kind::ConditionDeclaration ||
        node.kind == Kind::SpecialDefinition || node.kind == Kind::SpecialMember || node.kind == Kind::BitField) {
        auto specs = node.first;
        bool special = node.kind == Kind::SpecialDefinition || node.kind == Kind::SpecialMember;
        bool dependent = false;
        if (!special) for (auto c = ast[specs].first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::Class || ast[c].kind == Kind::ClassForward || ast[c].kind == Kind::Enum)
            {
                if (ast[c].kind == Kind::Enum) bind_template_declaration(c,s,deferred);
                else bind_template_class(c,s,0,deferred);
                if (!ast[c].detail && !ast[child(n,Kind::InitDeclarators)].first) {
                    auto cs = template_class_bindings.get(ast.nodes.occurrences[c].source);
                    for (auto d = scopes[cs].first_decl; d; d = declarations[d].next)
                        bind(s,entities[declarations[d].entity].name,declarations[d].entity);
                }
            }
            else dependent |= bind_template_expression(c,s);
        }
        auto bind_decl = [&](NodeId d, NodeId init, NodeId body) {
            auto name = decl_name(d);
            bool function = child(d,Kind::Parameters) || body || special;
            bool dep = dependent;
            // Parameters bind inside the function scope, not in the surrounding
            // declaration environment. Other declarator operands are type uses.
            for (auto c = ast[d].first; c; c = ast[c].next)
                if (ast[c].kind != Kind::Parameters && ast[c].kind != Kind::Identifier && ast[c].kind != Kind::TrailingReturn)
                    dep |= bind_template_expression(c,s);
            auto kind = spec_has(specs,KW_TYPEDEF) ? EntityKind::Alias : function ? EntityKind::Function : EntityKind::Variable;
            auto id = special && ast[ast[name].last].op != KW_OPERATOR ? 0 : terminal(name);
            auto e = pattern_declaration(kind,s,id,d,dep);
            entities[e].initializer = init;
            entities[e].is_static = spec_has(specs,KW_STATIC);
            if (!dep && !function) {
                bool local_class = false;
                for (auto c = ast[specs].first; c; c = ast[c].next)
                    local_class |= ast[c].kind == Kind::Class || ast[c].kind == Kind::ClassForward || ast[c].kind == Kind::Enum;
                if (!local_class && !child(d,Kind::Array)) {
                    entities[e].type = declarator(d,specifiers(specs,s),s,0,true); facts[d].type = entities[e].type;
                }
            }
            if (body) {
                Body b{body,d,s,e,n}; if (deferred) deferred->push_back(b); else bind_template_body(b);
            } else if (init && bind_template_expression(init,s)) template_pattern_entities.put(e,2);
        };
        if (node.kind == Kind::Function) { auto d = ast[specs].next; bind_decl(d,0,ast[d].next); }
        else if (special) { auto d = child(n,Kind::Declarator); bind_decl(d,0,child(n,Kind::Compound)); }
        else if (node.kind == Kind::ConditionDeclaration) { auto d = ast[specs].next; bind_decl(d,ast[d].next,0); }
        else if (node.kind == Kind::BitField) {
            for (auto c = node.first; c; c = ast[c].next) if (ast[c].kind == Kind::BitFieldDeclarator) {
                auto d = ast[c].first; bind_decl(d,0,0); bind_template_expression(ast[d].next,s);
            }
        } else for (auto c = ast[child(n,Kind::InitDeclarators)].first; c; c = ast[c].next) {
            auto d = ast[c].first; bind_decl(d,ast[d].next,0);
        }
    }
}
} }
