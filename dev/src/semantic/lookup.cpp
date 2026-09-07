#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Analyzer::Analyzer(syntax::Ast& tree, IdentifierTable& identifiers) : ast(tree), ids(identifiers)
{
    entities.push_back(Entity()); scopes.push_back(Scope()); declarations.push_back(Declaration());
    edges.push_back(Edge()); visited.push_back(0);
    global = make_scope(ScopeKind::Namespace, 0);
}
std::uint64_t Analyzer::key(ScopeId s, IdentifierId n) const { return (std::uint64_t(s) << 32) | n; }
EntityId Analyzer::local(ScopeId s, IdentifierId n, Lookup mode) const
{
    return (mode == Lookup::Tag ? tags : mode == Lookup::Namespace ? namespaces : ordinary).get(key(s, n));
}
ScopeId Analyzer::make_scope(ScopeKind k, ScopeId parent, IdentifierId name, EntityId e, bool visible)
{
    Scope sc; sc.kind = k; sc.parent = parent; sc.name = name; sc.entity = e;
    ScopeId id = scopes.size(); scopes.push_back(sc); visited.push_back(0);
    if (visible && parent) attach_scope(id, parent);
    return id;
}
void Analyzer::attach_scope(ScopeId s, ScopeId parent)
{
    if (scopes[parent].last_child) scopes[scopes[parent].last_child].next = s;
    else scopes[parent].first_child = s;
    scopes[parent].last_child = s;
}
EntityId Analyzer::make_entity(EntityKind k, ScopeId s, IdentifierId name, NodeId source)
{
    Entity e; e.kind = k; e.owner = s; e.name = name; e.source = source;
    entities.push_back(e); return entities.size() - 1;
}
void Analyzer::bind(ScopeId s, IdentifierId n, EntityId id)
{
    if (!n && entities[id].kind != EntityKind::Namespace) return;
    EntityKind k = entities[id].kind;
    EntityId old = local(s, n);
    if (old && ((entities[old].kind == EntityKind::Namespace || entities[old].kind == EntityKind::NamespaceAlias) !=
        (k == EntityKind::Namespace || k == EntityKind::NamespaceAlias)))
        throw std::runtime_error("namespace and binding collision");
    ordinary.put(key(s, n), id);
    if (k == EntityKind::Type) tags.put(key(s, n), id);
    if (k == EntityKind::Namespace || k == EntityKind::NamespaceAlias) namespaces.put(key(s, n), id);
}
std::uint32_t Analyzer::record(ScopeId s, EntityId e, NodeId source, TypeId type, EntityKind kind)
{
    Declaration d; d.entity = e; d.source = source; d.type = type; d.kind = kind;
    std::uint32_t id = declarations.size(); declarations.push_back(d);
    if (scopes[s].last_decl) declarations[scopes[s].last_decl].next = id;
    else scopes[s].first_decl = id;
    scopes[s].last_decl = id;
    if (source) { facts[source].entity = e; facts[source].type = type; facts[source].scope = s; }
    return id;
}
void Analyzer::add_edge(ScopeId s, ScopeId to, bool is_inline)
{
    for (std::uint32_t i = scopes[s].first_edge; i; i = edges[i].next)
        if (edges[i].target == to) return;
    Edge e; e.target = to; e.next = scopes[s].first_edge; e.inline_namespace = is_inline;
    scopes[s].first_edge = edges.size(); edges.push_back(e);
}
EntityId Analyzer::imported(ScopeId s, IdentifierId n, Lookup mode, std::uint64_t visit)
{
    if (visited[s] == visit) return 0;
    visited[s] = visit; ++lookup_work;
    EntityId result = local(s, n, mode);
    bool direct = result != 0;
    // Qualified lookup suppresses ordinary using directives after a direct hit;
    // inline namespace edges participate even alongside local declarations.
    for (std::uint32_t i = scopes[s].first_edge; i; i = edges[i].next) {
        if (direct && !edges[i].inline_namespace) continue;
        EntityId found = imported(edges[i].target, n, mode, visit);
        if (found && result && result != found) throw std::runtime_error("ambiguous lookup");
        if (found) result = found;
    }
    return result;
}
EntityId Analyzer::lookup(ScopeId s, IdentifierId n, Lookup mode, bool qualified)
{
    for (; s; s = qualified ? 0 : scopes[s].parent) {
        EntityId found = imported(s, n, mode, ++walk);
        if (found) return found;
    }
    return 0;
}
ScopeId Analyzer::target(EntityId e) const
{
    if (!e) return 0;
    if (entities[e].scope) return entities[e].scope;
    TypeId t = entities[e].type;
    return t && types[t].kind == TypeKind::Named ? entities[types[t].entity].scope : 0;
}
IdentifierId Analyzer::terminal(NodeId n) const { return n ? ast[ast[n].last].text : 0; }
NodeId Analyzer::decl_name(NodeId d) const
{
    for (NodeId c = ast[d].first; c; c = ast[c].next) {
        if (ast[c].kind == Kind::Identifier) return ast[c].detail;
        if (ast[c].kind == Kind::NestedDeclarator) return decl_name(ast[c].first);
    }
    return 0;
}
NodeId Analyzer::child(NodeId n, Kind k) const
{
    for (NodeId c = ast[n].first; c; c = ast[c].next) if (ast[c].kind == k) return c;
    return 0;
}
bool Analyzer::spec_has(NodeId n, ETokenType op) const
{
    for (NodeId c = ast[n].first; c; c = ast[c].next) if (ast[c].op == op) return true;
    return false;
}
bool Analyzer::encloses(ScopeId outer, ScopeId inner) const
{
    for (; inner; inner = scopes[inner].parent) if (inner == outer) return true;
    return false;
}
ScopeId Analyzer::name_owner(NodeId n, ScopeId s)
{
    if (!n) return s;
    bool qualified = ast[n].op == OP_COLON2;
    if (qualified) s = global;
    for (NodeId p = ast[n].first; p && p != ast[n].last; p = ast[p].next) {
        EntityId e = lookup(s, ast[p].text, Lookup::Ordinary, qualified);
        s = target(e);
        if (!s) throw std::runtime_error("name qualifier has no scope");
        qualified = true;
    }
    return s;
}
EntityId Analyzer::resolve(NodeId n, ScopeId s, Lookup mode)
{
    if (!n) return 0;
    // Namespace-only lookup applies to the leading qualifier too.
    if (mode == Lookup::Namespace) {
        bool qualified = ast[n].op == OP_COLON2;
        if (qualified) s = global;
        for (NodeId p = ast[n].first; p; p = ast[p].next) {
            EntityId e = lookup(s, ast[p].text, mode, qualified);
            if (p == ast[n].last) return e;
            s = target(e);
            if (!s) return 0;
            qualified = true;
        }
    }
    return lookup(name_owner(n, s), terminal(n), mode,
                  ast[n].first != ast[n].last || ast[n].op == OP_COLON2);
}
} }
