#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::pattern_declaration(EntityKind kind, ScopeId s, IdentifierId name, NodeId source, bool dependent)
{
    auto e = make_entity(kind,s,name,source);
    entities[e].template_pattern = true;
    template_pattern_entities.put(e,dependent ? 2 : 1);
    bind(s,name,e); record(s,e,source,0,kind); return e;
}
TemplateBinding Analyzer::bind_template_name(NodeId n, ScopeId s)
{
    if (!n) return TemplateBinding();
    auto source = ast.nodes.occurrences[n].source;
    bool pattern = !ast.nodes.occurrences[n].context;
    if (pattern) if (auto id = template_binding_index.get(source)) return template_bindings[id];
    ++template_binding_work;
    TemplateBinding r; r.scope = s;
    auto owner = ast[n].op == OP_COLON2 ? global : s;
    bool qualified = ast[n].op == OP_COLON2;
    for (auto p = ast[n].first; p; p = ast[p].next) {
        if (ast[p].detail && ast[ast[p].detail].kind == Kind::Decltype) {
            r.dependent |= bind_template_expression(ast[ast[p].detail].first,s);
            if (r.dependent) break;
            auto t = expression_type(ast[ast[p].detail].first,s,true);
            owner = types[t].kind == TypeKind::Named ? entities[types[t].entity].scope : 0;
            qualified = true; continue;
        }
        auto e = lookup(owner,p == ast[n].last ? terminal(n) : ast[p].text,p == ast[n].last ? Lookup::Ordinary : Lookup::Qualifier,qualified);
        if (!e) { r.entity = 0; break; }
        r.entity = e;
        auto args = child(p,Kind::TemplateArguments);
        bool class_id = args && entities[e].class_info && (entities[e].template_info || entities[e].specialization);
        r.dependent |= entities[e].template_parameter || entities[e].template_member || (!class_id && template_pattern_entities.get(e) == 2) ||
            (entities[e].template_pattern && !entities[e].type &&
             (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias)) ||
            (entities[e].type && dependent_type(entities[e].type));
        if (args) {
            for (auto a = ast[args].first; a; a = ast[a].next) {
                // The parser retains unresolved template arguments as value
                // syntax when the owning class's aliases are not yet visible.
                if (ast[a].kind == Kind::IdExpression) {
                    auto argument = bind_template_name(ast[a].detail,s);
                    if (argument.dependent || (argument.entity && (entities[argument.entity].kind == EntityKind::Type || entities[argument.entity].kind == EntityKind::Alias))) {
                        r.dependent |= argument.dependent; continue;
                    }
                }
                r.dependent |= bind_template_expression(a,s);
            }
            if (!r.dependent && !entities[e].template_pattern) {
                r.entity = e = class_template_name(p,e,s);
                r.dependent |= entities[e].type && dependent_type(entities[e].type);
            }
        }
        if (p == ast[n].last) break;
        if (r.dependent) { r.entity = 0; break; }
        auto cls = entities[e].class_info ? e : entities[e].kind == EntityKind::Alias ? types[entities[e].type].entity : 0;
        if (cls && entities[cls].class_info) complete_class(cls);
        owner = target(e); qualified = true;
        if (!owner) { r.entity = 0; break; }
    }
    // A retained nested function may first be bound in a concrete enclosing
    // specialization. Those bindings belong to that environment and cannot be
    // published as definition-wide source facts for other specializations.
    if (pattern) { template_binding_index.put(source,template_bindings.size()); template_bindings.push_back(r); }
    return r;
}
bool Analyzer::bind_template_expression(NodeId n, ScopeId s, bool callee)
{
    bool dependent = bind_template_expression_impl(n,s,callee);
    if (n && !dependent && !ast.nodes.occurrences[n].context)
        check_fixed_expression(n,s);
    return dependent;
}
bool Analyzer::bind_template_expression_impl(NodeId n, ScopeId s, bool callee)
{
    if (!n) return false;
    ++template_binding_work;
    auto node = ast[n];
    if (node.kind == Kind::IdExpression) {
        auto name = node.detail;
        if (callee && fundamental_cast_type(node.op)) return false;
        if (callee && ast[name].kind == Kind::TypeId) return bind_template_expression(name,s);
        auto binding = bind_template_name(name,s);
        auto e = binding.entity;
        if (!binding.dependent && check_template_field(n,s,e)) return false;
        if (!binding.dependent && !e && !callee) throw std::runtime_error("unbound template value name");
        if (e && !callee && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias ||
            entities[e].kind == EntityKind::Namespace || entities[e].kind == EntityKind::NamespaceAlias))
            throw std::runtime_error("template expression requires a value name");
        return binding.dependent;
    }
    if (node.kind == Kind::Call) {
        bool dependent = false;
        auto fn = node.first;
        for (auto a = ast[ast[fn].next].first; a; a = ast[a].next) dependent |= bind_template_expression(a,s);
        dependent |= bind_template_expression(fn,s,true);
        if (ast[fn].kind == Kind::IdExpression && !fundamental_cast_type(ast[fn].op) && ast[ast[fn].detail].kind != Kind::TypeId) {
            auto name = ast[fn].detail; auto binding = bind_template_name(name,s);
            auto id = terminal(name);
            if (!binding.entity && !binding.dependent &&
                (!dependent || ast[name].first != ast[name].last || ast[name].op == OP_COLON2) &&
                id != constant_builtin && id != abort_builtin)
                query_fact(expression_query(n,s));
        }
        return dependent;
    }
    if (node.kind == Kind::Member) {
        bool dependent = bind_template_expression(node.first,s);
        auto name = ast[ast[node.first].next].detail;
        for (auto p = ast[name].first; p; p = ast[p].next)
            if (auto args = child(p,Kind::TemplateArguments)) dependent |= bind_template_expression(args,s);
        auto receiver = node.first;
        while (ast[receiver].kind == Kind::Parenthesized) receiver = ast[receiver].first;
        bool current = node.op == OP_ARROW && ast[receiver].kind == Kind::KeywordLiteral && ast[receiver].op == KW_THIS;
        if (node.op == OP_DOT && ast[receiver].kind == Kind::Unary && ast[receiver].op == OP_STAR) {
            receiver = ast[receiver].first;
            while (ast[receiver].kind == Kind::Parenthesized) receiver = ast[receiver].first;
            current = ast[receiver].kind == Kind::KeywordLiteral && ast[receiver].op == KW_THIS;
        }
        auto object = template_object_context(s);
        if (current && object.owner && ast[name].first == ast[name].last && !child(ast[name].last,Kind::TemplateArguments)) {
            auto field = lookup(entities[object.owner].scope,terminal(name),Lookup::Ordinary,true);
            if (check_template_field(n,s,field,true)) return false;
        }
        return dependent;
    }
    if (node.kind == Kind::Name) return bind_template_name(n,s).dependent;
    if (node.kind == Kind::Identifier) return false; // A declaration's own name.
    if (node.kind == Kind::Sizeof || node.kind == Kind::TypeTrait) {
        ++unevaluated_depth; bool dependent = false;
        try { for (auto c = node.first; c; c = ast[c].next) dependent |= bind_template_expression(c,s); }
        catch (...) { --unevaluated_depth; throw; }
        --unevaluated_depth; return dependent;
    }
    if (node.kind == Kind::KeywordLiteral && node.op == KW_THIS) {
        auto object = template_object_context(s);
        if (object.owner && !object.available) throw std::runtime_error("this in static template member");
    }
    bool dependent = node.kind == Kind::KeywordLiteral && node.op == KW_THIS;
    if (node.detail) dependent |= bind_template_expression(node.detail,s);
    for (auto c = node.first; c; c = ast[c].next) dependent |= bind_template_expression(c,s);
    return dependent;
}
void Analyzer::bind_template_body(const Body& body)
{
    auto source = ast.nodes.occurrences[body.node].source;
    auto state = template_bound_bodies.get(source);
    if (state == unsigned(FactState::Success)) return;
    if (state == unsigned(FactState::Failure)) throw std::runtime_error("failed template body binding");
    if (state == unsigned(FactState::Active)) throw std::runtime_error("recursive template body binding");
    template_bound_bodies.put(source,unsigned(FactState::Active));
    try {
    auto owner = entities[body.entity].template_info ? templates[entities[body.entity].template_info].environment : body.owner;
    auto fs = make_scope(ScopeKind::Function,owner,entities[body.entity].name,body.entity,false);
    template_pattern_scopes.put(fs,1);
    auto d = body.declarator; NodeId params = 0;
    while (d) {
        if (auto p = child(d,Kind::Parameters)) params = p;
        auto nested = child(d,Kind::NestedDeclarator); d = nested ? ast[nested].first : 0;
    }
    bind_template_object_context(fs,params);
    for (auto p = ast[params].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        auto specs = ast[p].first, decl = ast[specs].next;
        bool dependent = bind_template_expression(specs,fs) | bind_template_expression(decl,fs);
        auto declared_type = facts[p].type;
        auto e = pattern_declaration(EntityKind::Parameter,fs,terminal(decl_name(decl)),p,dependent);
        if (declared_type) {
            facts[p].type = declared_type;
            entities[e].type = parameter_body_type(declared_type);
        }
        else if (!dependent) entities[e].type = parameter_body_type(declarator(decl,specifiers(specs,fs),fs));
    }
    bind_template_statement(body.node,fs);
    check_jumps(body.node,true);
    template_bound_bodies.put(source,unsigned(FactState::Success));
    } catch (...) { template_bound_bodies.put(source,unsigned(FactState::Failure)); throw; }
}
} }
