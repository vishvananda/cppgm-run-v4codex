#include "semantic/analyzer.h"
#include <stdexcept>
#include <sstream>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::class_type(NodeId n, ScopeId s, IdentifierId anonymous_name, bool emit, bool static_union)
{
    if (facts[n].type) return facts[n].type;
    NodeId name = ast[n].detail;
    IdentifierId id = name ? terminal(name) : anonymous_name;
    ETokenType key_op = ast[ast[n].first].op;
    bool definition = ast[n].kind == Kind::Class;
    bool anonymous_union = !id && key_op == KW_UNION;
    if (anonymous_union && scopes[s].kind == ScopeKind::Namespace && !static_union)
        throw std::runtime_error("namespace anonymous union requires static");
    if (!id) {
        const syntax::ClassRegion& region = ast.class_regions[ast[n].literal];
        std::string generated = "__anonymous_union_type__" + std::to_string(region.begin) + "_" + std::to_string(region.end);
        id = ids.intern(TextView(generated.data(), generated.size()));
    }
    ScopeId owner = name_owner(name, s);
    EntityId e = !name ? 0 : emit ? local(owner, id, Lookup::Tag) : lookup(owner, id, Lookup::Tag, owner != s);
    if (!e) {
        e = make_entity(EntityKind::Type, owner, id, n);
        entities[e].class_info = class_facts.size();
        class_facts.push_back(ClassFacts());
        entities[e].key = key_op;
        entities[e].type = types.named(e);
        entities[e].scope = make_scope(ScopeKind::Class, owner, id, e, false);
        if (name) {
            bind(owner, id, e);
            bind(entities[e].scope, id, e); // Injected-class-name, without a second source declaration.
        }
        emit = !anonymous_union;
    } else if (entities[e].kind != EntityKind::Type || entities[e].key == KW_ENUM ||
               ((entities[e].key == KW_UNION) != (key_op == KW_UNION)))
        throw std::runtime_error("incompatible class declaration");
    TypeId t = entities[e].type;
    facts[n].type = t; facts[n].entity = e;
    if (emit && !anonymous_union) {
        std::uint32_t d = record(owner, e, n, t, EntityKind::Type);
        declarations[d].key = key_op;
    }
    if (definition) {
        if (entities[e].complete) throw std::runtime_error("class redefinition");
        std::size_t deferred_begin = bodies.size();
        ++class_depth;
        ScopeId cs = entities[e].scope;
        attach_scope(cs, owner);
        for (NodeId c = ast[n].first; c; c = ast[c].next) declaration(c, cs);
        entities[e].complete = true;
        entities[e].definition = n;
        if (!--class_depth) {
            // Only complete-class contexts defer bodies. Each outermost class
            // owns its queue interval; a local class can drain its own interval
            // without delaying lookup past declarations following that class.
            std::size_t end = bodies.size();
            for (std::size_t i = deferred_begin; i < end; ++i) {
                Body body = bodies[i];
                function_body(body);
            }
            bodies.resize(deferred_begin);
        }
        if (anonymous_union) {
            for (std::uint32_t d = scopes[cs].first_decl; d; d = declarations[d].next) {
                EntityId member = declarations[d].entity;
                bind(s, entities[member].name, member);
                record(s, member, 0, entities[member].type, entities[member].kind);
            }
        }
    }
    return t;
}
TypeId Analyzer::enum_type(NodeId n, ScopeId s, IdentifierId anonymous_name, bool emit)
{
    if (facts[n].type) return facts[n].type;
    NodeId name = ast[n].detail;
    IdentifierId id = name ? terminal(name) : ast[n].text ? ast[n].text : anonymous_name;
    bool scoped = child(n, Kind::EnumKey);
    bool definition = ast[n].flags & 1;
    NodeId underlying_node = child(n, Kind::TypeId);
    ScopeId owner = name_owner(name, s);
    bool declares = emit || definition || scoped || underlying_node;
    EntityId e = declares ? local(owner, id, Lookup::Tag) : lookup(owner, id, Lookup::Tag, owner != s);
    if (!definition && !scoped && !underlying_node && !e)
        throw std::runtime_error("undeclared elaborated or opaque unscoped enum");
    TypeId underlying = underlying_node ? type_id(underlying_node, s) : types.fundamental(FT_INT);
    if (!e) {
        e = make_entity(EntityKind::Type, owner, id, n);
        entities[e].key = KW_ENUM; entities[e].scoped = scoped;
        entities[e].type = types.named(e); entities[e].underlying = underlying;
        entities[e].scope = make_scope(ScopeKind::Enum, owner, id, e, scoped);
        bind(owner, id, e); emit = true;
    } else if (entities[e].key != KW_ENUM || (underlying_node && entities[e].underlying != underlying))
        throw std::runtime_error("incompatible enum declaration");
    TypeId t = entities[e].type;
    facts[n].type = t; facts[n].entity = e;
    ScopeId es = entities[e].scope;
    ScopeId output_owner = owner;
    bool qualified_definition = definition && name && ast[name].first != ast[name].last;
    if (qualified_definition) {
        // Source-faithful qualified enum definitions have their own scope view;
        // both declarations still identify the same canonical enum entity.
        es = make_scope(ScopeKind::Enum, s, id, e);
        scopes[es].display_name = name;
        output_owner = s;
    }
    if (emit || definition || scoped || underlying_node) {
        std::uint32_t d = record(output_owner, e, n, t, EntityKind::Type);
        if (qualified_definition) declarations[d].display_name = name;
    }
    if (definition) {
        if (entities[e].complete) throw std::runtime_error("enum redefinition");
        std::uint64_t next = 0;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (ast[c].kind != Kind::Enumerator) continue;
            Constant value = ast[c].first ? evaluate(ast[c].first, entities[e].scope) : Constant(underlying, next);
            if (!value.valid || !integral(value.type)) throw std::runtime_error("invalid enumerator initializer");
            value = convert(value, underlying, true);
            EntityId v = make_entity(EntityKind::Enumerator, entities[e].scope, ast[c].text, c);
            entities[v].type = t; entities[v].constant = Constant(underlying, value.bits);
            bind(entities[e].scope, ast[c].text, v);
            if (!scoped) bind(owner, ast[c].text, v);
            std::uint32_t d = record(scoped ? es : owner, v, c, t, EntityKind::Enumerator);
            if (qualified_definition) declarations[d].display_name = name;
            next = value.bits + 1;
        }
        for (NodeId c = ast[n].first; c; c = ast[c].next)
            if (ast[c].kind == Kind::Enumerator) entities[facts[c].entity].constant.type = t;
        entities[e].complete = true;
        entities[e].definition = n;
    }
    return t;
}
} }
