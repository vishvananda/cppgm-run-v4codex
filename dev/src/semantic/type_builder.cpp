#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::source_type(EntityId e) const
{
    return entities[e].kind == EntityKind::Alias && entities[e].source ? facts[entities[e].source].type : entities[e].type;
}
EntityId Analyzer::declare_alias(ScopeId s, IdentifierId name, NodeId source, TypeId type)
{
    TypeId canonical = types.signature(type);
    EntityId e = local(s, name);
    if (e) {
        if ((entities[e].kind != EntityKind::Alias && entities[e].kind != EntityKind::Type) ||
            entities[e].type != canonical) throw std::runtime_error("conflicting type alias");
        return e;
    }
    e = make_entity(EntityKind::Alias, s, name, source);
    entities[e].type = canonical;
    bind(s, name, e);
    return e;
}
TypeId Analyzer::specifiers(NodeId n, ScopeId s, IdentifierId anonymous_name)
{
    TypeId result = 0;
    unsigned cv = 0, longs = 0;
    bool unsign = false, sign = false, short_int = false;
    EFundamentalType fundamental = FT_INT;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        const syntax::Node node = ast[c];
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward) {
            result = class_type(c, s, anonymous_name, node.kind == Kind::Class, spec_has(n, KW_STATIC));
            continue;
        }
        if (node.kind == Kind::Enum) { result = enum_type(c, s, anonymous_name, node.flags & 1); continue; }
        if (node.op == KW_DECLTYPE) { result = expression_type(node.first, s, true); continue; }
        if (node.detail) {
            EntityId e = resolve(node.detail, s);
            if (!e || (entities[e].kind != EntityKind::Type && entities[e].kind != EntityKind::Alias))
                throw std::runtime_error("type name is not a visible type");
            result = source_type(e);
            facts[c].entity = e;
            continue;
        }
        switch (node.op) {
        case KW_CONST: cv |= 1; break;
        case KW_VOLATILE: cv |= 2; break;
        case KW_LONG: ++longs; break;
        case KW_SHORT: short_int = true; break;
        case KW_UNSIGNED: unsign = true; break;
        case KW_SIGNED: sign = true; break;
        case KW_CHAR: fundamental = FT_CHAR; break;
        case KW_WCHAR_T: fundamental = FT_WCHAR_T; break;
        case KW_CHAR16_T: fundamental = FT_CHAR16_T; break;
        case KW_CHAR32_T: fundamental = FT_CHAR32_T; break;
        case KW_BOOL: fundamental = FT_BOOL; break;
        case KW_FLOAT: fundamental = FT_FLOAT; break;
        case KW_DOUBLE: fundamental = FT_DOUBLE; break;
        case KW_VOID: fundamental = FT_VOID; break;
        default: break;
        }
    }
    if (!result) {
        if (fundamental == FT_INT) {
            if (short_int) fundamental = unsign ? FT_UNSIGNED_SHORT_INT : FT_SHORT_INT;
            else if (longs > 1) fundamental = unsign ? FT_UNSIGNED_LONG_LONG_INT : FT_LONG_LONG_INT;
            else if (longs) fundamental = unsign ? FT_UNSIGNED_LONG_INT : FT_LONG_INT;
            else if (unsign) fundamental = FT_UNSIGNED_INT;
        } else if (fundamental == FT_CHAR) {
            if (unsign) fundamental = FT_UNSIGNED_CHAR;
            else if (sign) fundamental = FT_SIGNED_CHAR;
        } else if (fundamental == FT_DOUBLE && longs) fundamental = FT_LONG_DOUBLE;
        result = types.fundamental(fundamental);
    }
    result = types.qualify(result, cv);
    facts[n].type = result; facts[n].scope = s;
    return result;
}
TypeId Analyzer::type_id(NodeId n, ScopeId s)
{
    if (facts[n].type) return facts[n].type;
    NodeId specs = ast[n].first;
    TypeId t = declarator(ast[specs].next, specifiers(specs, s), s);
    facts[n].type = t; facts[n].scope = s;
    return t;
}
TypeId Analyzer::parameter(NodeId n, ScopeId s)
{
    if (facts[n].type) return facts[n].type;
    NodeId specs = ast[n].first;
    NodeId d = ast[specs].next;
    TypeId t = declarator(d, specifiers(specs, s), s);
    facts[n].type = t; facts[n].scope = s;
    return t;
}
TypeId Analyzer::declarator(NodeId n, TypeId base, ScopeId s)
{
    if (!n) return base;
    NodeId name = decl_name(n);
    if (name) s = name_owner(name, s);
    NodeId nested = 0;
    std::vector<NodeId> suffixes;
    bool after_direct = false;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        switch (ast[c].kind) {
        case Kind::Pointer:
            if (ast[c].detail) {
                EntityId owner = resolve(ast[c].detail, s, Lookup::Qualifier);
                if (!owner || !entities[owner].class_info) throw std::runtime_error("invalid member pointer owner");
                base = types.member_pointer(owner, base);
                break;
            }
            base = types.compound(ast[c].op == OP_AMP ? TypeKind::LRef :
                ast[c].op == OP_LAND ? TypeKind::RRef : TypeKind::Pointer, base);
            break;
        case Kind::CvQualifier:
            if (!after_direct) base = types.qualify(base, ast[c].op == KW_CONST ? 1 : 2);
            break;
        case Kind::NestedDeclarator: nested = ast[c].first; after_direct = true; break;
        case Kind::Identifier: after_direct = true; break;
        case Kind::Array: case Kind::Parameters: suffixes.push_back(c); after_direct = true; break;
        default: break;
        }
    }
    for (std::size_t i = suffixes.size(); i; --i) {
        NodeId c = suffixes[i - 1];
        if (ast[c].kind == Kind::Array) {
            std::uint64_t bound = 0;
            if (ast[c].first) {
                Constant v = evaluate(ast[c].first, s);
                if (!v.valid || !integral(v.type) || scoped_enum(v.type) || !v.bits ||
                    (!is_unsigned(v.type) && static_cast<std::int64_t>(v.bits) < 0))
                    throw std::runtime_error("array bound must be a positive integral constant");
                bound = v.bits;
            }
            base = types.compound(TypeKind::Array, base, bound);
        } else {
            std::vector<TypeId> params;
            bool variadic = false;
            for (NodeId p = ast[c].first; p; p = ast[p].next) {
                if (ast[p].kind == Kind::ParameterPack) { variadic = true; continue; }
                params.push_back(parameter(p, s));
                NodeId d = ast[ast[p].first].next;
                if (child(d, Kind::ParameterPack)) variadic = true;
            }
            if (params.size() == 1 && types[params[0]].kind == TypeKind::Fundamental &&
                types[params[0]].fundamental == FT_VOID && !variadic) params.clear();
            NodeId trailing = child(n, Kind::TrailingReturn);
            if (trailing) base = type_id(ast[trailing].first, s);
            unsigned member_cv = 0;
            for (NodeId q = ast[c].next; q; q = ast[q].next)
                if (ast[q].kind == Kind::CvQualifier) member_cv |= ast[q].op == KW_CONST ? 1 : 2;
            base = types.function(base, params, variadic, member_cv);
            facts[c].type = base;
        }
    }
    if (nested) base = declarator(nested, base, s);
    facts[n].type = base; facts[n].scope = s;
    return base;
}
EntityId Analyzer::declare_object(NodeId d, NodeId init, TypeId t, NodeId specs, ScopeId s, NodeId source)
{
    NodeId name = decl_name(d);
    IdentifierId id = terminal(name);
    ScopeId owner = name_owner(name, s);
    if (!encloses(s, owner)) throw std::runtime_error("qualified definition outside enclosing scope");
    bool alias = spec_has(specs, KW_TYPEDEF);
    bool function = types[t].kind == TypeKind::Function;
    if (!alias && !function && spec_has(specs, KW_CONSTEXPR)) t = types.qualify(t, 1);
    if (!alias && !function) {
        if (types[t].kind == TypeKind::Fundamental && types[t].fundamental == FT_VOID)
            throw std::runtime_error("object of void type");
        if ((types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) && !init &&
            scopes[s].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN))
            throw std::runtime_error("uninitialized reference");
    }
    EntityKind kind = alias ? EntityKind::Alias : function ? EntityKind::Function : EntityKind::Variable;
    if (alias) {
        EntityId e = declare_alias(owner, id, d, t);
        record(owner, e, d, t, kind);
        return e;
    }
    if (calls && types[t].kind == TypeKind::Array && !types[t].bound && init) {
        NodeId list = ast[init].first;
        if (ast[list].kind == Kind::BracedInit) {
            std::uint64_t count = 0;
            for (NodeId c = ast[list].first; c; c = ast[c].next) ++count;
            t = types.compound(TypeKind::Array, types[t].child, count);
        }
    }
    TypeId canonical = types.signature(t);
    bool constructor = (ast[source].kind == Kind::SpecialMember || ast[source].kind == Kind::SpecialDefinition) &&
        scopes[owner].kind == ScopeKind::Class && scopes[owner].name == id;
    EntityId cls = constructor ? scopes[owner].entity : 0;
    EntityId e = constructor ? class_facts[entities[cls].class_info].constructor : local(owner, id);
    if (function && !constructor) e = declare_function(owner, id, source, canonical);
    else if (e && entities[e].kind == kind) {
        if (calls && kind == EntityKind::Variable && (scopes[owner].kind == ScopeKind::Block || scopes[owner].kind == ScopeKind::Control) &&
            !spec_has(specs, KW_EXTERN)) throw std::runtime_error("duplicate local variable");
        entities[e].type = types.composite(entities[e].type, canonical);
    } else {
        e = make_entity(kind, owner, id, source);
        entities[e].type = canonical;
        if (constructor) class_facts[entities[cls].class_info].constructor = e;
        else bind(owner, id, e);
    }
    entities[e].is_static |= spec_has(specs, KW_STATIC);
    entities[e].c_linkage |= c_linkage;
    entities[e].inline_function |= spec_has(specs, KW_INLINE) || spec_has(specs, KW_CONSTEXPR);
    entities[e].thread_local_storage |= spec_has(specs, KW_THREAD_LOCAL);
    entities[e].external_decl |= spec_has(specs, KW_EXTERN);
    if (!function && !spec_has(specs, KW_EXTERN)) entities[e].definition = source;
    if (init && !function) { entities[e].initializer = init; entities[e].definition = source; }
    if (calls && function) function_defaults(e, d, owner);
    if (calls && function && scopes[owner].kind == ScopeKind::Class) member_facts(e);
    record(owner, e, d, t, kind);
    if (calls && init && !function) initialize(init, canonical, owner);
    if (calls && !init && !function && scopes[owner].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN)) default_initialize(e);
    if (init && !alias && !function && integral(t)) {
        Constant v = evaluate(init, owner);
        if (calls && spec_has(specs, KW_CONSTEXPR) && !v.valid) throw std::runtime_error("nonconstant constexpr initializer");
        if (v.valid) {
            v = convert(v, t);
            if (types[t].cv == 1 || types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef)
                entities[e].constant = v;
        }
    }
    if (calls && !init && !function && integral(t) && spec_has(specs, KW_CONSTEXPR))
        throw std::runtime_error("constexpr object requires initializer");
    if (calls && init && spec_has(specs, KW_CONSTEXPR) && integral(t) && ast[ast[init].first].kind == Kind::Literal)
        facts[ast[init].first].type = t;
    return e;
}
} }
