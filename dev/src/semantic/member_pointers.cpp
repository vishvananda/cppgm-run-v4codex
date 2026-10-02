#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeId Analyzer::form_member_pointer(TypeId owner, TypeId member)
{
    if (!owner || !member) return 0;
    owner = types.unqualified(types.signature(owner));
    auto t = types[owner], m = types[member];
    if ((!dependent_type(owner) && (t.kind != TypeKind::Named || !entities[t.entity].class_info)) ||
        m.kind == TypeKind::LRef || m.kind == TypeKind::RRef || fundamental(member,FT_VOID)) return 0;
    return types.member_pointer_type(owner,member);
}
TypeQueryFact Analyzer::member_pointer_value(Expression a, Expression b, ETokenType op, ScopeId s)
{
    auto fail = [] { return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands); };
    TypeId target = a.type;
    auto category = a.category;
    if (op == OP_ARROWSTAR) {
        target = decay(target);
        if (types[target].kind != TypeKind::Pointer) return fail();
        target = types[target].child; category = ValueCategory::Lvalue;
    }
    auto member = types[b.type];
    if (!class_value(target) || member.kind != TypeKind::MemberPointer) return fail();
    auto owner = member.member_owner();
    if (types.unqualified(target) != owner) {
        complete_class(types[target].entity);
        if (!entities[types[target].entity].complete) return incomplete_query(target);
        if (!derived_from(target,owner)) return fail();
        auto path = base_path(target,member.entity);
        if (base_adjustments[path].ambiguous || !base_accessible(types[target].entity,member.entity,s)) return fail();
    }
    TypeQueryFact fact; auto& result = fact.expression;
    result.type = member.child;
    auto value = types[member.child];
    if (value.kind == TypeKind::Function) {
        if ((types[target].cv & ~value.cv) ||
            (value.ref == RefQualifier::Lvalue && category != ValueCategory::Lvalue) ||
            (value.ref == RefQualifier::Rvalue && category == ValueCategory::Lvalue)) return fail();
        result.form = ExpressionForm::BoundMember;
    } else {
        result.type = types.qualify(member.child,value.cv | types[target].cv);
        result.category = category == ValueCategory::Lvalue ? category : ValueCategory::Xvalue;
    }
    record_object(result,0,target,base_steps(target,member.entity));
    return fact;
}
Expression Analyzer::member_pointer_expression(NodeId n, ScopeId s)
{
    auto object = ast.first(n), pointer = ast.next(object);
    auto fact = member_pointer_value(expressions[object],expressions[pointer],ast.op(n),s);
    if (fact.state == FactState::Failure) throw std::runtime_error("invalid member pointer application");
    auto result = fact.expression;
    auto& use = object_uses[result.object_use]; use.node = object; use.member_pointer = pointer;
    size(use.type);
    auto operand = pointer;
    while (ast.kind(operand) == syntax::Kind::Parenthesized) operand = ast.first(operand);
    auto e = expressions[operand].entity;
    if (current_function && result.form == ExpressionForm::BoundMember && nonstatic_field(e) &&
        !member_pointer_flow_requests.get(current_function)) {
        member_pointer_flow_requests.put(current_function,1);
        member_pointer_flow_functions.push_back(current_function);
    }
    if (result.form == ExpressionForm::BoundMember && ast.kind(operand) == syntax::Kind::IdExpression &&
        e && entities[e].kind == EntityKind::Enumerator && entities[e].constant.valid) {
        auto value = static_value(pointer,expressions[pointer].type);
        if (value.kind == StaticValue::MemberFunction && value.entity)
            object_uses[result.object_use].member_target = static_index.get(key(pointer,expressions[pointer].type));
    }
    return result;
}
} }
