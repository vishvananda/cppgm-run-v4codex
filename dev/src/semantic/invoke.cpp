#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::invoke_member_value(Expression member, Expression object, ScopeId s, EntityId ordinary)
{
    const auto owner = types[member.type].member_owner();
    const bool direct = class_value(object.type) &&
        (types.unqualified(object.type) == owner || derived_from(object.type,owner));
    RangeOperation dereference;
    auto receiver = object;
    if (!direct) {
        TypeQuery query; query.kind = QueryKind::Unary; query.op = OP_STAR;
        query.name = operator_name(OP_STAR); query.context = s;
        query.entity = ordinary;
        TypeQueryFact child; child.expression = object;
        auto fact = query_operator(query,{child});
        if (fact.state == FactState::Failure || fact.incomplete) return fact;
        dereference.op = OP_STAR; dereference.function = fact.selected;
        dereference.result = fact.expression;
        dereference.receiver = fact.selected && entities[fact.selected].member_info && !entities[fact.selected].is_static;
        receiver = fact.expression;
        if (receiver.category == ValueCategory::Prvalue && class_value(receiver.type) &&
            (abstract_value(receiver.type) || !default_destruction_valid(receiver.type,s)))
            return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        prepare_range_operation(dereference,{object},s);
    }
    auto result = member_pointer_value(receiver,member,OP_DOTSTAR,s);
    if (result.state == FactState::Failure || result.incomplete) return result;
    if (!direct) {
        object_uses[result.expression.object_use].invoke_dereference = invoke_dereferences.size();
        invoke_dereferences.push_back(dereference);
    }
    if (result.expression.form != ExpressionForm::BoundMember)
        result.expression.form = ExpressionForm::InvokeMemberData;
    return result;
}
TypeQueryFact Analyzer::query_invoke_member(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    if (children.size() < 2) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
    auto bound = invoke_member_value(children[0].expression,children[1].expression,q.context,q.entity);
    if (bound.state == FactState::Failure || bound.incomplete) return bound;
    if (bound.expression.form == ExpressionForm::InvokeMemberData)
        return children.size() == 2 ? bound : TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
    std::vector<TypeQueryFact> arguments(1,bound);
    arguments.insert(arguments.end(),children.begin()+2,children.end());
    // Reuse indirect-call conversion checking with a typed bound function.
    // The original canonical query remains the cache owner.
    return query_call(q,arguments);
}
Expression Analyzer::invoke_call(NodeId n, ScopeId s, NodeId callee, std::vector<NodeId> args)
{
    auto fn = expressions[callee];
    if (types[fn.type].kind != TypeKind::MemberPointer)
        return callable_expression(n,s,callee,std::move(args),fn);
    if (args.empty()) throw std::runtime_error("member invocation requires a receiver");
    auto receiver = args.front(); args.erase(args.begin());
    auto bound = query_fact(expression_query(n,s));
    if (bound.state == FactState::Failure || bound.incomplete)
        throw std::runtime_error("invalid invocation receiver");
    auto result = bound.expression;
    auto use = object_uses[result.object_use];
    use.node = receiver; use.member_pointer = callee;
    size(use.type);
    if (use.invoke_dereference) {
        auto operation = invoke_dereferences[use.invoke_dereference];
        prepare_range_operation(operation,{expressions[receiver]},s,!unevaluated_depth);
        use.invoke_dereference = invoke_dereferences.size(); invoke_dereferences.push_back(operation);
    }
    result.object_use = object_uses.size(); object_uses.push_back(use);
    if (result.form == ExpressionForm::InvokeMemberData) {
        if (!args.empty()) throw std::runtime_error("member data invocation takes one receiver");
        facts.edit(n).type = result.type;
        return result;
    }
    auto returned = types[types[fn.type].child].child;
    facts.edit(n).type = returned;
    std::vector<Conversion> selected;
    for (unsigned i = 0; i < args.size(); ++i)
        selected.push_back(copy_conversion_recipe(conversions[result.conversions+i]));
    record_call(result,args,selected);
    return result;
}
bool Analyzer::invoke_receiver_nonthrowing(const ObjectUse& use)
{
    if (!use.invoke_dereference) return true;
    auto op = invoke_dereferences[use.invoke_dereference];
    bool result = !op.function || function_nonthrowing(op.function);
    for (unsigned i = 0; i < op.result.count; ++i)
        result &= conversion_nonthrowing(conversions[op.result.conversions+i]);
    if (class_value(op.result.type) && op.result.category == ValueCategory::Prvalue)
        result &= type_destructor_nonthrowing(op.result.type);
    return result;
}
std::uint32_t Analyzer::constant_invoke_receiver(const ObjectUse& use, ScopeId s)
{
    if (!use.invoke_dereference) return constant_node_object(use.node);
    auto op = invoke_dereferences[use.invoke_dereference];
    if (!op.function) {
        auto value = constant_node_conversion(use.node,conversions[op.result.conversions],s);
        return value.valid ? value.bits : 0;
    }
    if (!entities[op.function].constexpr_function || op.virtual_slot) return 0;
    std::vector<Constant> arguments;
    auto object = op.receiver ? constant_base_projection(constant_address(use.node,s),op.adjustment) : 0;
    if (!op.receiver) {
        auto value = constant_node_conversion(use.node,conversions[op.result.conversions],s);
        if (!value.valid) return 0;
        arguments.push_back(value);
    }
    auto value = execute_constant(op.function,arguments,object);
    if (!value.valid) return 0;
    return types[op.returned].kind == TypeKind::LRef || types[op.returned].kind == TypeKind::RRef ?
        value.bits : constant_storage_address(value.type,value);
}
Constant Analyzer::constant_query_invoke(QueryId id)
{
    auto q = type_queries[id]; auto fact = query_fact(id);
    auto member = constants[query_value(query_edges[q.offset])];
    if (!member.valid || !member.bits) return Constant();
    auto e = member_constant_value(member).member;
    auto use = object_uses[fact.expression.object_use];
    auto source = query_edges[q.offset+1];
    std::uint32_t object = 0;
    if (use.invoke_dereference) {
        auto op = invoke_dereferences[use.invoke_dereference];
        Constant value;
        if (!op.function) value = constant_query_conversion(source,conversions[op.result.conversions]);
        else {
            if (!entities[op.function].constexpr_function || op.virtual_slot) return Constant();
            auto receiver = op.receiver ? constant_base_projection(constant_query_object(source),op.adjustment) : 0;
            std::vector<Constant> args;
            if (!op.receiver) {
                auto arg = constant_query_conversion(source,conversions[op.result.conversions]);
                if (!arg.valid) return Constant();
                args.push_back(arg);
            }
            value = execute_constant(op.function,args,receiver);
        }
        if (!value.valid) return Constant();
        object = !op.function || types[op.returned].kind == TypeKind::LRef || types[op.returned].kind == TypeKind::RRef ?
            value.bits : constant_storage_address(value.type,value);
    } else object = constant_query_object(source);
    object = constant_member_receiver(constant_base_projection(object,use.adjustment),member);
    if (!object) return Constant();
    if (fact.expression.form == ExpressionForm::InvokeMemberData) {
        auto reference = types.compound(fact.expression.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef,fact.expression.type);
        return Constant(reference,constant_subobject(object,entities[e].type,e));
    }
    if (!entities[e].constexpr_function || members[entities[e].member_info].virtual_member) return Constant();
    std::vector<Constant> args;
    for (unsigned i = 2; i < q.count; ++i) {
        auto value = constant_query_conversion(query_edges[q.offset+i],conversions[fact.expression.conversions+i-2]);
        if (!value.valid) return Constant();
        args.push_back(value);
    }
    return execute_constant(e,args,object);
}
} }
