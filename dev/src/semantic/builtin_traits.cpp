#include "semantic/analyzer.h"
#include "support/type_traits.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
namespace {
struct TraitProbe {
    bool& immediate; bool old_immediate; bool& explicit_access; bool old_explicit;
    ScopeId& access; ScopeId old_access;
    TraitProbe(bool& i, bool& e, ScopeId& a, ScopeId global) : immediate(i), old_immediate(i),
        explicit_access(e), old_explicit(e), access(a), old_access(a) { i=true; e=false; a=global; }
    ~TraitProbe(){immediate=old_immediate; explicit_access=old_explicit; access=old_access;}
};
bool referenceable(const Type& t)
{
    return !(t.kind == TypeKind::Fundamental && t.fundamental == FT_VOID) &&
        !(t.kind == TypeKind::Function && (t.cv || t.ref != RefQualifier::None));
}
TypeId remove_cv(Types& types, TypeId t, unsigned removed)
{
    auto type = types[t];
    if (type.kind == TypeKind::Array) return types.compound(TypeKind::Array,remove_cv(types,type.child,removed),type.bound,type.unknown_bound);
    return types.qualify(types.unqualified(t),type.cv & ~removed);
}
TypeId transform_type(Analyzer& sem, BuiltinTrait trait, TypeId t)
{
    auto& types = sem.types; auto type = types[t];
    bool ref = type.kind == TypeKind::LRef || type.kind == TypeKind::RRef;
    switch (trait) {
    case BuiltinTrait::RemoveCV: return types.unqualified(t);
    case BuiltinTrait::RemoveConst: return remove_cv(types,t,1);
    case BuiltinTrait::RemoveVolatile: return remove_cv(types,t,2);
    case BuiltinTrait::RemoveReference: return ref ? type.child : t;
    case BuiltinTrait::RemoveCVRef: return types.unqualified(ref ? type.child : t);
    case BuiltinTrait::RemovePointer: return type.kind == TypeKind::Pointer ? type.child : t;
    case BuiltinTrait::RemoveExtent: return type.kind == TypeKind::Array ? type.child : t;
    case BuiltinTrait::RemoveAllExtents:
        while (types[t].kind == TypeKind::Array) t = types[t].child;
        return t;
    case BuiltinTrait::AddPointer:
        if (ref) t = type.child;
        if (referenceable(types[t]) || (types[t].kind == TypeKind::Fundamental && types[t].fundamental == FT_VOID))
            return types.compound(TypeKind::Pointer,t);
        return t;
    case BuiltinTrait::AddLRef: case BuiltinTrait::AddRRef:
        return referenceable(type) ? types.compound(trait == BuiltinTrait::AddLRef ? TypeKind::LRef : TypeKind::RRef,t) : t;
    case BuiltinTrait::MakeSigned: case BuiltinTrait::MakeUnsigned: {
        bool enumeration = type.kind == TypeKind::Named && sem.entities[type.entity].key == KW_ENUM;
        if (enumeration) type = types[sem.entities[type.entity].underlying];
        auto f = type.fundamental;
        if (type.kind == TypeKind::Fundamental && bit_integer_kind(f)) {
            bool unsign = trait == BuiltinTrait::MakeUnsigned;
            if (!unsign && type.bound < 2) return 0;
            return types.qualify(types.bit_integer(type.bound,unsign),types[t].cv);
        }
        if (type.kind != TypeKind::Fundamental || f == FT_BOOL ||
            !(f < FT_BOOL || f == FT_INT128 || f == FT_UINT128)) return 0;
        bool uns = trait == BuiltinTrait::MakeUnsigned;
        // Named character types and enums use the first integer rank of their
        // width. Ordinary integers retain their rank (long versus long long).
        if (enumeration || f == FT_CHAR || f == FT_WCHAR_T || f == FT_CHAR16_T || f == FT_CHAR32_T) {
            auto width = sem.type_width(t);
            f = width == 8 ? FT_SIGNED_CHAR : width == 16 ? FT_SHORT_INT : width == 32 ? FT_INT :
                width == 64 ? FT_LONG_INT : FT_INT128;
        }
        if (f == FT_INT128 || f == FT_UINT128) f = uns ? FT_UINT128 : FT_INT128;
        else if (uns && f <= FT_LONG_LONG_INT) f = EFundamentalType(unsigned(f)+unsigned(FT_UNSIGNED_CHAR));
        else if (!uns && f >= FT_UNSIGNED_CHAR && f <= FT_UNSIGNED_LONG_LONG_INT)
            f = EFundamentalType(unsigned(f)-unsigned(FT_UNSIGNED_CHAR));
        return types.qualify(types.fundamental(f),types[t].cv);
    }
    default: return 0;
    }
}
}
QueryId Analyzer::type_operation_query(NodeId n, ScopeId s)
{
    auto node = ast[n]; auto first = node.first;
    if (node.kind == syntax::Kind::TypeTrait && BuiltinTrait(node.flags) == BuiltinTrait::Offsetof) return offsetof_query(n,s);
    TypeQuery q; std::vector<QueryId> children;
    if (node.kind == syntax::Kind::TypeTrait && node.flags) {
        q.kind = QueryKind::BuiltinTrait; q.value = node.flags; q.name = node.text;
        std::vector<ArgumentId> args;
        for (auto a = first; a; a = ast.next(a))
            append_template_argument(a,s,template_argument_node(a,s),args);
        // The builtin alias's index has a size_t parameter type. Retain that
        // converted identity even when its type pack remains dependent, so
        // substitution and ABI consumers see the same template argument.
        if (BuiltinTrait(node.flags) == BuiltinTrait::TypePackElement && !args.empty())
            if (auto index = convert_argument(args[0],types.fundamental(FT_UNSIGNED_LONG_INT))) args[0] = index;
        q.arguments = intern_arguments(args);
    } else {
        q.kind = node.op == KW_TYPEID ? QueryKind::Typeid : QueryKind::Sizeof; q.op = node.op;
        if (ast.kind(first) == syntax::Kind::TypeId) q.type = type_id(first,s);
        else children.push_back(expression_query(first,s));
        if (template_type_probe && (children.empty() ? !q.type : !children[0])) return 0;
    }
    return intern_query(q,children);
}
TypeQueryFact Analyzer::query_builtin_trait(QueryId id, const TypeQuery& query)
{
    auto trait = BuiltinTrait(query.value);
    auto args = argument_packs[query.arguments];
    if (template_type_transform(trait)) return query_template_type_trait(query);
    bool convertible = trait == BuiltinTrait::Convertible || trait == BuiltinTrait::NothrowConvertible;
    bool reference_temporary = trait >= BuiltinTrait::ReferenceBindsTemporary && trait <= BuiltinTrait::ReferenceConvertsTemporary;
    bool binary = convertible || reference_temporary || trait == BuiltinTrait::Same || trait == BuiltinTrait::BaseOf ||
        trait == BuiltinTrait::Assignable || trait == BuiltinTrait::NothrowAssignable || trait == BuiltinTrait::TriviallyAssignable;
    bool construct = trait == BuiltinTrait::Constructible || trait == BuiltinTrait::NothrowConstructible || trait == BuiltinTrait::TriviallyConstructible;
    if (!args.count || (binary ? args.count != 2 : !construct && args.count != 1))
        throw std::runtime_error("invalid type trait arity");
    auto t = argument_types[args.offset];
    TypeQueryFact result; result.expression.type = types.fundamental(FT_BOOL);
    if (trait == BuiltinTrait::ArrayRank) {
        unsigned rank = 0;
        while (types[t].kind == TypeKind::Array) { ++rank; t = types[t].child; }
        result.expression.type = types.fundamental(FT_UNSIGNED_LONG_INT);
        builtin_trait_values.put(id,constants.size()); constants.push_back(Constant(result.expression.type,rank));
        return result;
    }
    if (trait == BuiltinTrait::Decay) {
        if (types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef) t = types[t].child;
        result.expression.type = types.unqualified(decay(t)); return result;
    }
    if (trait == BuiltinTrait::Underlying) {
        if (types[t].kind != TypeKind::Named || entities[types[t].entity].key != KW_ENUM)
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        result.expression.type = entities[types[t].entity].underlying; return result;
    }
    if (type_transform(trait)) {
        auto transformed = transform_type(*this,trait,t);
        if (!transformed) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        result.expression.type = transformed; return result;
    }
    bool value = false;
    if (reference_temporary) {
        TraitProbe probe(immediate_query_probe,explicit_instantiation_naming,access_override,global);
        value = reference_temporary_property(unsigned(trait),t,argument_types[args.offset+1]);
    } else if (trait == BuiltinTrait::Same) value = t == argument_types[args.offset+1];
    else if (trait == BuiltinTrait::BaseOf) {
        auto u = argument_types[args.offset+1];
        bool classes = class_value(t) && class_value(u) && entities[types[t].entity].key != KW_UNION && entities[types[u].entity].key != KW_UNION;
        if (classes && types.unqualified(t) != types.unqualified(u)) {
            complete_class(types[u].entity);
            if (!entities[types[u].entity].complete) throw std::runtime_error("base trait requires a complete derived class");
        }
        value = classes && (types.unqualified(t) == types.unqualified(u) || class_derives(types[u].entity,types[t].entity));
    } else if (binary || construct) {
        // Probe a typed hypothetical expression in an unrelated access context.
        // Expected overload rejection stays in TypeQueryFact; no fake AST and
        // no function body demand are needed to answer an operation trait.
        TraitProbe probe(immediate_query_probe,explicit_instantiation_naming,access_override,global);
        if (convertible) {
            auto target = argument_types[args.offset+1];
            if (fundamental(t,FT_VOID) || fundamental(target,FT_VOID))
                value = fundamental(t,FT_VOID) && fundamental(target,FT_VOID);
            else if (types[target].kind != TypeKind::Array && types[target].kind != TypeKind::Function && referenceable(types[t])) {
                TypeQuery source; source.kind = QueryKind::Value;
                source.type = types.compound(TypeKind::RRef,t);
                auto x = query_fact(intern_query(source,{})).expression;
                auto c = conversion_value(x,target);
                value = valid_fixed_conversion(x,0,c,global);
                if (value && trait == BuiltinTrait::NothrowConvertible) value = conversion_nonthrowing(c);
            }
            builtin_trait_values.put(id,constants.size()); constants.push_back(Constant(result.expression.type,value));
            return result;
        }
        std::vector<QueryId> operands;
        std::vector<Expression> values;
        for (unsigned j = construct ? 1 : 0; j < args.count; ++j) {
            auto arg = argument_types[args.offset+j];
            if (fundamental(arg,FT_VOID)) { values.clear(); break; }
            TypeQuery value; value.kind = QueryKind::Value;
            value.type = types.compound(TypeKind::RRef,arg);
            auto operand = intern_query(value,{}); operands.push_back(operand);
            values.push_back(query_fact(operand).expression);
        }
        bool nothrow = trait == BuiltinTrait::NothrowConstructible || trait == BuiltinTrait::NothrowAssignable;
        bool trivial = trait == BuiltinTrait::TriviallyConstructible || trait == BuiltinTrait::TriviallyAssignable;
        if (values.size() == args.count-(construct ? 1 : 0)) {
            auto type = types[t];
            QueryId operation = 0;
            if (!construct) {
                TypeQuery assignment; assignment.kind = QueryKind::Binary; assignment.op = OP_ASS;
                assignment.name = operator_name(OP_ASS); assignment.context = global;
                operation = intern_query(assignment,operands);
            } else if (type.kind == TypeKind::Array) {
                if (args.count == 1 && !type.unknown_bound) {
                    TypeQuery element = query; element.arguments = intern_arguments({type.child});
                    auto child = intern_query(element,{}); query_fact(child);
                    value = constants[builtin_trait_values.get(child)].bits != 0;
                }
            } else if (class_value(t)) {
                TypeQuery target; target.kind = QueryKind::TypeValue; target.type = t;
                operands.insert(operands.begin(),intern_query(target,{}));
                TypeQuery call; call.kind = QueryKind::Call; call.context = global;
                operation = intern_query(call,operands);
            } else if (type.kind != TypeKind::Function && !fundamental(t,FT_VOID)) {
                bool ref = type.kind == TypeKind::LRef || type.kind == TypeKind::RRef;
                value = values.empty() && !ref;
                if (values.size() == 1) {
                    auto x = values[0];
                    auto c = direct_initialization_conversion(x,t);
                    value = valid_fixed_conversion(x,0,c,global);
                    if (value && nothrow) value = conversion_nonthrowing(c);
                    if (value && trivial) value = c.kind == Conversion::Kind::Standard;
                }
            }
            if (operation) {
                auto fact = query_fact(operation);
                value = fact.state == FactState::Success;
                // Parenthesized C++11 aggregate/array initialization cannot
                // accept an element list (braced initialization can).
                if (construct && fact.initialization && !values.empty()) value = false;
                if (value && nothrow) value = query_nonthrowing(operation);
                if (value && trivial) {
                    if (fact.selected) {
                        auto member = entities[fact.selected].member_info;
                        value = member && members[member].synthetic;
                        if (value && members[member].transfer != TransferKind::None) value = trivial_transfer(fact.selected);
                        else if (value && construct) value = default_constructor_valid(fact.selected) &&
                            members[member].default_properties == BooleanFact::True;
                        else value = false;
                    }
                    if (construct) value &= trivial_destructor(t);
                    for (unsigned j = 0; value && j < fact.expression.count; ++j)
                        value &= conversions[fact.expression.conversions+j].kind == Conversion::Kind::Standard;
                }
            }
        }
    }
    else value = builtin_type_property(unsigned(trait),t);
    builtin_trait_values.put(id,constants.size()); constants.push_back(Constant(result.expression.type,value));
    return result;
}
bool Analyzer::builtin_type_property(unsigned operation, TypeId t)
{
    auto trait = BuiltinTrait(operation);
    if (trait >= BuiltinTrait::TrivialConstructor && trait <= BuiltinTrait::NothrowAssign)
        return legacy_type_property(operation,t);
    auto type = types[t];
    bool named = type.kind == TypeKind::Named;
    bool enumeration = named && entities[type.entity].key == KW_ENUM;
    bool cls = named && !enumeration;
    bool ref = type.kind == TypeKind::LRef || type.kind == TypeKind::RRef;
    bool integer = type.kind == TypeKind::Fundamental && integral(t);
    bool floating = type.kind == TypeKind::Fundamental && floating_type(t);
    bool number = integer || floating || complex_type(t);
    bool function = type.kind == TypeKind::Function;
    bool ptr = type.kind == TypeKind::Pointer, member = type.kind == TypeKind::MemberPointer;
    switch (trait) {
    case BuiltinTrait::Const: case BuiltinTrait::Volatile:
        while (type.kind == TypeKind::Array) type = types[type.child];
        return !ref && !function && (type.cv & (trait == BuiltinTrait::Const ? 1 : 2));
    case BuiltinTrait::Void: return fundamental(t,FT_VOID);
    case BuiltinTrait::Array: return type.kind == TypeKind::Array && (type.unknown_bound || type.bound);
    case BuiltinTrait::BoundedArray: return type.kind == TypeKind::Array && type.bound;
    case BuiltinTrait::UnboundedArray: return type.kind == TypeKind::Array && type.unknown_bound;
    case BuiltinTrait::LvalueReference: return type.kind == TypeKind::LRef;
    case BuiltinTrait::RvalueReference: return type.kind == TypeKind::RRef;
    case BuiltinTrait::Reference: return ref;
    case BuiltinTrait::Pointer: return ptr;
    case BuiltinTrait::Function: return function;
    case BuiltinTrait::Object: return !ref && !function && !fundamental(t,FT_VOID);
    case BuiltinTrait::Integral: return integer;
    case BuiltinTrait::Floating: return floating;
    case BuiltinTrait::Arithmetic: return number;
    case BuiltinTrait::Fundamental: return type.kind == TypeKind::Fundamental;
    case BuiltinTrait::Compound: return type.kind != TypeKind::Fundamental;
    case BuiltinTrait::Referenceable: return referenceable(type);
    case BuiltinTrait::Signed: return floating || (integer && !is_unsigned(t));
    case BuiltinTrait::Unsigned: return integer && is_unsigned(t);
    case BuiltinTrait::Scalar: return number || enumeration || ptr || block_pointer(t) || member || fundamental(t,FT_NULLPTR_T);
    case BuiltinTrait::MemberPointer: return member;
    case BuiltinTrait::MemberObjectPointer: return member && types[type.child].kind != TypeKind::Function;
    case BuiltinTrait::MemberFunctionPointer: return member && types[type.child].kind == TypeKind::Function;
    case BuiltinTrait::Enum: return enumeration;
    case BuiltinTrait::Union: return cls && entities[type.entity].key == KW_UNION;
    case BuiltinTrait::Class: return cls && entities[type.entity].key != KW_UNION;
    default: break;
    }
    if (trait == BuiltinTrait::Destructible || trait == BuiltinTrait::TriviallyDestructible || trait == BuiltinTrait::NothrowDestructible) {
        if (ref) return true;
        if (type.kind == TypeKind::Array) return !type.unknown_bound && builtin_type_property(operation,type.child);
        if (function || fundamental(t,FT_VOID)) return false;
        if (!cls) return true;
        TraitProbe probe(immediate_query_probe,explicit_instantiation_naming,access_override,global);
        TypeQuery receiver; receiver.kind = QueryKind::Value; receiver.type = types.compound(TypeKind::LRef,t);
        TypeQuery destructor; destructor.kind = QueryKind::Destructor; destructor.type = t;
        destructor.context = global; destructor.value = 1; destructor.op = OP_DOT;
        auto query = intern_query(destructor,{intern_query(receiver,{})});
        auto fact = query_fact(query);
        if (fact.state != FactState::Success) return false;
        if (trait == BuiltinTrait::TriviallyDestructible) return trivial_destructor(t);
        if (trait == BuiltinTrait::NothrowDestructible) {
            TypeQuery call; call.kind = QueryKind::Call; call.context = global;
            return query_nonthrowing(intern_query(call,{query}));
        }
        return true;
    }
    if (cls) {
        complete_class(type.entity);
        if (!entities[type.entity].complete) throw std::runtime_error("type trait requires a complete class");
    }
    switch (trait) {
    case BuiltinTrait::Aggregate: return aggregate_type(t);
    case BuiltinTrait::Empty:
        if (!cls || entities[type.entity].key == KW_UNION) return false;
        class_layout(type.entity); return empty_class(t);
    case BuiltinTrait::Polymorphic: return cls && polymorphic(type.entity);
    case BuiltinTrait::Abstract: return cls && abstract_value(t);
    case BuiltinTrait::Final: return cls && class_facts[entities[type.entity].class_info].final_class;
    case BuiltinTrait::Literal: return literal_type(t);
    case BuiltinTrait::VirtualDestructor: return cls && member_fact(destructor_declaration(t)).virtual_member;
    case BuiltinTrait::TrivialDestructor:
        if (type.kind == TypeKind::LRef || type.kind == TypeKind::RRef) return true;
        if (fundamental(t,FT_VOID) || type.kind == TypeKind::Function) return false;
        return trivial_destructor(t);
    default: break;
    }
    if (type.kind == TypeKind::Array) return builtin_type_property(operation,type.child);
    if (!cls) return vector_kind(type.kind) || address_value(t) || type.kind == TypeKind::MemberPointer || enumeration ||
        (type.kind == TypeKind::Fundamental && type.fundamental != FT_VOID);
    auto identity = key(operation,types.unqualified(t));
    auto state = BooleanFact(builtin_type_properties.get(identity));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Active || state == BooleanFact::Failure) throw std::runtime_error("recursive/failed type property");
    builtin_type_properties.put(identity,unsigned(BooleanFact::Active));
    bool value = true;
    auto info = entities[type.entity].class_info;
    auto scope = entities[type.entity].scope;
    if (trait == BuiltinTrait::TriviallyCopyable) {
        value = trivial_destructor(t);
        for (unsigned assignment = 0; assignment < 2; ++assignment) {
            ensure_transfers(t,assignment);
            auto family = assignment ? local(scope,operator_name(OP_ASS)) : class_facts[info].constructor;
            for (auto e : candidates(family)) {
                if (entities[e].owner != scope || !transfer_member(e) || deleted_transfer(e)) continue;
                value &= trivial_transfer(e);
            }
        }
    } else if (trait == BuiltinTrait::Trivial) {
        value = builtin_type_property(unsigned(BuiltinTrait::TriviallyCopyable),t);
        value &= legacy_type_property(unsigned(BuiltinTrait::TrivialConstructor),t);
    } else if (trait == BuiltinTrait::Pod) {
        value = builtin_type_property(unsigned(BuiltinTrait::Trivial),t) &&
            builtin_type_property(unsigned(BuiltinTrait::StandardLayout),t);
        for (auto d = scopes[scope].first_decl; value && d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == scope)
                value &= builtin_type_property(operation,entities[field].type);
        }
    } else if (trait == BuiltinTrait::StandardLayout) {
        value = !dynamic_class(type.entity);
        bool fields = false; unsigned access = 0; TypeId first = 0;
        for (auto d = scopes[scope].first_decl; value && d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != scope) continue;
            auto member = entities[field];
            if (!fields) { first = member.type; access = unsigned(member.access); }
            fields = true;
            value &= access == unsigned(member.access) && builtin_type_property(operation,member.type);
        }
        while (types[first].kind == TypeKind::Array) first = types[first].child;
        unsigned data_bases = 0;
        for (auto b = class_facts[info].first_base; value && b; b = bases[b].next) {
            auto base = entities[bases[b].base].type;
            value &= !bases[b].virtual_base && builtin_type_property(operation,base);
            class_layout(bases[b].base);
            if (!empty_class(base)) ++data_bases;
            if (class_value(first)) value &= !class_derives(bases[b].base,types[first].entity);
        }
        value &= fields ? !data_bases : data_bases <= 1;
        // Repeated base subobjects also violate standard layout (CWG 1672).
        Index seen; std::vector<EntityId> pending{type.entity};
        for (unsigned i = 0; value && i < pending.size(); ++i) {
            auto current = pending[i];
            if (seen.get(current)) { value=false; break; }
            seen.put(current,1);
            for (auto b = class_facts[entities[current].class_info].first_base; b; b = bases[b].next)
                pending.push_back(bases[b].base);
        }
    } else throw std::logic_error("unknown structural type trait");
    builtin_type_properties.put(identity,unsigned(value ? BooleanFact::True : BooleanFact::False));
    return value;
}
} }
