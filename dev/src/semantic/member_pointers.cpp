#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Expression Analyzer::member_pointer_expression(NodeId n, ScopeId s)
{
    NodeId object = ast[n].first, pointer = ast[object].next;
    auto a = expressions[object], b = expressions[pointer];
    TypeId target = a.type;
    auto category = a.category;
    if (ast[n].op == OP_ARROWSTAR) {
        target = decay(target);
        if (types[target].kind != TypeKind::Pointer) throw std::runtime_error("member application requires object pointer");
        target = types[target].child; category = ValueCategory::Lvalue;
    }
    auto member = types[b.type];
    if (!class_value(target) || member.kind != TypeKind::MemberPointer)
        throw std::runtime_error("invalid member pointer application");
    TypeId owner = entities[member.entity].type;
    if (types.unqualified(target) != types.unqualified(owner)) {
        if (!derived_from(target,owner)) throw std::runtime_error("unrelated member pointer object");
        check_base_access(target,owner,s);
    }
    size(target);
    Expression result;
    result.type = member.child;
    auto value = types[member.child];
    if (value.kind == TypeKind::Function) {
        if ((types[target].cv & ~value.cv) ||
            (value.ref == RefQualifier::Lvalue && category != ValueCategory::Lvalue) ||
            (value.ref == RefQualifier::Rvalue && category == ValueCategory::Lvalue))
            throw std::runtime_error("member pointer object qualifiers");
        result.form = ExpressionForm::BoundMember;
    } else {
        result.type = types.qualify(member.child,value.cv | types[target].cv);
        result.category = category == ValueCategory::Lvalue ? category : ValueCategory::Xvalue;
    }
    record_object(result,object,target,base_steps(target,member.entity));
    object_uses[result.object_use].member_pointer = pointer;
    return result;
}
} }
