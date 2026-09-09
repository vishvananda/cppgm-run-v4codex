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
    if (name) s = name_owner(name, s, true);
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
            RefQualifier ref = RefQualifier::None;
            for (NodeId q = ast[c].next; q; q = ast[q].next) {
                if (ast[q].kind == Kind::CvQualifier) member_cv |= ast[q].op == KW_CONST ? 1 : 2;
                if (ast[q].kind == Kind::FunctionQualifier && (ast[q].op == OP_AMP || ast[q].op == OP_LAND)) {
                    if (ref != RefQualifier::None) throw std::runtime_error("duplicate ref qualifier");
                    ref = ast[q].op == OP_AMP ? RefQualifier::Lvalue : RefQualifier::Rvalue;
                }
            }
            base = types.function(base, params, variadic, member_cv, ref);
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
    bool destructor = ast[ast[name].last].op == OP_COMPL;
    ScopeId owner = name_owner(name, s, true);
    if (!encloses(s, owner)) throw std::runtime_error("qualified definition outside enclosing scope");
    if (destructor && scopes[owner].kind == ScopeKind::Class) {
        TextView text = ids.spelling(scopes[owner].name);
        std::string label = "~" + std::string(text.data, text.size);
        id = ids.intern(TextView(label.data(), label.size()));
    }
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
        if (ast[list].kind == Kind::Literal && ast.literals[ast[list].literal].kind == LiteralKind::string)
            t = types.compound(TypeKind::Array, types[t].child, ast.literals[ast[list].literal].elements);
        else if (ast[list].kind == Kind::BracedInit) {
            std::uint64_t count = 0;
            for (NodeId c = ast[list].first; c; c = ast[c].next) ++count;
            t = types.compound(TypeKind::Array, types[t].child, count);
        }
    }
    TypeId canonical = types.signature(t);
    bool constructor = (ast[source].kind == Kind::SpecialMember || ast[source].kind == Kind::SpecialDefinition) &&
        scopes[owner].kind == ScopeKind::Class && scopes[owner].name == id && ast[ast[name].last].op != OP_COMPL;
    EntityId cls = constructor ? scopes[owner].entity : 0;
    if (calls && function && types[t].ref != RefQualifier::None &&
        (scopes[owner].kind != ScopeKind::Class || spec_has(specs, KW_STATIC) || constructor || destructor))
        throw std::runtime_error("ref qualifier requires ordinary nonstatic member");
    EntityId e = constructor ? class_facts[entities[cls].class_info].constructor : local(owner, id);
    if (constructor) {
        EntityId selected = declare_function(owner, id, source, canonical, true);
        class_facts[entities[cls].class_info].constructor = merge_lookup(e, selected);
        e = selected;
    }
    else if (function) e = declare_function(owner, id, source, canonical);
    else if (e && entities[e].kind == kind) {
        if (calls && scopes[owner].kind == ScopeKind::Class && owner == s)
            throw std::runtime_error("duplicate class member");
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
    if (calls && function && entities[e].is_static && types[t].ref != RefQualifier::None)
        throw std::runtime_error("static member cannot be ref qualified");
    entities[e].mutable_field |= spec_has(specs, KW_MUTABLE);
    if (calls && function) declare_operator(e, name);
    entities[e].c_linkage |= c_linkage;
    entities[e].no_inline |= ast[source].flags & 64;
    entities[e].force_inline |= ast[source].flags & 128;
    entities[e].inline_function |= spec_has(specs, KW_INLINE) || spec_has(specs, KW_CONSTEXPR);
    entities[e].inline_function |= spec_has(child(source, Kind::MemberSpecifiers), KW_INLINE);
    entities[e].thread_local_storage |= spec_has(specs, KW_THREAD_LOCAL);
    entities[e].external_decl |= spec_has(specs, KW_EXTERN);
    if (!function && !spec_has(specs, KW_EXTERN) && !(scopes[s].kind == ScopeKind::Class && entities[e].is_static)) entities[e].definition = source;
    if (init && !function) { entities[e].initializer = init; if (!(scopes[s].kind == ScopeKind::Class && entities[e].is_static)) entities[e].definition = source; }
    if (calls && function) { function_defaults(e, d, owner); exception_specification(e, d, owner); }
    if (calls && function && scopes[owner].kind == ScopeKind::Class) {
        member_facts(e);
        auto m = entities[e].member_info;
        members[m].constructor = constructor;
        members[m].destructor = destructor;
        if (destructor) class_facts[entities[scopes[owner].entity].class_info].destructor = e;
        members[m].explicit_constructor = spec_has(child(source, Kind::MemberSpecifiers), KW_EXPLICIT);
        NodeId special = child(child(source, Kind::Initializer), Kind::SpecialInitializer);
        members[m].deleted = special && ast[special].op == KW_DELETE;
        if (constructor && !special) class_facts[entities[cls].class_info].aggregate = false;
    }
    record(owner, e, d, t, kind);
    bool member_initializer = calls && init && !function && scopes[s].kind == ScopeKind::Class && !entities[e].is_static;
    if (calls && init && !function && scopes[s].kind == ScopeKind::Class && entities[e].is_static &&
        !spec_has(specs, KW_CONSTEXPR) && (!(types[t].cv & 1) || !integral(t)))
        throw std::runtime_error("in-class static initializer requires const integral or constexpr member");
    if (calls && !function && !entities[e].is_static && scopes[owner].kind == ScopeKind::Class && entities[e].access != Access::Public)
        class_facts[entities[scopes[owner].entity].class_info].aggregate = false;
    if (member_initializer) {
        auto cls = scopes[s].entity;
        auto info = entities[cls].class_info;
        class_facts[info].aggregate = false;
        if (entities[cls].key == KW_UNION) {
            if (class_facts[info].variant_initializer) throw std::runtime_error("multiple default union variant initializers");
            class_facts[info].variant_initializer = e;
        }
    }
    if (calls && init && !function && !member_initializer) initialize(init, canonical, owner);
    if (calls && !init && !function && scopes[s].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN)) default_initialize(e);
    if (init && !alias && !function && integral(t) && !member_initializer) {
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
    if (calls && !function && !alias && !member_initializer && scopes[s].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN)) register_destruction(e);
    return e;
}
} }
