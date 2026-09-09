#include "semantic/analyzer.h"
#include <stdexcept>
#include <chrono>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::consume(NodeId n)
{
    typedef std::chrono::steady_clock Clock;
    Clock::time_point start;
    if (ast.telemetry) start = Clock::now();
    facts.resize(ast.nodes.size());
    if (calls) expressions.resize(ast.nodes.size());
    declaration(n, global);
    if (ast.telemetry) analysis_ms += std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}
void Analyzer::finish()
{
    while (demand_cursor < demand_queue.size()) {
        EntityId e = demand_queue[demand_cursor++];
        std::uint32_t m = entities[e].member_info;
        MemberFacts f = members[m];
        members[m].demand = DemandState::Active;
        if (f.body && !entities[e].definition)
            function_body({f.body, f.declarator, entities[e].owner, e, f.source});
        if (members[m].synthetic && transfer_member(e)) {
            prepare_transfer(e);
            if (members[m].deleted) throw std::runtime_error("deleted defaulted special member");
            if (members[m].constructor) {
                bool direct = members[m].transfer_trivial && copy_storage_type(entities[scopes[entities[e].owner].entity].type);
                members[m].transfer_direct = direct;
            }
            for (unsigned j = 0; j < members[m].transfer_count; ++j) {
                auto action = transfers[members[m].transfer_begin+j];
                if (!action.function) continue;
                if (constructor_member(action.function)) {
                    if (action.field) members[entities[action.function].member_info].complete_entry = true;
                    else members[entities[action.function].member_info].base_entry = true;
                }
                demand_member(action.function);
            }
        } else if (members[m].constructor) constructor_actions(e);
        if (members[m].destructor) destructor_actions(e);
        members[m].demand = DemandState::Complete;
    }
    // The delegation graph has at most one edge per constructor. Each demanded
    // vertex is colored once; converging chains do not repeat completed work.
    Index delegation_states;
    std::vector<EntityId> chain;
    for (EntityId root : demand_queue) {
        EntityId e = root;
        while (e && !delegation_states.get(e)) {
            delegation_states.put(e, 1); chain.push_back(e);
            e = members[entities[e].member_info].delegated_constructor;
        }
        if (e && delegation_states.get(e) == 1) throw std::runtime_error("cyclic constructor delegation");
        for (EntityId seen : chain) delegation_states.put(seen, 2);
        chain.clear();
    }
    if (calls) { finish_allocations(); prepare_function_boundaries(); }
    for (NodeId body : jump_bodies) check_jumps(body);
}
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
    facts[n].entity = e; facts[n].scope = ns;
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
    ScopeId saved_access = access_override;
    NodeId named = ast[n].kind == Kind::Function ? ast[specs].next : ast[ast[list].first].first;
    ScopeId owner = name_owner(decl_name(named), s, true);
    if (calls && scopes[owner].kind == ScopeKind::Class) access_override = owner;
    TypeId base = specifiers(specs, s, anonymous_name);
    if (calls) for (NodeId c = ast[specs].first; c; c = ast[c].next)
        if (auto object = anonymous_object(c)) anonymous_objects.put(n, object);
    if (ast[n].kind == Kind::Function) {
        NodeId d = ast[specs].next;
        TypeId t = declarator(d, base, s);
        EntityId e = declare_object(d, 0, t, specs, s, n);
        schedule_body({ast[d].next, d, entities[e].owner, e, n});
        access_override = saved_access; return;
    }
    for (NodeId item = ast[list].first; item; item = ast[item].next) {
        NodeId d = ast[item].first;
        TypeId t = declarator(d, base, s);
        EntityId e = declare_object(d, ast[d].next, t, specs, s, n);
        if (calls && scopes[s].kind == ScopeKind::Class && (ast.alignment_owners.get(n) || ast.alignment_owners.get(specs))) {
            auto alignment = std::max(alignment_attributes(n, s), alignment_attributes(specs, s));
            if (alignment && alignment < size(t, true)) throw std::runtime_error("weakened field alignment");
            field_metadata(e).alignment = alignment;
        }
    }
    access_override = saved_access;
}
void Analyzer::declaration(NodeId n, ScopeId s)
{
    ++analyzed;
    if (calls && scopes[s].kind == ScopeKind::Class && friend_declaration(n, s)) return;
    switch (ast[n].kind) {
    case Kind::Access:
        if (calls && scopes[s].kind == ScopeKind::Class)
            class_facts[entities[scopes[s].entity].class_info].current_access = ast[n].op == KW_PRIVATE ? Access::Private : ast[n].op == KW_PROTECTED ? Access::Protected : Access::Public;
        break;
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
        if (calls && scopes[s].kind == ScopeKind::Class && inherit_using(name, s)) break;
        for (NodeId p = ast[name].first; p; p = ast[p].next)
            if (ast[p].first) throw std::runtime_error("using declaration names template-id");
        EntityId e = resolve(name, s);
        if (!e) throw std::runtime_error("unknown using target");
        if (calls && scopes[s].kind == ScopeKind::Class) {
            for (EntityId member : candidates(e)) {
                check_access(member, s, name_owner(name, s));
                using_access.put(key(s, member), unsigned(declaration_access(s)) + 1);
            }
        }
        if (calls && function_binding(e)) using_functions.put(key(s, terminal(name)), 1);
        bind(s, terminal(name), e);
        record(s, e, n, source_type(e), entities[e].kind);
        break;
    }
    case Kind::Alias: {
        TypeId t = type_id(ast[n].first, s);
        EntityId e = declare_alias(s, ast[n].text, n, t);
        record(s, e, n, t, EntityKind::Alias);
        break;
    }
    case Kind::SimpleDeclaration: case Kind::Function: simple(n, s); break;
    case Kind::Class: case Kind::ClassForward: class_type(n, s); break;
    case Kind::Enum: enum_type(n, s); break;
    case Kind::BitField: if (calls) bit_field_declaration(n, s); break;
    case Kind::Template: template_declaration(n, s); break;
    case Kind::StaticAssert: {
        Constant v = evaluate(ast[n].first, s);
        if (!v.valid || scoped_enum(v.type) || !v.bits) throw std::runtime_error("static assertion is not a true integral constant");
        break;
    }
    case Kind::SpecialMember: case Kind::SpecialDefinition: {
        NodeId d = child(n, Kind::Declarator);
        NodeId name = decl_name(d), part = ast[name].last;
        TypeId result = calls && ast[part].op == KW_OPERATOR && ast[part].detail ?
            type_id(ast[part].detail,name_owner(name,s,true)) : types.fundamental(FT_VOID);
        TypeId t = declarator(d, result, s);
        EntityId e = declare_object(d, 0, t, 0, s, n);
        if (calls && child(child(n, Kind::Initializer), Kind::SpecialInitializer)) {
            facts[n].entity = e; facts[n].type = entities[e].type;
        }
        if (ast[n].kind == Kind::SpecialDefinition) {
            NodeId b = child(n, Kind::Compound);
            if (!b) b = child(n, Kind::FunctionTry);
            schedule_body({b, d, entities[e].owner, e, n});
        }
        break;
    }
    case Kind::Linkage: {
        bool saved = c_linkage;
        const auto literal = ast.literals[ast[n].literal];
        c_linkage = literal.type == FT_CHAR && literal.bytes == 2 && ast.literal_bytes[literal.offset] == 'C';
        for (NodeId c = ast[n].first; c; c = ast[c].next) declaration(c, s);
        c_linkage = saved; break;
    }
    case Kind::ExplicitInstantiation:
        for (NodeId c = ast[n].first; c; c = ast[c].next) declaration(c, s);
        break;
    default: break;
    }
}
void Analyzer::schedule_body(const Body& body)
{
    if (calls && entities[body.entity].template_info) {
        TemplateFunction& t = templates[entities[body.entity].template_info];
        t.body = body.node; t.declarator = body.declarator; t.source = body.source;
        return;
    }
    if (calls && entities[body.entity].member_info) {
        std::uint32_t m = entities[body.entity].member_info;
        if (members[m].source) throw std::runtime_error("duplicate member definition");
        members[m].body = body.node; members[m].declarator = body.declarator; members[m].source = body.source;
        if (class_depth) { entities[body.entity].inline_function = true; return; }
        if (!entities[body.entity].inline_function && (members[m].constructor || members[m].destructor))
            members[m].base_entry = true;
    }
    if (class_depth) bodies.push_back(body);
    else function_body(body);
}
void Analyzer::function_body(const Body& body)
{
    if (entities[body.entity].definition) throw std::runtime_error("function redefinition");
    ScopeId fs = make_scope(ScopeKind::Function, body.owner, entities[body.entity].name, body.entity);
    entities[body.entity].definition = body.source;
    entities[body.entity].body = body.node;
    entities[body.entity].scope = fs;
    facts[body.source].entity = body.entity;
    facts[body.source].type = entities[body.entity].type;
    facts[body.source].scope = fs;
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
        entities[e].type = types[t].kind == TypeKind::Array ? types.compound(TypeKind::Pointer, types.signature(types[t].child)) :
            types[t].kind == TypeKind::Function ? types.compound(TypeKind::Pointer, types.signature(t)) : types.signature(t);
        bind(fs, name, e); record(fs, e, p, t, EntityKind::Parameter);
        if (calls && class_value(entities[e].type)) register_destruction(e);
    }
    TypeId saved_return = return_type;
    EntityId saved_function = current_function; current_function = body.entity;
    return_type = types[entities[body.entity].type].child;
    if (calls && constructor_member(body.entity)) constructor_actions(body.entity);
    statements(body.node, fs);
    if (calls) finish_class_returns(body.entity);
    if (calls) jump_bodies.push_back(body.node);
    return_type = saved_return;
    current_function = saved_function;
}
void Analyzer::statements(NodeId n, ScopeId s)
{
    if (!n) return;
    if (calls) { resolve_statement(n, s); return; }
    switch (ast[n].kind) {
    case Kind::If: case Kind::Switch: case Kind::For: case Kind::RangeFor:
    case Kind::While: case Kind::Do: {
        ScopeId control = make_scope(ScopeKind::Block, s);
        facts[n].scope = control;
        NodeId body = ast[n].kind == Kind::Do ? ast[n].first : ast[n].last;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            bool unbraced = ast[n].kind != Kind::If && c == body && ast[c].kind != Kind::Compound;
            statements(c, unbraced ? make_scope(ScopeKind::Block, control) : control);
        }
        break;
    }
    case Kind::Then: case Kind::Else: {
        NodeId body = ast[n].first;
        statements(body, ast[body].kind == Kind::Compound ? s : make_scope(ScopeKind::Block, s));
        break;
    }
    case Kind::ConditionDeclaration: {
        NodeId specs = ast[n].first, d = ast[specs].next;
        TypeId t = declarator(d, specifiers(specs, s), s);
        declare_object(d, ast[d].next, t, specs, s, n);
        break;
    }
    case Kind::Compound: {
        ScopeId bs = make_scope(ScopeKind::Block, s);
        facts[n].scope = bs;
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
                if (types[t].kind == TypeKind::Named && entities[types[t].entity].class_info) {
                    EntityId cls = types[t].entity;
                    ScopeId cs = entities[cls].scope;
                    EntityId ctor = class_facts[entities[cls].class_info].constructor;
                    if (!ctor && !class_facts[entities[cls].class_info].default_constructor)
                        class_facts[entities[cls].class_info].default_constructor = make_scope(ScopeKind::Function, cs, entities[cls].name);
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
