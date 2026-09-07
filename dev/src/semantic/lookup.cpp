#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Analyzer::Analyzer(syntax::Ast& tree, IdentifierTable& identifiers, bool with_calls) : ast(tree), ids(identifiers), calls(with_calls)
{
    entities.push_back(Entity()); scopes.push_back(Scope()); declarations.push_back(Declaration());
    edges.push_back(Edge()); visited.push_back(0); constants.resize(2); class_facts.resize(1); members.resize(1); bases.push_back({0,0}); actions.push_back({0,0,0});
    constant_builtin = ids.intern(TextView("__builtin_constant_p", 20));
    abort_builtin = ids.intern(TextView("__builtin_abort", 15));
    templates.resize(1); argument_packs.resize(1); specializations.resize(1);
    conversions.push_back(Conversion());
    global = make_scope(ScopeKind::Namespace, 0);
    if (calls) {
        IdentifierId name = ids.intern(TextView("nullptr_t", 9));
        EntityId e = make_entity(EntityKind::Alias, global, name, 0);
        entities[e].type = types.fundamental(FT_NULLPTR_T); bind(global, name, e);
    }
}
std::uint64_t Analyzer::key(ScopeId s, IdentifierId n) const { return (std::uint64_t(s) << 32) | n; }
EntityId Analyzer::local(ScopeId s, IdentifierId n, Lookup mode) const
{
    return (mode == Lookup::Tag ? tags : mode == Lookup::Namespace ? namespaces : mode == Lookup::Qualifier ? qualifiers : ordinary).get(key(s, n));
}
ScopeId Analyzer::make_scope(ScopeKind k, ScopeId parent, IdentifierId name, EntityId e, bool visible)
{
    Scope sc; sc.kind = k; sc.parent = parent; sc.name = name; sc.entity = e;
    sc.depth = scopes[parent].depth + 1;
    ScopeId pjump = scopes[parent].jump;
    ScopeId grandjump = scopes[pjump].jump;
    sc.jump = scopes[parent].depth - scopes[pjump].depth == scopes[pjump].depth - scopes[grandjump].depth ?
        grandjump : parent;
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
    if (calls && n && scopes[s].kind == ScopeKind::Block) {
        ScopeId parent = scopes[s].parent;
        if ((scopes[parent].kind == ScopeKind::Function || scopes[parent].kind == ScopeKind::Control) && local(parent, n))
            throw std::runtime_error("redeclaration in outermost statement block");
    }
    if (calls && old && old != id && entities[old].kind == EntityKind::Parameter && k == EntityKind::Parameter)
        throw std::runtime_error("duplicate parameter name");
    if (old && ((entities[old].kind == EntityKind::Namespace || entities[old].kind == EntityKind::NamespaceAlias) !=
        (k == EntityKind::Namespace || k == EntityKind::NamespaceAlias)))
        throw std::runtime_error("namespace and binding collision");
    if (old != id && function_binding(old) && function_binding(id)) id = merge_lookup(old, id);
    else if (old && old != id && (function_binding(old) || function_binding(id)) &&
             entities[old].kind != EntityKind::Type && k != EntityKind::Type)
        throw std::runtime_error("function and ordinary binding conflict");
    ordinary.put(key(s, n), id);
    if (target(id)) qualifiers.put(key(s, n), id);
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
    std::uint32_t existing = edge_index.get(key(s, to));
    if (existing) {
        if (is_inline && !edges[existing].inline_namespace) {
            edges[existing].inline_next = scopes[s].first_inline;
            scopes[s].first_inline = existing;
        }
        edges[existing].inline_namespace |= is_inline;
        return;
    }
    Edge e; e.target = to; e.next = scopes[s].first_edge; e.inline_namespace = is_inline;
    if (is_inline) {
        e.inline_next = scopes[s].first_inline;
        scopes[s].first_inline = edges.size();
    }
    edge_index.put(key(s, to), edges.size());
    scopes[s].first_edge = edges.size(); edges.push_back(e);
}
EntityId Analyzer::merge_lookup(EntityId a, EntityId b)
{
    const EntityId ambiguous = ~EntityId(0);
    if (!a || a == b) return b;
    if (!b) return a;
    if (a == ambiguous || b == ambiguous) return ambiguous;
    if (function_binding(a) && function_binding(b)) {
        EntityId e = make_entity(EntityKind::Overload, 0, entities[a].name, 0);
        entities[e].first = a; entities[e].second = b;
        return e;
    }
    // The PA6 contract distinguishes independently declared aliases, including
    // aliases for the same type. Multiple paths to one entity remain unique.
    return ambiguous;
}
EntityId Analyzer::imported(ScopeId s, IdentifierId n, Lookup mode, std::uint64_t visit)
{
    if (visited[s] == visit) return 0;
    // Search the entire inline namespace set before ordinary directives. A hit
    // anywhere in that set suppresses directives everywhere in the set.
    std::size_t begin = qualified_work.size();
    qualified_work.push_back(s);
    visited[s] = visit;
    EntityId result = 0;
    for (std::size_t p = begin; p < qualified_work.size(); ++p) {
        ScopeId current = qualified_work[p];
        ++lookup_work;
        result = merge_lookup(result, local(current, n, mode));
        for (std::uint32_t i = scopes[current].first_inline; i; i = edges[i].inline_next) {
            ScopeId to = edges[i].target;
            if (edges[i].inline_namespace && visited[to] != visit) {
                visited[to] = visit;
                qualified_work.push_back(to);
            }
        }
    }
    if (!result) {
        std::size_t end = qualified_work.size();
        for (std::size_t p = begin; p < end; ++p)
            for (std::uint32_t i = scopes[qualified_work[p]].first_edge; i; i = edges[i].next)
                if (!edges[i].inline_namespace)
                    result = merge_lookup(result, imported(edges[i].target, n, mode, visit));
    }
    qualified_work.resize(begin);
    if (result == ~EntityId(0)) throw std::runtime_error("ambiguous lookup");
    return result;
}
ScopeId Analyzer::common_ancestor(ScopeId a, ScopeId b) const
{
    // One geometric jump link per scope gives logarithmic ancestor walks.
    while (scopes[a].depth > scopes[b].depth)
        a = scopes[scopes[a].jump].depth >= scopes[b].depth ? scopes[a].jump : scopes[a].parent;
    while (scopes[b].depth > scopes[a].depth)
        b = scopes[scopes[b].jump].depth >= scopes[a].depth ? scopes[b].jump : scopes[b].parent;
    while (a != b) {
        bool jump = scopes[a].jump != scopes[b].jump;
        a = jump ? scopes[a].jump : scopes[a].parent;
        b = jump ? scopes[b].jump : scopes[b].parent;
    }
    return a;
}
EntityId Analyzer::lookup(ScopeId s, IdentifierId n, Lookup mode, bool qualified)
{
    if (qualified) return imported(s, n, mode, ++walk);
    // Nominated declarations participate at the nearest common ancestor of
    // their namespace and the active directive, not at the directive's scope.
    // Scratch state visits only active edges; no snapshot or semantic cache.
    Index pending;
    std::vector<ScopeId> work;
    const EntityId ambiguous = ~EntityId(0);
    std::uint64_t visit = ++walk;
    for (; s; s = scopes[s].parent) {
        ++lookup_work;
        work.clear();
        for (std::uint32_t edge = scopes[s].first_edge; edge; edge = edges[edge].next)
            work.push_back(edges[edge].target);
        for (std::size_t i = 0; i < work.size(); ++i) {
            ScopeId ns = work[i];
            if (visited[ns] == visit) continue;
            visited[ns] = visit; ++lookup_work;
            EntityId found = local(ns, n, mode);
            if (found) {
                ScopeId anchor = common_ancestor(s, ns);
                EntityId previous = pending.get(anchor);
                pending.put(anchor, merge_lookup(previous, found));
            }
            for (std::uint32_t edge = scopes[ns].first_edge; edge; edge = edges[edge].next)
                work.push_back(edges[edge].target);
        }
        EntityId direct = local(s, n, mode), nominated = pending.get(s);
        EntityId result = merge_lookup(direct, nominated);
        if (result == ambiguous)
            throw std::runtime_error("ambiguous unqualified lookup");
        if (result) return result;
    }
    return 0;
}
ScopeId Analyzer::target(EntityId e) const
{
    if (!e) return 0;
    EntityKind kind = entities[e].kind;
    if (kind != EntityKind::Type && kind != EntityKind::Alias && kind != EntityKind::Namespace && kind != EntityKind::NamespaceAlias)
        return 0;
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
        EntityId e = lookup(s, ast[p].text, Lookup::Qualifier, qualified);
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
