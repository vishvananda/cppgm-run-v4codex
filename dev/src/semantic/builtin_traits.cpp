#include "semantic/analyzer.h"
#include "support/type_traits.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
QueryId Analyzer::type_operation_query(NodeId n, ScopeId s)
{
    auto node = ast[n]; auto first = node.first;
    TypeQuery q; std::vector<QueryId> children;
    if (node.kind == syntax::Kind::TypeTrait && node.flags) {
        q.kind = QueryKind::BuiltinTrait; q.value = node.flags; q.name = node.text;
        std::vector<ArgumentId> args;
        for (auto a = first; a; a = ast[a].next)
            append_template_argument(a,s,template_argument_node(a,s),args);
        q.arguments = intern_arguments(args);
    } else {
        q.kind = node.op == KW_TYPEID ? QueryKind::Typeid : QueryKind::Sizeof; q.op = node.op;
        if (ast[first].kind == syntax::Kind::TypeId) q.type = type_id(first,s);
        else children.push_back(expression_query(first,s));
        if (template_type_probe && (children.empty() ? !q.type : !children[0])) return 0;
    }
    return intern_query(q,children);
}
TypeQueryFact Analyzer::query_builtin_trait(QueryId id, const TypeQuery& query)
{
    auto trait = BuiltinTrait(query.value);
    auto args = argument_packs[query.arguments];
    bool binary = trait == BuiltinTrait::Same || trait == BuiltinTrait::BaseOf ||
        trait == BuiltinTrait::Assignable || trait == BuiltinTrait::NothrowAssignable || trait == BuiltinTrait::TriviallyAssignable;
    bool construct = trait == BuiltinTrait::Constructible || trait == BuiltinTrait::NothrowConstructible || trait == BuiltinTrait::TriviallyConstructible;
    if (!args.count || (binary ? args.count != 2 : !construct && args.count != 1))
        throw std::runtime_error("invalid type trait arity");
    auto t = argument_types[args.offset];
    TypeQueryFact result; result.expression.type = types.fundamental(FT_BOOL);
    if (trait == BuiltinTrait::Underlying) {
        if (types[t].kind != TypeKind::Named || entities[types[t].entity].key != KW_ENUM)
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        result.expression.type = entities[types[t].entity].underlying; return result;
    }
    bool value = false;
    if (trait == BuiltinTrait::Same) value = t == argument_types[args.offset+1];
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
        struct Probe {
            bool& immediate; bool old_immediate; bool& explicit_access; bool old_explicit;
            ScopeId& access; ScopeId old_access;
            Probe(bool& i, bool& e, ScopeId& a, ScopeId global) : immediate(i), old_immediate(i),
                explicit_access(e), old_explicit(e), access(a), old_access(a) { i=true; e=false; a=global; }
            ~Probe(){immediate=old_immediate; explicit_access=old_explicit; access=old_access;}
        } probe(immediate_query_probe,explicit_instantiation_naming,access_override,global);
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
                if (args.count == 1 && type.bound) {
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
                    auto c = standard_conversion(x,t);
                    if (!c.valid() && class_value(x.type)) c = conversion_function_value(x,t,true,ref);
                    if (!c.valid()) c = conversion_value(x,t);
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
    auto type = types[t];
    bool named = type.kind == TypeKind::Named;
    bool enumeration = named && entities[type.entity].key == KW_ENUM;
    bool cls = named && !enumeration;
    switch (trait) {
    case BuiltinTrait::Enum: return enumeration;
    case BuiltinTrait::Union: return cls && entities[type.entity].key == KW_UNION;
    case BuiltinTrait::Class: return cls && entities[type.entity].key != KW_UNION;
    default: break;
    }
    if (cls) {
        complete_class(type.entity);
        if (!entities[type.entity].complete) throw std::runtime_error("type trait requires a complete class");
    }
    switch (trait) {
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
    if (!cls) return type.kind == TypeKind::Pointer || type.kind == TypeKind::MemberPointer || enumeration ||
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
        auto ctor = default_constructor(t,scope,false);
        value &= ctor && default_constructor_valid(ctor) &&
            members[entities[ctor].member_info].default_properties == BooleanFact::True;
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
