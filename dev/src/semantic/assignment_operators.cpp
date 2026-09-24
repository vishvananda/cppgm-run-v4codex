#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
ETokenType Analyzer::compound_operation(ETokenType op) const
{
    switch (op) {
    case OP_PLUSASS: return OP_PLUS;
    case OP_MINUSASS: return OP_MINUS;
    case OP_STARASS: return OP_STAR;
    case OP_DIVASS: return OP_DIV;
    case OP_MODASS: return OP_MOD;
    case OP_BANDASS: return OP_AMP;
    case OP_BORASS: return OP_BOR;
    case OP_XORASS: return OP_XOR;
    case OP_LSHIFTASS: return OP_LSHIFT;
    case OP_RSHIFTASS: return OP_RSHIFT;
    default: return TOK_INVALID;
    }
}
void Analyzer::builtin_assignment_values(ETokenType op, const std::vector<Expression>& args,
    std::vector<BuiltinOperator>& results)
{
    auto binary = compound_operation(op);
    Index seen;
    auto add = [&](TypeId target, TypeId right, TypeId computation) {
        if (types[target].cv & 1 || seen.get(key(target,right))) return;
        seen.put(key(target,right),1);
        BuiltinOperator candidate; candidate.type = target; candidate.category = ValueCategory::Lvalue;
        candidate.computation = computation;
        auto left = args[0];
        // Builtin assignment modifies a bit-field directly; it does not bind
        // the reference parameter used to model an overload's object operand.
        if (field_fact(left.entity).bit_field) left.entity = 0;
        candidate.arguments[0] = conversion_value(left,types.compound(TypeKind::LRef,target));
        candidate.arguments[1] = conversion_value(args[1],right);
        if (candidate.arguments[0].valid() && candidate.arguments[1].valid()) results.push_back(candidate);
    };
    if (op == OP_ASS) {
        // Builtin assignment does not convert a class lhs. Its own member
        // assignment candidates were considered by the operator owner.
        auto t = args[0].type;
        if (arithmetic(t) || pointer(t) || types[t].kind == TypeKind::MemberPointer || scoped_enum(t) || fundamental(t,FT_NULLPTR_T)) add(t,t,0);
        return;
    }
    auto left = builtin_operand_types_value(args[0]), right = builtin_operand_types_value(args[1]);
    for (auto a : left) for (auto b : right) {
        auto target = class_value(args[0].type) ? a : args[0].type;
        if (types[a].kind == TypeKind::Named) continue;
        bool shift = binary == OP_LSHIFT || binary == OP_RSHIFT;
        bool integer = shift || binary == OP_MOD || binary == OP_AMP || binary == OP_BOR || binary == OP_XOR;
        if (arithmetic(a) && arithmetic(b) && (!integer || (integral(a) && integral(b))))
            add(target,shift ? promote(b) : arithmetic_type(a,b),shift ? promote(a) : arithmetic_type(a,b));
        if ((binary == OP_PLUS || binary == OP_MINUS) && object_pointer(a) && integral(b) && !scoped_enum(b)) {
            if (size(types[a].child,false,true)) add(target,promote(b),a);
        }
        // E1 op= E2 has the validity of E1 = E1 op E2, with one
        // evaluation of E1. A pointer sum can convert back to bool.
        if (binary == OP_PLUS && fundamental(a,FT_BOOL) && object_pointer(b) && size(types[b].child,false,true))
            add(target,b,promote(a));
    }
}
} }
