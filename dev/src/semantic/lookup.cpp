#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
Analyzer::Analyzer(syntax::Ast& tree, IdentifierTable& identifiers, bool with_calls, bool with_definitions, bool host) : ast(tree), definitions(with_definitions), ids(identifiers), calls(with_calls), host_abi(host)
{
    entities.push_back(Entity()); scopes.push_back(Scope()); declarations.push_back(Declaration());
    edges.push_back(Edge()); visited.push_back(0); constants.resize(2); class_facts.resize(1); members.resize(1); bases.push_back({0,0}); actions.push_back({0,0,0});
    constant_builtin = ids.intern(TextView("__builtin_constant_p", 20));
    abort_builtin = ids.intern(TextView("__builtin_abort", 15));
    invoke_builtin = ids.intern(TextView("__builtin_invoke", 16));
    expect_builtin = ids.intern(TextView("__builtin_expect", 16));
    templates.resize(1); argument_packs.resize(1); specializations.resize(1);
    conversions.push_back(Conversion());
    global = make_scope(ScopeKind::Namespace, 0);
    // SysV AMD64 va_list is an array of one record, not a pointer alias. Keep
    // the ABI shape in ordinary typed class/field facts for layout and decay.
    auto tag_name = ids.intern(TextView("__va_list_tag",13));
    auto tag = make_entity(EntityKind::Type,global,tag_name,0);
    entities[tag].key = KW_STRUCT; entities[tag].type = types.named(tag);
    entities[tag].scope = make_scope(ScopeKind::Class,global,tag_name,tag,false);
    entities[tag].class_info = class_facts.size(); class_facts.push_back(ClassFacts());
    auto field_scope = entities[tag].scope;
    auto word = types.fundamental(FT_UNSIGNED_INT);
    auto ptr = types.compound(TypeKind::Pointer,types.fundamental(FT_VOID));
    const char* field_names[] = {"gp_offset","fp_offset","overflow_arg_area","reg_save_area"};
    for (unsigned i = 0; i < 4; ++i) {
        auto field_name = ids.intern(TextView(field_names[i],std::char_traits<char>::length(field_names[i])));
        auto field = make_entity(EntityKind::Variable,field_scope,field_name,0);
        entities[field].type = i < 2 ? word : ptr;
        bind(field_scope,field_name,field);
        record(field_scope,field,0,entities[field].type,EntityKind::Variable);
    }
    entities[tag].complete = true;
    class_facts[entities[tag].class_info].definition_state = FactState::Success;
    auto va_name = ids.intern(TextView("__builtin_va_list",17));
    auto va = make_entity(EntityKind::Alias,global,va_name,0);
    variadic_list_type = entities[va].type = types.compound(TypeKind::Array,entities[tag].type,1);
    bind(global,va_name,va);
    if (calls) {
        IdentifierId name = ids.intern(TextView("nullptr_t", 9));
        EntityId e = make_entity(EntityKind::Alias, global, name, 0);
        entities[e].type = types.fundamental(FT_NULLPTR_T); bind(global, name, e);
    }
}
std::uint64_t Analyzer::key(ScopeId s, IdentifierId n) const { return (std::uint64_t(s) << 32) | n; }
EntityId Analyzer::local(ScopeId s, IdentifierId n, Lookup mode) const
{
    auto e = (mode == Lookup::Tag ? tags : mode == Lookup::Namespace ? namespaces : mode == Lookup::Qualifier ? qualifiers : ordinary).get(key(s, n));
    // [temp.local]/4: inherited injected names of different specializations
    // denote one primary when used as a template-name. Normalize at the local
    // producer, before merging base lookup sets; ordinary type lookup stays
    // ambiguous. No unrelated declarations or specializations are searched.
    if (mode == Lookup::Template && e && scopes[s].kind == ScopeKind::Class &&
        scopes[s].entity == e && entities[e].specialization) return specialization_pattern(e);
    return e;
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
    Entity e; if (calls && source) e.access = declaration_access(s); e.kind = k; e.owner = s; e.name = name; e.source = source;
    if (definitions && scopes[s].kind == ScopeKind::Class) {
        e.template_member = definition_owner(scopes[s].entity).specialization != 0;
        if (source && !ast.nodes.occurrences[source].context && pattern_scope(s))
            e.template_member = e.template_pattern = true;
    }
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
    if (old != id && function_binding(old) && function_binding(id)) {
        id = merge_lookup(old, id);
        if (calls && using_functions.get(key(s, n)) &&
            (scopes[s].kind == ScopeKind::Class || scopes[s].kind == ScopeKind::Namespace)) {
            // A using-declaration retains canonical function identities. Local
            // members hide imported base signatures regardless of declaration
            // order; a namespace declaration of an imported signature conflicts.
            auto shape = [&](EntityId e) {
                Type t = types[entities[e].type];
                std::vector<TypeId> params(types.parameters.begin() + t.offset, types.parameters.begin() + t.offset + t.count);
                auto head = entities[e].template_info;
                // Namespace template conflicts include the return type;
                // base-member hiding and ordinary function conflicts do not.
                auto result = head && scopes[s].kind == ScopeKind::Namespace ? t.child : types.fundamental(FT_VOID);
                auto type = types.function(result,params,t.variadic,t.cv,t.ref);
                if (!head) return key(0,type);
                auto identity = key(head,type);
                auto shape = using_template_shapes.get(identity);
                if (!shape) {
                    shape = template_declaration_shape(type,templates[head].environment);
                    using_template_shapes.put(identity,shape);
                }
                return key(1,shape);
            };
            std::vector<EntityId> all = candidates(id);
            Index owned;
            for (EntityId e : all) if (entities[e].owner == s) owned.put(shape(e), e);
            EntityId kept = 0;
            for (EntityId e : all) {
                if (entities[e].owner != s && owned.get(shape(e))) {
                    if (scopes[s].kind == ScopeKind::Namespace) throw std::runtime_error("function conflicts with namespace using-declaration");
                    continue;
                }
                kept = merge_lookup(kept, e);
            }
            id = kept;
        }
    }
    else if (old && old != id && (function_binding(old) || function_binding(id)) &&
             entities[old].kind != EntityKind::Type && k != EntityKind::Type)
        throw std::runtime_error("function and ordinary binding conflict");
    bool hidden_tag = k == EntityKind::Type && old &&
        (function_binding(old) || entities[old].kind == EntityKind::Variable || entities[old].kind == EntityKind::Enumerator);
    if (!hidden_tag) ordinary.put(key(s, n), id);
    if ((entities[id].template_pattern && (k == EntityKind::Type || k == EntityKind::Alias)) || target(id) || (definitions && (k == EntityKind::Type || k == EntityKind::Alias) && dependent_type(entities[id].type)))
        qualifiers.put(key(s, n), id);
    if (k == EntityKind::Type) tags.put(key(s, n), id);
    if (k == EntityKind::Namespace || k == EntityKind::NamespaceAlias) namespaces.put(key(s, n), id);
}
std::uint32_t Analyzer::record(ScopeId s, EntityId e, NodeId source, TypeId type, EntityKind kind)
{
    if (type && type != types.signature(type)) declared_storage_types.put(e,type);
    Declaration d; d.entity = e; d.source = source; d.type = type; d.kind = kind;
    std::uint32_t id = declarations.size(); declarations.push_back(d);
    if (scopes[s].last_decl) declarations[scopes[s].last_decl].next = id;
    else scopes[s].first_decl = id;
    scopes[s].last_decl = id;
    if (source) {
        { auto& published = facts.edit(source); published.entity = e; published.type = type; published.scope = s; }
        if (definitions) publish_template_binding(source,e);
    }
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
void Analyzer::inject_class(ScopeId owner, ScopeId members)
{
    add_edge(owner,members);
    auto edge = edge_index.get(key(owner,members));
    if (edges[edge].injected_member) return;
    edges[edge].injected_member = true;
    edges[edge].inline_next = scopes[owner].first_inline;
    scopes[owner].first_inline = edge;
    injected_class_owners.put(scopes[members].entity,scopes[owner].entity);
}
EntityId Analyzer::merge_lookup(EntityId a, EntityId b)
{
    const EntityId ambiguous = ~EntityId(0);
    if (!a || a == b) return b;
    if (!b) return a;
    if (a == ambiguous || b == ambiguous) return ambiguous;
    auto namespace_name = [&](EntityId e) {
        return entities[e].kind == EntityKind::Namespace || entities[e].kind == EntityKind::NamespaceAlias;
    };
    if (namespace_name(a) && namespace_name(b) && target(a) == target(b)) return a;
    if (function_binding(a) && function_binding(b)) {
        EntityId e = make_entity(EntityKind::Overload, 0, entities[a].name, 0);
        entities[e].first = a; entities[e].second = b;
        entities[e].template_pattern = entities[a].template_pattern || entities[b].template_pattern;
        entities[e].template_member = entities[a].template_member || entities[b].template_member;
        if (entities[e].template_pattern) template_pattern_entities.put(e,
            template_pattern_entities.get(a) == 2 || template_pattern_entities.get(b) == 2 ? 2 : 1);
        return e;
    }
    // [dcl.typedef] aliases name their associated type, not a new entity.
    // [class.member.lookup]/3 likewise replaces type declarations by their
    // types before merging base lookup sets. Keep a declaration representative
    // for the subsequent access check; alias templates are distinct entities.
    auto type_name = [&](EntityId e) {
        return !entities[e].template_info &&
            (entities[e].kind == EntityKind::Type || entities[e].kind == EntityKind::Alias);
    };
    if (type_name(a) && type_name(b) && entities[a].type && entities[a].type == entities[b].type) return a;
    return ambiguous;
}
EntityId Analyzer::imported(ScopeId s, IdentifierId n, Lookup mode, std::uint64_t visit)
{
    if (visited[s] == visit) return 0;
    // Inline namespaces and anonymous class members contribute direct names.
    // Search their indexed edges before directives or base-class edges.
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
            if ((edges[i].inline_namespace || edges[i].injected_member) && visited[to] != visit) {
                visited[to] = visit;
                qualified_work.push_back(to);
            }
        }
    }
    if (!result) {
        std::size_t end = qualified_work.size();
        for (std::size_t p = begin; p < end; ++p)
            for (std::uint32_t i = scopes[qualified_work[p]].first_edge; i; i = edges[i].next)
                if (!edges[i].inline_namespace && !edges[i].injected_member)
                    result = merge_lookup(result, imported(edges[i].target, n, mode, visit));
    }
    qualified_work.resize(begin);
    // Keep ambiguity as a compact result through the whole indexed traversal.
    // Ordinary lookup diagnoses it; immediate substitution consumes failure.
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
    if (definitions && scopes[s].kind == ScopeKind::Class) complete_class(scopes[s].entity);
    if (qualified) {
        auto found = imported(s, n, mode, ++walk);
        if (!found && s == global && mode != Lookup::Tag && mode != Lookup::Namespace) found = builtin_type_template(n);
        if (found == ~EntityId(0)) throw std::runtime_error("ambiguous lookup");
        return found;
    }
    // Nominated declarations participate at the nearest common ancestor of
    // their namespace and the active directive, not at the directive's scope.
    // Scratch state visits only active edges; no snapshot or semantic cache.
    Index pending;
    std::vector<ScopeId> work;
    const EntityId ambiguous = ~EntityId(0);
    std::uint64_t visit = ++walk;
    for (; s; s = scopes[s].parent) {
        ++lookup_work;
        if (calls && scopes[s].kind == ScopeKind::Class) {
            if (EntityId found = imported(s, n, mode, ++walk)) {
                if (found == ambiguous) throw std::runtime_error("ambiguous lookup");
                return found;
            }
            continue;
        }
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
        if (!direct && s == global && mode != Lookup::Tag && mode != Lookup::Namespace) direct = builtin_type_template(n);
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
IdentifierId Analyzer::terminal(NodeId n)
{
    NodeId part = ast.last(n);
    if (calls && ast.op(part) == KW_OPERATOR && ast.kind(ast.first(part)) == Kind::Literal)
        return literal_name(ast.text(ast.last(part)));
    ETokenType op = operator_token(n);
    return calls && op != TOK_INVALID ? operator_name(op,array_operator(n)) : n ? ast.text(ast.last(n)) : 0;
}
NodeId Analyzer::decl_name(NodeId d) const
{
    for (NodeId c = ast.first(d); c; c = ast.next(c)) {
        if (ast.kind(c) == Kind::Identifier) return ast.detail(c);
        if (ast.kind(c) == Kind::NestedDeclarator) return decl_name(ast.first(c));
    }
    return 0;
}
NodeId Analyzer::child(NodeId n, Kind k) const
{
    for (NodeId c = ast.first(n); c; c = ast.next(c)) if (ast.kind(c) == k) return c;
    return 0;
}
NodeId Analyzer::declarator_pack(NodeId n) const
{
    // Parenthesized pointer/reference declarators retain the ellipsis beside
    // their identifier. Function parameter lists inside the declarator own
    // separate packs and must not be searched here.
    while (n) {
        if (auto pack = child(n,Kind::ParameterPack)) return pack;
        auto nested = child(n,Kind::NestedDeclarator);
        n = ast.first(nested);
    }
    return 0;
}
bool Analyzer::spec_has(NodeId n, ETokenType op) const
{
    for (NodeId c = ast.first(n); c; c = ast.next(c)) if (ast.op(c) == op) return true;
    return false;
}
bool Analyzer::encloses(ScopeId outer, ScopeId inner) const
{
    for (; inner; inner = scopes[inner].parent) if (inner == outer) return true;
    return false;
}
ScopeId Analyzer::name_owner(NodeId n, ScopeId s, bool declaration)
{
    if (!n) return s;
    ScopeId context = s;
    bool qualified = ast.op(n) == OP_COLON2;
    if (qualified) s = global;
    for (NodeId p = ast.first(n); p && p != ast.last(n); p = ast.next(p)) {
        if (ast.kind(ast.detail(p)) == Kind::Decltype) {
            TypeId t = expression_type(ast.first(ast.detail(p)), context, true);
            if (types[t].kind != TypeKind::Named) throw std::runtime_error("decltype qualifier is not a class");
            s = entities[types[t].entity].scope; qualified = true; continue;
        }
        EntityId e = lookup(s, ast.text(p), child(p,Kind::TemplateArguments) ? Lookup::Template : Lookup::Qualifier, qualified);
        if (definitions) e = class_template_name(p,e,context);
        if (definitions && e) {
            auto type = entities[e].kind == EntityKind::Alias ? source_type(e) : entities[e].type;
            auto cls = entities[e].class_info ? e : types[type].kind == TypeKind::Named ? types[type].entity : 0;
            if (cls && entities[cls].class_info) complete_class(cls);
        }
        if (calls && e && !declaration) check_access(e, context, s);
        s = target(e);
        if (!s) throw std::runtime_error("name qualifier has no scope");
        qualified = true;
    }
    return s;
}
EntityId Analyzer::resolve(NodeId n, ScopeId s, Lookup mode)
{
    if (!n) return 0;
    if (mode == Lookup::Ordinary && ast.op(ast.last(n)) == KW_OPERATOR && ast.detail(ast.last(n)))
        return resolve_conversion_name(n,s);
    // A decltype-specifier can be the complete class-or-decltype in a base,
    // not just an intermediate nested-name qualifier. Its query owns the
    // selected type; there is no terminal identifier to look up.
    if (ast.kind(ast.detail(ast.last(n))) == Kind::Decltype) {
        auto type = type_name(n,s);
        return types[type].kind == TypeKind::Named ? types[type].entity : 0;
    }
    if (definitions && (mode == Lookup::Ordinary || mode == Lookup::Qualifier) && ast.nodes.occurrences[n].context) {
        auto id = template_binding_index.get(ast.nodes.occurrences[n].source);
        if (id) {
            auto binding = template_bindings[id]; auto e = binding.entity;
            if (e && entities[e].template_info && entities[e].class_info && child(ast.last(n),syntax::Kind::TemplateArguments))
                return class_template_name(ast.last(n),e,s);
            if (e && !binding.dependent && !entities[e].template_pattern && !entities[e].template_parameter) return e;
        }
    }
    // Namespace-only lookup applies to the leading qualifier too.
    if (mode == Lookup::Namespace) {
        bool qualified = ast.op(n) == OP_COLON2;
        if (qualified) s = global;
        for (NodeId p = ast.first(n); p; p = ast.next(p)) {
            EntityId e = lookup(s, ast.text(p), mode, qualified);
            if (p == ast.last(n)) return e;
            s = target(e);
            if (!s) return 0;
            qualified = true;
        }
    }
    ScopeId owner = name_owner(n, s);
    if (calls && mode == Lookup::Ordinary && operator_token(n) == OP_ASS) {
        ScopeId cls = naming_class(owner);
        if (cls) ensure_transfers(entities[scopes[cls].entity].type, true);
    }
    EntityId result = lookup(owner, terminal(n), child(ast.last(n),Kind::TemplateArguments) ? Lookup::Template : mode, ast.first(n) != ast.last(n) || ast.op(n) == OP_COLON2);
    if (definitions) result = class_template_name(ast.last(n),result,s);
    if (definitions) result = variable_template_name(ast.last(n),result,s);
    if (calls && result && !function_binding(result)) check_access(result, s, owner);
    return result;
}
} }
