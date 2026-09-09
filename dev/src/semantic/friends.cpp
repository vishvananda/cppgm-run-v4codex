#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::friend_declaration(NodeId n, ScopeId s)
{
    NodeId specs = ast[n].first;
    if (!spec_has(specs, KW_FRIEND)) return false;
    ScopeId ns = s;
    while (scopes[ns].kind != ScopeKind::Namespace) ns = scopes[ns].parent;
    EntityId cls = scopes[s].entity;
    NodeId friend_type = child(specs, Kind::ClassForward);
    if (friend_type) {
        NodeId name = ast[friend_type].detail;
        EntityId target = resolve(name, s, Lookup::Tag);
        if (!target) {
            if (ast[name].first != ast[name].last) throw std::runtime_error("undeclared qualified friend class");
            TypeId type = class_type(friend_type, ns);
            target = types[type].entity;
        }
        if (!entities[target].class_info) throw std::runtime_error("friend is not a class");
        friendships.put(key(cls, target), 1); return true;
    }
    TypeId base = specifiers(specs, s);
    NodeId list = child(n, Kind::InitDeclarators);
    auto add = [&](NodeId d, NodeId body) {
        TypeId type = types.signature(declarator(d, base, s));
        if (types[type].kind != TypeKind::Function) throw std::runtime_error("friend declaration is not a function");
        NodeId name = decl_name(d);
        bool qualified = ast[name].first != ast[name].last || ast[name].op == OP_COLON2;
        ScopeId owner = qualified ? name_owner(name, s) : ns;
        EntityId function = 0;
        if (qualified) {
            EntityId found = lookup(owner, terminal(name), Lookup::Ordinary, true);
            if (function_binding(found)) for (EntityId candidate : candidates(found))
                if (entities[candidate].type == type) function = candidate;
            if (!function) throw std::runtime_error("qualified friend must match a declared function");
        } else function = declare_function(owner, terminal(name), n, type, true);
        friendships.put(key(cls, function), 1);
        if (!qualified) hidden_friends.put(key(cls, terminal(name)), merge_lookup(hidden_friends.get(key(cls, terminal(name))), function));
        function_defaults(function, d, s); exception_specification(function, d, s);
        record(s, function, d, type, EntityKind::Function);
        if (body) {
            entities[function].inline_function = true;
            schedule_body({body, d, s, function, n});
        }
    };
    if (ast[n].kind == Kind::Function) { NodeId d = ast[specs].next; add(d, ast[d].next); }
    else for (NodeId item = ast[list].first; item; item = ast[item].next) add(ast[item].first, 0);
    return true;
}
EntityId Analyzer::associated_lookup(IdentifierId name, const std::vector<NodeId>& args)
{
    Index seen_types, seen_scopes;
    std::vector<TypeId> work;
    std::vector<ScopeId> spaces;
    EntityId result = 0;
    for (NodeId n : args) if (expressions[n].type) work.push_back(expressions[n].type);
    for (std::size_t i = 0; i < work.size(); ++i) {
        TypeId id = work[i];
        if (seen_types.get(id)) continue;
        seen_types.put(id, 1); auto type = types[id];
        if (type.kind == TypeKind::Pointer || type.kind == TypeKind::LRef || type.kind == TypeKind::RRef || type.kind == TypeKind::Array) work.push_back(type.child);
        else if (type.kind == TypeKind::Function) {
            work.push_back(type.child);
            for (unsigned j = 0; j < type.count; ++j) work.push_back(types.parameters[type.offset+j]);
        } else if (type.kind == TypeKind::Named) {
            EntityId cls = type.entity;
            if (entities[cls].class_info) {
                result = merge_lookup(result, hidden_friends.get(key(cls, name)));
                for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) work.push_back(entities[bases[b].base].type);
            }
            ScopeId owner = entities[cls].owner;
            if (scopes[owner].kind == ScopeKind::Class) work.push_back(entities[scopes[owner].entity].type);
            while (owner && scopes[owner].kind != ScopeKind::Namespace) owner = scopes[owner].parent;
            if (owner) spaces.push_back(owner);
        }
    }
    for (std::size_t i = 0; i < spaces.size(); ++i) {
        ScopeId ns = spaces[i];
        if (seen_scopes.get(ns)) continue;
        seen_scopes.put(ns, 1);
        EntityId found = local(ns, name);
        if (function_binding(found)) result = merge_lookup(result, found);
        for (auto edge = scopes[ns].first_inline; edge; edge = edges[edge].inline_next) spaces.push_back(edges[edge].target);
        ScopeId parent = scopes[ns].parent;
        auto edge = edge_index.get(key(parent, ns));
        if (edge && edges[edge].inline_namespace) spaces.push_back(parent);
    }
    return result;
}
} }
