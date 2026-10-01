#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
TypeId Analyzer::form_block_pointer(TypeId child)
{
    if (!child) return 0;
    auto type = types[child];
    if (type.kind != TypeKind::Function || type.cv || type.ref != RefQualifier::None) return 0;
    return types.compound(TypeKind::BlockPointer,child);
}
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
    NodeId bit_width = 0;
    EFundamentalType fundamental = FT_INT;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        const syntax::Node node = ast[c];
        if (template_type_probe && (node.kind == Kind::Class || node.kind == Kind::ClassForward || node.kind == Kind::Enum) && !facts[c].type) return 0;
        if (node.kind == Kind::Class || node.kind == Kind::ClassForward) {
            result = class_type(c, s, anonymous_name, node.kind == Kind::Class, spec_has(n, KW_STATIC));
            continue;
        }
        if (node.kind == Kind::VectorAttribute) continue;
        if (node.kind == Kind::BitIntType) {
            if (bit_width) throw std::runtime_error("duplicate bit-integer specifier");
            bit_width = node.first; continue;
        }
        if (node.kind == Kind::AtomicType) {
            result = type_id(node.first,s);
            if (!atomic_operand(result)) throw std::runtime_error("invalid atomic type operand");
            result = types.qualify(result,4); continue;
        }
        if (node.kind == Kind::Enum) { result = enum_type(c, s, anonymous_name, node.flags & 1); continue; }
        if (node.op == KW_DECLTYPE) {
            if (node.flags & 2) {
                if (ast[node.first].kind == Kind::TypeId) result = type_id(node.first,s);
                else {
                    TypeQuery query; query.kind = QueryKind::Typeof;
                    result = query_decltype(intern_query(query,{expression_query(node.first,s)}),false);
                }
            } else result = expression_type(node.first, s, true);
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
        case KW_INT128: fundamental = FT_INT128; break;
        case KW_UINT128: fundamental = FT_UINT128; break;
        case KW_FLOAT: fundamental = FT_FLOAT; break;
        case KW_DOUBLE: fundamental = FT_DOUBLE; break;
        case KW_VOID: fundamental = FT_VOID; break;
        case KW_AUTO: if (calls) result = placeholder_type(); break;
        default: break;
        }
    }
    if (bit_width) {
        if (result || longs || short_int || (unsign && sign) || fundamental != FT_INT || spec_has(n,KW_INT))
            throw std::runtime_error("invalid bit-integer type specifiers");
        if (pattern_scope(s)) bind_template_expression(bit_width,s);
        result = bit_integer_type(expression_query(bit_width,s),unsign);
        if (!result) {
            if (template_type_probe) return 0;
            throw std::runtime_error("invalid bit-integer width (supported range: signed 2..128, unsigned 1..128)");
        }
    }
    if (!result) {
        if (fundamental == FT_INT128 && unsign) fundamental = FT_UINT128;
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
    result = types.qualify(vector_attributes(result,n,s), cv);
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
    base = vector_attributes(base,n,s);
    NodeId nested = 0;
    std::vector<NodeId> suffixes;
    bool after_direct = false;
    for (NodeId c = ast[n].first; c; c = ast[c].next) {
        switch (ast[c].kind) {
        case Kind::Pointer:
            if (ast[c].detail) {
                auto owner = definitions ? type_name(ast[c].detail,s) : source_type(resolve(ast[c].detail,s,Lookup::Qualifier));
                base = form_member_pointer(owner,base);
                if (!base) {
                    if (template_type_probe) return 0;
                    throw std::runtime_error("invalid member pointer type");
                }
                break;
            }
            if (ast[c].op == OP_XOR) {
                base = form_block_pointer(base);
                if (!base) {
                    if (template_type_probe) return 0;
                    throw std::runtime_error("block pointer requires an unqualified function type");
                }
            } else base = types.compound(ast[c].op == OP_AMP ? TypeKind::LRef :
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
            if (calls && types.storage_alignment(base) && !dependent_type(base) && size(base)%size(base,true))
                throw std::runtime_error("array element size is not a multiple of its alignment");
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
                // Only the first new[] extent is an ordinary evaluated
                // expression. Other array dimensions require constant values.
                if (c == dynamic_array) {
                    EvaluationScope checking(*this,true);
                    auto x = expression(ast[c].first,s);
                    if (!integral(x.type) || scoped_enum(x.type)) throw std::runtime_error("array allocation bound must be integral");
                }
                EvaluationScope mode(*this,c != dynamic_array);
                Constant v = evaluate(ast[c].first, s);
                if (c == dynamic_array) {
                    if (v.valid && negative_constant(v)) throw std::runtime_error("negative array allocation bound");
                } else if (!v.valid || !integral(v.type) || scoped_enum(v.type) || (!v.bits && !host_abi) ||
                    (negative_constant(v) || integer_value(v) > ~std::uint64_t(0)))
                    throw std::runtime_error("array bound must be a nonnegative integral constant");
                bound = v.valid ? std::uint64_t(integer_value(v)) : 0;
            }
            base = types.compound(TypeKind::Array, base, bound,!ast[c].first);
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
                if (definitions && declarator_pack(d)) {
                    // An unnamed nondependent parameter followed by ... is
                    // the comma-optional C varargs form, not a pack expansion.
                    if (!decl_name(d) && !argument_packs[expansion_parameters(params.back())].count) variadic = true;
                    else params.back() = types.compound(TypeKind::PackExpansion,0,params.back());
                }
                else if (declarator_pack(d)) variadic = true;
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
namespace {
IdentifierId destructor_identifier(IdentifierTable& ids, IdentifierId class_name)
{
    auto text = ids.spelling(class_name);
    auto label = "~" + std::string(text.data,text.size);
    return ids.intern(TextView(label.data(),label.size()));
}
}
ScopeId Analyzer::object_declaration_owner(NodeId name, ScopeId s)
{
    ScopeId owner = name_owner(name, s, true);
    if (definitions && owner == active_template_scope && scopes[scopes[owner].parent].kind == ScopeKind::Class)
        owner = scopes[owner].parent;
    ScopeId enclosing = definitions && scopes[s].kind == ScopeKind::Template ? scopes[s].parent : s;
    bool retained_member = member_definition_environment && encloses(member_definition_environment,s) &&
        scopes[member_definition_environment].parent == owner;
    if (!encloses(enclosing, owner) && !retained_member) throw std::runtime_error("qualified definition outside enclosing scope");
    return owner;
}
EntityId Analyzer::declare_object(NodeId d, NodeId init, TypeId t, NodeId specs, ScopeId s, NodeId source)
{
    bool external = spec_has(specs,KW_EXTERN) || linkage_extern_declarations.get(source);
    NodeId name = decl_name(d);
    IdentifierId id = terminal(name);
    bool destructor = ast[ast[name].last].op == OP_COMPL;
    ScopeId owner = object_declaration_owner(name,s);
    ScopeId definition_scope = member_definition_environment == s ? s : owner;
    if (destructor && scopes[owner].kind == ScopeKind::Class)
        id = destructor_identifier(ids,scopes[owner].name);
    bool alias = spec_has(specs, KW_TYPEDEF);
    bool function = types[t].kind == TypeKind::Function;
    if (calls && spec_has(specs,KW_CONSTEXPR) && (alias || (!function && owner == s && scopes[owner].kind == ScopeKind::Class && !spec_has(specs,KW_STATIC))))
        throw std::runtime_error("invalid constexpr declaration specifier");
    bool block_extern = !alias && !function && external &&
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
            scopes[s].kind != ScopeKind::Class && !external)
            throw std::runtime_error("uninitialized reference");
    }
    EntityKind kind = alias ? EntityKind::Alias : function ? EntityKind::Function : EntityKind::Variable;
    if (alias) {
        EntityId e = declare_alias(owner, id, d, t);
        record(owner, e, d, t, kind);
        return e;
    }
    bool defer_inline = definitions && !function && scopes[s].kind == ScopeKind::Class &&
        spec_has(specs,KW_STATIC) && spec_has(specs,KW_INLINE) && ast.nodes.occurrences[source].context;
    if (calls && init && !defer_inline) {
        auto source = init;
        while (ast[source].kind == Kind::Initializer) source = ast[source].first;
        expand_expression_list(source,s);
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
            !external) throw std::runtime_error("duplicate local variable");
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
    if (!function && types.storage_alignment(t)) field_metadata(e).type_alignment = types.storage_alignment(t);
    if (block_extern) { block_extern_entities.put(key(owner,id),e); bind(s,id,e); }
    else if (hidden_external) bind(owner,id,e);
    if (calls && scopes[owner].kind == ScopeKind::Namespace && spec_has(specs,KW_STATIC) &&
        entities[e].source != source && !entities[e].is_static && entities[e].c_linkage)
        throw std::runtime_error("static declaration follows external C declaration");
    entities[e].is_static |= spec_has(specs, KW_STATIC);
    if (calls && function && entities[e].is_static && types[t].ref != RefQualifier::None)
        throw std::runtime_error("static member cannot be ref qualified");
    entities[e].mutable_field |= spec_has(specs, KW_MUTABLE);
    if (calls && function) declare_operator(e, name);
    declaration_attributes(e,specs,source,d);
    for (auto label = ast[d].first; label; label = ast[label].next) {
        if (ast[label].kind != Kind::Specifier || ast[label].op != KW_ASM) continue;
        const auto& literal = ast.literals[ast[ast[label].first].literal];
        if (literal.kind != LiteralKind::string || literal.type != FT_CHAR || literal.bytes < 2)
            throw std::runtime_error("asm label requires a nonempty narrow string");
        auto data = ast.literal_bytes.data()+literal.offset;
        for (unsigned i = 0; i+1 < literal.bytes; ++i)
            if (!data[i]) throw std::runtime_error("embedded null in asm label");
        auto object_name = ids.intern(TextView(data,literal.bytes-1));
        auto prior = assembler_names.get(e);
        if (prior && prior != object_name) throw std::runtime_error("conflicting asm labels");
        assembler_names.put(e,object_name);
    }
    bool specialized_member_declaration = source == explicit_specialization_source && !init && scopes[owner].kind == ScopeKind::Class;
    if (calls && !function && entities[e].definition && entities[e].is_static &&
        scopes[owner].kind == ScopeKind::Class && scopes[s].kind != ScopeKind::Class &&
        !specialized_member_declaration && source != explicit_specialization_source)
        throw std::runtime_error("static data member defined twice");
    if (calls && init && !function && entities[e].is_static && scopes[owner].kind == ScopeKind::Class &&
        entities[e].initializer && source != explicit_specialization_source)
        throw std::runtime_error("static member initializer specified twice");
    bool member_declaration = scopes[s].kind == ScopeKind::Class && entities[e].is_static && !entities[e].inline_variable;
    if (calls && !function && entities[e].inline_variable && entities[e].definition &&
        (init || (!external && !member_declaration)))
        throw std::runtime_error("inline variable defined twice in one translation unit");
    if (!function && !specialized_member_declaration && !external && !member_declaration) entities[e].definition = source;
    if (init && !function) { entities[e].initializer = init; if (!member_declaration) entities[e].definition = source; }
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
    if (calls && init && !function && !defer_inline && types[t].kind == TypeKind::Array && types[t].unknown_bound) {
        // The entity is visible during its initializer, but its bound is not
        // guessed from source clauses: brace elision may consume several per
        // element. Reuse the checked plan when publishing the completed type.
        t = complete_array_initializer(init,entities[e].type,definition_scope);
        canonical = types.signature(t); entities[e].type = canonical;
    }
    if (calls && !function && !defer_inline && spec_has(specs,KW_CONSTEXPR) && !literal_type(canonical))
        throw std::runtime_error("constexpr variable requires a literal type");
    record(block_extern ? s : owner, e, d, t, kind);
    if (defer_inline) {
        InlineVariableDefinition def;
        def.declarator = d; def.specifiers = specs; def.scope = s;
        inline_variable_definitions.put(e,inline_variable_recipes.size());
        inline_variable_recipes.push_back(def);
    } else finish_object_initializer(e,init,d,specs,s,definition_scope,t,external);
    return e;
}
} }
