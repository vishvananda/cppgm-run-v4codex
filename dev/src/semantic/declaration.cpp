#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::consume(NodeId n)
{
    facts.resize(ast.nodes.size());
    declaration(n, global);
    // All pending bodies belong to the completed region. No grammar replay.
    while (body_cursor < bodies.size()) {
        Body b = bodies[body_cursor++];
        function_body(b);
    }
    bodies.clear(); body_cursor = 0;
}
void Analyzer::finish() {}
void Analyzer::namespace_declaration(NodeId n, ScopeId s)
{
    IdentifierId name = ast[n].text;
    EntityId e = local(s, name);
    if (e && entities[e].kind != EntityKind::Namespace) throw std::runtime_error("namespace cannot reopen binding/alias");
    if (!e) {
        e = make_entity(EntityKind::Namespace, s, name, n);
        entities[e].scope = make_scope(ScopeKind::Namespace, s, name, e);
        bind(s, name, e);
    }
    ScopeId ns = entities[e].scope;
    if (!name || child(n, Kind::Inline)) add_edge(s, ns, true);
    for (NodeId c = ast[n].first; c; c = ast[c].next) declaration(c, ns);
}
void Analyzer::template_declaration(NodeId n, ScopeId s)
{
    ScopeId ts = make_scope(ScopeKind::Template, s);
    NodeId params = ast[n].first;
    for (NodeId p = ast[ast[params].first].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::TypeParameter) continue;
        NodeId identifier = child(p, Kind::Identifier);
        if (!identifier) continue;
        IdentifierId name = ast[identifier].text;
        EntityId e = make_entity(EntityKind::Type, ts, name, p);
        entities[e].key = child(p, Kind::TemplateTemplate) ? KW_TEMPLATE : KW_TYPENAME;
        entities[e].template_parameter = true;
        entities[e].type = types.named(e);
        bind(ts, name, e); record(ts, e, p, entities[e].type, EntityKind::Type);
    }
    declaration(ast[params].next, ts);
    // A template's declarations are visible outside its parameter environment;
    // parameters themselves are not exported.
    for (std::uint32_t d = scopes[ts].first_decl; d; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (!entities[e].template_parameter) bind(s, entities[e].name, e);
    }
}
void Analyzer::simple(NodeId n, ScopeId s)
{
    NodeId specs = ast[n].first;
    NodeId list = child(n, Kind::InitDeclarators);
    IdentifierId anonymous_name = list ? terminal(decl_name(ast[ast[list].first].first)) : 0;
    TypeId base = specifiers(specs, s, anonymous_name);
    if (ast[n].kind == Kind::Function) {
        NodeId d = ast[specs].next;
        TypeId t = declarator(d, base, s);
        EntityId e = declare_object(d, 0, t, specs, s, n);
        bodies.push_back({ast[d].next, d, entities[e].owner, e});
        return;
    }
    for (NodeId item = ast[list].first; item; item = ast[item].next) {
        NodeId d = ast[item].first;
        TypeId t = declarator(d, base, s);
        declare_object(d, ast[d].next, t, specs, s, n);
    }
}
void Analyzer::declaration(NodeId n, ScopeId s)
{
    ++analyzed;
    switch (ast[n].kind) {
    case Kind::Namespace: namespace_declaration(n, s); break;
    case Kind::NamespaceAlias: {
        EntityId e = resolve(ast[ast[n].first].detail, s, Lookup::Namespace);
        if (!e) throw std::runtime_error("namespace alias target is not a namespace");
        EntityId old = local(s, ast[n].text);
        if (old && (entities[old].kind != EntityKind::NamespaceAlias || target(old) != target(e)))
            throw std::runtime_error("conflicting namespace alias");
        if (!old) {
            EntityId a = make_entity(EntityKind::NamespaceAlias, s, ast[n].text, n);
            entities[a].scope = target(e); bind(s, ast[n].text, a);
        }
        break;
    }
    case Kind::UsingDirective: {
        EntityId e = resolve(ast[ast[n].first].detail, s, Lookup::Namespace);
        if (!e) throw std::runtime_error("using directive target is not a namespace");
        add_edge(s, target(e)); break;
    }
    case Kind::UsingDeclaration: {
        NodeId name = ast[ast[n].first].detail;
        for (NodeId p = ast[name].first; p; p = ast[p].next)
            if (ast[p].first) throw std::runtime_error("using declaration names template-id");
        EntityId e = resolve(name, s);
        if (!e) throw std::runtime_error("unknown using target");
        bind(s, terminal(name), e);
        record(s, e, n, entities[e].type, entities[e].kind);
        break;
    }
    case Kind::Alias: {
        TypeId t = type_id(ast[n].first, s);
        EntityId e = make_entity(EntityKind::Alias, s, ast[n].text, n);
        entities[e].type = t; bind(s, ast[n].text, e); record(s, e, n, t, EntityKind::Alias);
        break;
    }
    case Kind::SimpleDeclaration: case Kind::Function: simple(n, s); break;
    case Kind::Class: case Kind::ClassForward: class_type(n, s); break;
    case Kind::Enum: enum_type(n, s); break;
    case Kind::Template: template_declaration(n, s); break;
    case Kind::StaticAssert: {
        Constant v = evaluate(ast[n].first, s);
        if (!v.valid || scoped_enum(v.type) || !v.bits) throw std::runtime_error("static assertion is not a true integral constant");
        break;
    }
    case Kind::SpecialMember: case Kind::SpecialDefinition: {
        NodeId d = child(n, Kind::Declarator);
        TypeId t = declarator(d, types.fundamental(FT_VOID), s);
        EntityId e = declare_object(d, 0, t, 0, s, n);
        if (ast[n].kind == Kind::SpecialDefinition) {
            NodeId b = child(n, Kind::Compound);
            bodies.push_back({b, d, entities[e].owner, e});
        }
        break;
    }
    case Kind::Linkage: case Kind::ExplicitInstantiation:
        for (NodeId c = ast[n].first; c; c = ast[c].next) declaration(c, s);
        break;
    default: break;
    }
}
void Analyzer::function_body(const Body& body)
{
    ScopeId fs = make_scope(ScopeKind::Function, body.owner, entities[body.entity].name, body.entity);
    NodeId d = body.declarator;
    // The function suffix closest to the declarator name supplies parameters.
    NodeId params = 0;
    while (d) {
        NodeId candidate = child(d, Kind::Parameters);
        if (candidate) params = candidate;
        NodeId nested = child(d, Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    for (NodeId p = ast[params].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        TypeId t = facts[p].type;
        if (types[t].kind == TypeKind::Fundamental && types[t].fundamental == FT_VOID) continue;
        IdentifierId name = terminal(decl_name(ast[ast[p].first].next));
        EntityId e = make_entity(EntityKind::Parameter, fs, name, p);
        entities[e].type = types[t].kind == TypeKind::Array ? types.compound(TypeKind::Pointer, types[t].child) :
            types[t].kind == TypeKind::Function ? types.compound(TypeKind::Pointer, types.signature(t)) : t;
        bind(fs, name, e); record(fs, e, p, t, EntityKind::Parameter);
    }
    statements(body.node, fs);
}
void Analyzer::statements(NodeId n, ScopeId s)
{
    if (!n) return;
    switch (ast[n].kind) {
    case Kind::Compound: {
        ScopeId bs = make_scope(ScopeKind::Block, s);
        for (NodeId c = ast[n].first; c; c = ast[c].next) statements(c, bs);
        break;
    }
    case Kind::SimpleDeclaration: case Kind::Alias: case Kind::UsingDeclaration:
    case Kind::UsingDirective: case Kind::StaticAssert: case Kind::Class: case Kind::ClassForward:
    case Kind::Enum: declaration(n, s); break;
    case Kind::Call: {
        NodeId callee = ast[n].first;
        if (ast[callee].kind == Kind::IdExpression && ast[callee].detail) {
            EntityId e = resolve(ast[callee].detail, s);
            if (e && (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias)) {
                TypeId t = entities[e].type;
                if (types[t].kind == TypeKind::Named && entities[types[t].entity].key != KW_ENUM) {
                    EntityId cls = types[t].entity;
                    ScopeId cs = entities[cls].scope;
                    EntityId ctor = local(cs, entities[cls].name);
                    if (!ctor && !entities[cls].default_constructor)
                        entities[cls].default_constructor = make_scope(ScopeKind::Function, cs, entities[cls].name);
                }
            }
        }
        for (NodeId c = ast[n].first; c; c = ast[c].next) statements(c, s);
        break;
    }
    // PA6 inspects declaration-bearing statement wrappers, not expression types.
    default:
        for (NodeId c = ast[n].first; c; c = ast[c].next) statements(c, s);
        break;
    }
}
} }
