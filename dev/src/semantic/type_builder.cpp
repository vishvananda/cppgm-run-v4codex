#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::parameter_body_type(TypeId source)
{
    return types[source].kind == TypeKind::Array || types[source].kind == TypeKind::DependentArray ? types.compound(TypeKind::Pointer,types.signature(types[source].child)) :
        types[source].kind == TypeKind::Function ? types.compound(TypeKind::Pointer,types.signature(source)) : types.signature(source);
}
TypeId Analyzer::source_type(EntityId e) const
{
    return entities[e].kind == EntityKind::Alias && entities[e].source ? facts[entities[e].source].type : entities[e].type;
}
EntityId Analyzer::declare_alias(ScopeId s, IdentifierId name, NodeId source, TypeId type)
{
    auto environment = s;
    if (definitions && active_template_scope == s) s = scopes[s].parent;
    TypeId canonical = types.signature(type);
    if (scopes[s].kind == ScopeKind::Class) {
        auto previous = class_typedef_declarations.get(key(s,name));
        if (previous && previous != source) throw std::runtime_error("class typedef-name redeclared");
        class_typedef_declarations.put(key(s,name),source);
    }
    EntityId e = local(s, name);
    if (e) {
        if (definitions && (environment == active_template_scope) != (entities[e].template_info != 0))
            throw std::runtime_error("alias template conflicts with nontemplate declaration");
        if (definitions && environment == active_template_scope && entities[e].template_info) {
            if (!equivalent_alias_template(e,canonical,environment)) throw std::runtime_error("conflicting alias template");
            return e;
        }
        if ((entities[e].kind != EntityKind::Alias && entities[e].kind != EntityKind::Type) ||
            entities[e].type != canonical) throw std::runtime_error("conflicting type alias");
        return e;
    }
    e = make_entity(EntityKind::Alias, s, name, source);
    entities[e].type = canonical;
    bind(s, name, e);
    if (s != environment) bind(environment,name,e);
    return e;
}
TypeId Analyzer::specifiers(NodeId n, ScopeId s, IdentifierId anonymous_name)
{
    if (definitions) if (auto type = reuse_template_type(n,s)) return type;
    TypeId result = 0;
    unsigned cv = 0, longs = 0;
    bool unsign = false, sign = false, short_int = false;
    EFundamentalType fundamental = FT_INT;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        const syntax::Node node = ast[c];
        if (template_type_probe && (node.kind == Kind::Class || node.kind == Kind::ClassForward || node.kind == Kind::Enum)) return 0;
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward) {
            result = class_type(c, s, anonymous_name, node.kind == Kind::Class, spec_has(n, KW_STATIC));
            continue;
        }
        if (node.kind == Kind::Enum) { result = enum_type(c, s, anonymous_name, node.flags & 1); continue; }
        if (node.op == KW_DECLTYPE) {
            result = expression_type(node.first, s, true);
            if (template_type_probe && !result) return 0;
            continue;
        }
        if (node.detail) {
            if (definitions) {
                result = type_name(node.detail,s,0,!(node.flags & 1)); facts.edit(c).entity = facts[node.detail].entity;
                if (template_type_probe && !result) return 0;
                continue;
            }
            EntityId e = resolve(node.detail, s);
            if (!e || (entities[e].kind != EntityKind::Type && entities[e].kind != EntityKind::Alias))
                throw std::runtime_error("type name is not a visible type");
            result = source_type(e);
            facts.edit(c).entity = e;
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
    { auto& published = facts.edit(n); published.type = result; published.scope = s; }
    return result;
}
bool Analyzer::prototype_scope_needed(NodeId parameters)
{
    auto source = ast.nodes.occurrences[parameters].source;
    auto slot = source/4, shift = (source%4)*2;
    if (slot >= prototype_scope_requirements.size())
        prototype_scope_requirements.resize((ast.nodes.parsed_size()+3)/4);
    auto known = (prototype_scope_requirements[slot] >> shift) & 3;
    if (known) return known == 2;
    // A source-only predicate: parameter type expressions may refer to earlier
    // parameters. Reuse it across every concrete declaration of this syntax.
    bool needed = false;
    // This syntax walk cannot reenter semantic analysis. Retain one scratch
    // buffer up to the largest parameter list, without per-declaration churn.
    auto& work = prototype_scope_work; work.clear();
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        auto specs = ast[p].first; work.push_back(specs); work.push_back(ast[specs].next);
    }
    for (std::size_t j = 0; j < work.size() && !needed; ++j) {
        auto node = ast[work[j]];
        if (node.kind == Kind::IdExpression || node.op == KW_DECLTYPE) { needed = true; break; }
        if (node.detail) work.push_back(node.detail);
        for (auto child = node.first; child; child = ast[child].next) work.push_back(child);
    }
    prototype_scope_requirements[slot] |= (needed ? 2 : 1) << shift;
    return needed;
}
FunctionQualifiers Analyzer::function_qualifiers(NodeId parameters)
{
    FunctionQualifiers result;
    for (auto q = ast[parameters].next; q; q = ast[q].next) {
        if (ast[q].kind == Kind::CvQualifier) result.cv |= ast[q].op == KW_CONST ? 1 : 2;
        if (ast[q].kind == Kind::FunctionQualifier && (ast[q].op == OP_AMP || ast[q].op == OP_LAND)) {
            if (result.ref != RefQualifier::None) throw std::runtime_error("duplicate ref qualifier");
            result.ref = ast[q].op == OP_AMP ? RefQualifier::Lvalue : RefQualifier::Rvalue;
        }
    }
    return result;
}
TypeId Analyzer::type_id(NodeId n, ScopeId s)
{
    if (facts[n].type) return facts[n].type;
    if (definitions) if (auto type = reuse_template_type(n,s)) return type;
    NodeId specs = ast[n].first;
    TypeId t = declarator(ast[specs].next, specifiers(specs, s), s);
    { auto& published = facts.edit(n); published.type = t; published.scope = s; }
    return t;
}
TypeId Analyzer::parameter(NodeId n, ScopeId s)
{
    if (facts[n].type) return facts[n].type;
    NodeId specs = ast[n].first;
    if (calls && spec_has(specs,KW_CONSTEXPR)) throw std::runtime_error("constexpr parameter declaration");
    NodeId d = ast[specs].next;
    TypeId t = declarator(d, specifiers(specs, s), s);
    { auto& published = facts.edit(n); published.type = t; published.scope = s; }
    return t;
}
TypeId Analyzer::declarator(NodeId n, TypeId base, ScopeId s, NodeId dynamic_array, bool name_resolved, NodeId specs)
{
    if (template_type_probe && !base) return 0;
    if (!n) return base;
    if (definitions && !dynamic_array && !deducing_placeholder) if (auto type = reuse_template_type(n,s)) return type;
    NodeId name = decl_name(n);
    if (name && !name_resolved) {
        auto owner = name_owner(name,s,true);
        if (definitions && s == active_template_scope && scopes[owner].kind == ScopeKind::Class)
            s = member_template_environment(s,owner);
        else if (!(definitions && scopes[s].kind == ScopeKind::Template &&
            (scopes[owner].kind == ScopeKind::Namespace || scopes[s].parent == owner))) s = owner;
    }
    NodeId nested = 0;
    std::vector<NodeId> suffixes;
    bool after_direct = false;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        switch (ast[c].kind) {
        case Kind::Pointer:
            if (ast[c].detail) {
                EntityId owner = resolve(ast[c].detail, s, Lookup::Qualifier);
                if (template_type_probe && owner && (entities[owner].template_pattern ||
                    entities[owner].template_parameter || dependent_type(entities[owner].type))) return 0;
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
                if (definitions && !ast.nodes.occurrences[c].context && (template_type_probe || active_template_scope) &&
                    bind_template_expression(ast[c].first,s)) {
                    auto query = expression_query(ast[c].first,s);
                    if (!query && template_type_probe) return 0;
                    query_fact(query);
                    base = types.compound(TypeKind::DependentArray,base,query);
                    continue;
                }
                Constant v = evaluate(ast[c].first, s);
                if (c == dynamic_array) {
                    auto x = expression(ast[c].first,s);
                    if (!integral(x.type) || scoped_enum(x.type)) throw std::runtime_error("array allocation bound must be integral");
                    if (v.valid && !is_unsigned(v.type) && static_cast<std::int64_t>(v.bits) < 0) throw std::runtime_error("negative array allocation bound");
                } else if (!v.valid || !integral(v.type) || scoped_enum(v.type) || !v.bits ||
                    (!is_unsigned(v.type) && static_cast<std::int64_t>(v.bits) < 0))
                    throw std::runtime_error("array bound must be a positive integral constant");
                bound = v.valid ? v.bits : 0;
            }
            base = types.compound(TypeKind::Array, base, bound);
        } else {
            std::vector<TypeId> params;
            bool variadic = false;
            NodeId trailing = child(n, Kind::TrailingReturn);
            ScopeId parameter_scope = s;
            if (definitions && (active_template_scope || scopes[s].kind == ScopeKind::Template || trailing || prototype_scope_needed(c)))
                parameter_scope = make_scope(ScopeKind::Block,s);
            for (NodeId p = ast[c].first; p; p = ast[p].next) {
                if (ast[p].kind == Kind::ParameterPack) { variadic = true; continue; }
                params.push_back(parameter(p, parameter_scope));
                if (template_type_probe && !params.back()) return 0;
                NodeId d = ast[ast[p].first].next;
                if (definitions && child(d,Kind::ParameterPack)) {
                    // An unnamed nondependent parameter followed by ... is
                    // the comma-optional C varargs form, not a pack expansion.
                    if (!decl_name(d) && !argument_packs[expansion_parameters(params.back())].count) variadic = true;
                    else params.back() = types.compound(TypeKind::PackExpansion,0,params.back());
                }
                else if (child(d,Kind::ParameterPack)) variadic = true;
                auto id = terminal(decl_name(d));
                if (parameter_scope != s && id) {
                    auto e = make_entity(EntityKind::Parameter,parameter_scope,id,p);
                    auto type = params.back();
                    if (types[type].kind == TypeKind::PackExpansion) {
                        entities[e].parameter_pack = true;
                        type = types[type].bound;
                    }
                    // The signature owns expansion; an occurrence of the
                    // parameter denotes one element within that expansion.
                    entities[e].type = parameter_body_type(type);
                    signature_parameters.put(e,params.size()); bind(parameter_scope,id,e);
                }
            }
            if (params.size() == 1 && types[params[0]].kind == TypeKind::Fundamental &&
                types[params[0]].fundamental == FT_VOID && !variadic) params.clear();
            if (trailing) {
                // The member declarator owns this/cv before a function entity
                // or body exists. Keep that identity on its prototype scope.
                auto owner = s;
                while (scopes[owner].kind == ScopeKind::Template) owner = scopes[owner].parent;
                if (scopes[owner].kind == ScopeKind::Class && name) {
                    TemplateObjectContext object; object.owner = scopes[owner].entity;
                    object.available = !spec_has(specs,KW_STATIC);
                    object.cv = function_qualifiers(c).cv;
                    if (spec_has(specs,KW_CONSTEXPR) && object.available) object.cv |= 1;
                    template_object_context_index.put(parameter_scope,template_object_contexts.size());
                    template_object_contexts.push_back(object);
                }
                base = type_id(ast[trailing].first, parameter_scope);
                if (template_type_probe && !base) return 0;
            }
            auto qualifiers = function_qualifiers(c);
            base = types.function(base, params, variadic, qualifiers.cv, qualifiers.ref);
            facts.edit(c).type = base;
        }
    }
    if (nested) base = declarator(nested, base, s,dynamic_array,name_resolved);
    { auto& published = facts.edit(n); published.type = base; published.scope = s; }
    return base;
}
void Analyzer::declaration_attributes(EntityId e, NodeId specs, NodeId source)
{
    entities[e].c_linkage |= c_linkage;
    entities[e].no_inline |= ast[source].flags & 64;
    entities[e].force_inline |= ast[source].flags & 128;
    if (calls && (ast[source].flags & 16)) {
        Type f = types[entities[e].type];
        if (f.kind != TypeKind::Function || f.variadic || !f.count || !(arithmetic(f.child) || integral(f.child) || pointer(f.child)) ||
            !integral(types.parameters[f.offset+f.count-1])) throw std::runtime_error("invalid stable-prefix query signature");
        entities[e].stable_prefix = true;
    }
    if (entities[e].kind == EntityKind::Function) {
        bool constant = spec_has(specs,KW_CONSTEXPR) ||
            spec_has(child(source,Kind::MemberSpecifiers),KW_CONSTEXPR);
        if (calls && !entities[e].specialization && !entities[e].template_member) {
            auto known = constexpr_declarations.get(e);
            if (known && known != (constant ? 2U : 1U)) throw std::runtime_error("inconsistent constexpr declaration");
            constexpr_declarations.put(e,constant ? 2 : 1);
        }
        entities[e].constexpr_function |= constant;
        entities[e].inline_function |= spec_has(specs, KW_INLINE) || spec_has(specs, KW_CONSTEXPR);
        entities[e].inline_function |= spec_has(child(source, Kind::MemberSpecifiers), KW_INLINE) || entities[e].constexpr_function;
    }
    if (entities[e].kind == EntityKind::Variable && spec_has(specs,KW_CONSTEXPR)) constexpr_declarations.put(e,2);
    entities[e].thread_local_storage |= spec_has(specs, KW_THREAD_LOCAL);
    entities[e].external_decl |= spec_has(specs, KW_EXTERN);
}
EntityId Analyzer::declare_object(NodeId d, NodeId init, TypeId t, NodeId specs, ScopeId s, NodeId source)
{
    NodeId name = decl_name(d);
    IdentifierId id = terminal(name);
    bool destructor = ast[ast[name].last].op == OP_COMPL;
    ScopeId owner = name_owner(name, s, true);
    if (definitions && owner == active_template_scope && scopes[scopes[owner].parent].kind == ScopeKind::Class)
        owner = scopes[owner].parent;
    ScopeId definition_scope = member_definition_environment == s ? s : owner;
    ScopeId enclosing = definitions && scopes[s].kind == ScopeKind::Template ? scopes[s].parent : s;
    bool retained_member = member_definition_environment && encloses(member_definition_environment,s) &&
        scopes[member_definition_environment].parent == owner;
    if (!encloses(enclosing, owner) && !retained_member) throw std::runtime_error("qualified definition outside enclosing scope");
    if (destructor && scopes[owner].kind == ScopeKind::Class) {
        TextView text = ids.spelling(scopes[owner].name);
        std::string label = "~" + std::string(text.data, text.size);
        id = ids.intern(TextView(label.data(), label.size()));
    }
    bool alias = spec_has(specs, KW_TYPEDEF);
    bool function = types[t].kind == TypeKind::Function;
    if (calls && spec_has(specs,KW_CONSTEXPR) && (alias || (!function && owner == s && scopes[owner].kind == ScopeKind::Class && !spec_has(specs,KW_STATIC))))
        throw std::runtime_error("invalid constexpr declaration specifier");
    bool block_extern = !alias && !function && spec_has(specs,KW_EXTERN) &&
        owner == s && scopes[owner].kind != ScopeKind::Namespace && scopes[owner].kind != ScopeKind::Class;
    if (block_extern) {
        if (init) throw std::runtime_error("block extern declaration has initializer");
        while (scopes[owner].kind != ScopeKind::Namespace) owner = scopes[owner].parent;
    }
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
    if (calls && init) {
        auto source = init;
        while (ast[source].kind == Kind::Initializer) source = ast[source].first;
        expand_expression_list(source,s);
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
    if (calls && function) t = constexpr_member_type(t,specs,source,d,owner);
    TypeId canonical = types.signature(t);
    if (definitions && !function && active_template_scope == s)
        return declare_variable_template(d,init,canonical,s,source);
    bool constructor = (ast[source].kind == Kind::SpecialMember || ast[source].kind == Kind::SpecialDefinition) &&
        scopes[owner].kind == ScopeKind::Class && scopes[owner].name == id && ast[ast[name].last].op != OP_COMPL;
    TypeId conversion_target = calls && ast[ast[name].last].op == KW_OPERATOR && ast[ast[name].last].detail ? types[canonical].child : 0;
    if (conversion_target && (scopes[owner].kind != ScopeKind::Class || spec_has(specs,KW_STATIC) ||
        spec_has(child(source,Kind::MemberSpecifiers),KW_STATIC) || types[canonical].count || types[canonical].variadic))
        throw std::runtime_error("conversion function must be a nonstatic nullary member");
    EntityId cls = constructor ? scopes[owner].entity : 0;
    if (calls && function && types[t].ref != RefQualifier::None &&
        (scopes[owner].kind != ScopeKind::Class || spec_has(specs, KW_STATIC) || constructor || destructor))
        throw std::runtime_error("ref qualifier requires ordinary nonstatic member");
    bool source_constructor = constructor && !entities[cls].class_info && entities[cls].template_pattern;
    EntityId e = constructor ? source_constructor ? template_pattern_members.get(key(cls,unsigned(PatternMemberKind::Constructors))) :
        class_facts[entities[cls].class_info].constructor : local(owner, id);
    bool specialized_template = false;
    if (source == explicit_specialization_source && function)
        for (auto candidate : candidates(e)) specialized_template |= entities[candidate].template_info != 0;
    if (source == explicit_specialization_source && scopes[owner].kind == ScopeKind::Class && !specialized_template) {
        auto cls = scopes[owner].entity;
        if (!entities[cls].specialization || entities[cls].explicit_specialization)
            throw std::runtime_error("member specialization requires an implicit class specialization");
        bool match = false;
        for (auto member : candidates(e))
            match |= entities[member].owner == owner && entities[member].kind == kind &&
                (function ? entities[member].type == canonical : entities[member].is_static);
        if (!match) throw std::runtime_error("specialized member has no matching declaration");
    }
    bool hidden_external = false;
    if (!e && kind == EntityKind::Variable && scopes[owner].kind == ScopeKind::Namespace) {
        e = block_extern_entities.get(key(owner,id)); hidden_external = e != 0;
    }
    if (source == explicit_specialization_source && !function && scopes[owner].kind != ScopeKind::Class) {
        if (!e || !entities[e].template_info) throw std::runtime_error("variable specialization requires a primary");
        e = variable_template_name(ast[name].last,e,s,false);
        if (entities[e].type != canonical) throw std::runtime_error("specialized variable type mismatch");
    } else if (source == explicit_specialization_source && !active_template_scope && function && (scopes[owner].kind != ScopeKind::Class || specialized_template)) {
        e = declare_function_specialization(name,s,canonical);
    } else if (constructor) {
        EntityId selected = declare_function(owner, id, source, canonical, true);
        if (source_constructor) template_pattern_members.put(key(cls,unsigned(PatternMemberKind::Constructors)),merge_lookup(e,selected));
        else class_facts[entities[cls].class_info].constructor = merge_lookup(e, selected);
        e = selected;
    }
    else if (function) e = declare_function(owner, id, source, canonical, false, conversion_target);
    else if (e && placeholder_objects.get(e) == d) {
        entities[e].type = canonical; placeholder_objects.put(e,0);
    }
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
        else if (!block_extern) bind(owner, id, e);
    }
    if (function && template_first_signature_index.get(e)) t = entities[e].type;
    if (source == explicit_specialization_source) {
        if (!function && entities[e].explicit_specialization && entities[e].definition && init)
            throw std::runtime_error("variable specialization redefinition");
        select_explicit_specialization(e,source);
    }
    if (block_extern) { block_extern_entities.put(key(owner,id),e); bind(s,id,e); }
    else if (hidden_external) bind(owner,id,e);
    entities[e].is_static |= spec_has(specs, KW_STATIC);
    if (calls && function && entities[e].is_static && types[t].ref != RefQualifier::None)
        throw std::runtime_error("static member cannot be ref qualified");
    entities[e].mutable_field |= spec_has(specs, KW_MUTABLE);
    if (calls && function) declare_operator(e, name);
    declaration_attributes(e,specs,source);
    bool specialized_member_declaration = source == explicit_specialization_source && !init && scopes[owner].kind == ScopeKind::Class;
    if (calls && !function && entities[e].definition && entities[e].is_static &&
        scopes[owner].kind == ScopeKind::Class && scopes[s].kind != ScopeKind::Class &&
        !specialized_member_declaration && source != explicit_specialization_source)
        throw std::runtime_error("static data member defined twice");
    if (calls && init && !function && entities[e].is_static && scopes[owner].kind == ScopeKind::Class &&
        entities[e].initializer && source != explicit_specialization_source)
        throw std::runtime_error("static member initializer specified twice");
    if (!function && !specialized_member_declaration && !spec_has(specs, KW_EXTERN) && !(scopes[s].kind == ScopeKind::Class && entities[e].is_static)) entities[e].definition = source;
    if (init && !function) { entities[e].initializer = init; if (!(scopes[s].kind == ScopeKind::Class && entities[e].is_static)) entities[e].definition = source; }
    if (calls && function) { function_defaults(e, d, definition_scope, source); exception_specification(e, d, definition_scope); }
    auto special = child(init, Kind::SpecialInitializer);
    if (!special) special = child(child(source, Kind::Initializer), Kind::SpecialInitializer);
    if (function && special && ast[special].op == KW_DELETE) {
        if (entities[e].source != source || entities[e].deleted_function) throw std::runtime_error("deleted definition must be the first declaration");
        entities[e].deleted_function = entities[e].inline_function = true;
    } else if (function && entities[e].deleted_function && ast[source].kind == Kind::Function)
        throw std::runtime_error("definition of deleted function");
    if (calls && function && scopes[owner].kind == ScopeKind::Class) {
        member_facts(e);
        auto m = entities[e].member_info;
        if (definitions && ast.nodes.occurrences[d].context && !members[m].prototype)
            members[m].prototype = template_prototype_sources.get(ast.nodes.occurrences[d].source);
        if (conversion_target && !members[m].conversion_target) {
            auto info = entities[scopes[owner].entity].class_info;
            members[m].conversion_target = conversion_target;
            members[m].next_conversion = class_facts[info].first_conversion;
            class_facts[info].first_conversion = e;
            auto k = key(owner,conversion_target);
            conversion_bindings.put(k,merge_lookup(conversion_bindings.get(k),e));
        }
        members[m].constructor = constructor;
        members[m].destructor = destructor;
        if (destructor) class_facts[entities[scopes[owner].entity].class_info].destructor = e;
        explicit_specifier(e,source,s);
        members[m].deleted = special && ast[special].op == KW_DELETE;
        classify_transfer(e, special, s);
        if (constructor && !special && !source_constructor) class_facts[entities[cls].class_info].aggregate = false;
    }
    if (calls) virtual_declaration(e, d, init, specs, source, s);
    if (calls && !function && spec_has(specs,KW_CONSTEXPR) && !literal_type(canonical))
        throw std::runtime_error("constexpr variable requires a literal type");
    record(block_extern ? s : owner, e, d, t, kind);
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
    if (calls && init && !function && !member_initializer) initialize(init, canonical, definition_scope);
    if (calls && init && !function && !alias && !member_initializer) retain_initializer_references(e);
    if (calls && !function && types[canonical].kind == TypeKind::Array &&
        (spec_has(specs,KW_CONSTEXPR) || (init && !member_initializer && !entities[e].is_static &&
            !entities[e].external_decl && scopes[owner].kind != ScopeKind::Namespace && scopes[owner].kind != ScopeKind::Class)))
        prepare_constant_array(e,spec_has(specs,KW_CONSTEXPR));
    if (calls && !entities[e].initializer && !function && !alias && scopes[s].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN)) default_initialize(e,d);
    if (init && !alias && !function && (!calls || (types[t].kind != TypeKind::LRef && types[t].kind != TypeKind::RRef)) && (integral(t) || floating_type(value_type(t))) && !member_initializer) {
        Constant v = calls ? convert(constant_initialize(init,t,definition_scope),t) : convert(evaluate(init, definition_scope),t);
        if (calls && spec_has(specs, KW_CONSTEXPR) && !v.valid) throw std::runtime_error("nonconstant constexpr initializer");
        if (v.valid) {
            // A reference may preserve an address constant while reads through
            // its volatile-qualified referent are never constant values.
            if (!(types[v.type].cv & 2) && (!floating_type(v.type) || spec_has(specs,KW_CONSTEXPR)) &&
                (types[t].cv == 1 || types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef))
                entities[e].constant = v;
        }
    }
    if (calls && !function && spec_has(specs,KW_CONSTEXPR) &&
        (!(integral(t) || floating_type(t)) || types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) && types[t].kind != TypeKind::Array)
        check_constant_object(e);
    if (calls && !entities[e].initializer && !function && (integral(t) || floating_type(value_type(t))) && spec_has(specs, KW_CONSTEXPR))
        throw std::runtime_error("constexpr object requires initializer");
    if (calls && init && spec_has(specs, KW_CONSTEXPR) && integral(t) && ast[ast[init].first].kind == Kind::Literal)
        facts.edit(ast[init].first).type = t;
    if (calls && !function && !alias && !member_initializer && scopes[s].kind != ScopeKind::Class && !spec_has(specs, KW_EXTERN)) register_destruction(e);
    if (definitions && !function && !alias && entities[e].definition && scopes[s].kind == ScopeKind::Namespace)
        demand_class_constant_storage(t);
    if (definitions && !unevaluated_depth && local_static(e) && static_initialization(e))
        demand_constant_relocations(constant_initialize(init,t,definition_scope,object_constructor(e)));
    return e;
}
} }
