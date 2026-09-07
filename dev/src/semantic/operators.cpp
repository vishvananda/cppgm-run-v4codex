#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::modifiable(NodeId n)
{
    Expression e = expressions[n];
    if (e.category != ValueCategory::Lvalue || (types[e.type].cv & 1) || types[e.type].kind == TypeKind::Array ||
        types[e.type].kind == TypeKind::Function) throw std::runtime_error("modifiable lvalue required");
}
Expression Analyzer::unary_expression(NodeId n, ScopeId s)
{
    NodeId operand = ast[n].first;
    Expression a = expression(operand, s), r;
    ETokenType op = ast[n].op;
    if (op == OP_AMP) {
        if (a.form == ExpressionForm::Overload) return a;
        if (a.category == ValueCategory::Prvalue) throw std::runtime_error("address of rvalue");
        r.type = types.compound(TypeKind::Pointer, a.type); return r;
    }
    TypeId t = decay(a.type);
    if (op == OP_STAR) {
        if (!pointer(t) || fundamental(types[t].child, FT_VOID)) throw std::runtime_error("invalid dereference");
        r.type = types[t].child; r.category = ValueCategory::Lvalue; return r;
    }
    if (op == OP_INC || op == OP_DEC) {
        modifiable(operand);
        if ((!arithmetic(a.type) && !pointer(t)) || types[a.type].kind == TypeKind::Named ||
            (op == OP_DEC && fundamental(a.type, FT_BOOL))) throw std::runtime_error("invalid increment operand");
        r.type = ast[n].kind == Kind::Postfix ? types.unqualified(a.type) : a.type;
        r.category = ast[n].kind == Kind::Postfix ? ValueCategory::Prvalue : ValueCategory::Lvalue;
        return r;
    }
    if (op == OP_LNOT) { require_conversion(operand, types.fundamental(FT_BOOL)); r.type = types.fundamental(FT_BOOL); return r; }
    if (!arithmetic(t) || (op == OP_COMPL && !integral(t))) throw std::runtime_error("invalid unary arithmetic");
    r.type = promote(t); return r;
}
TypeId Analyzer::builtin_binary(ETokenType op, NodeId an, NodeId bn)
{
    TypeId a = decay(expressions[an].type), b = decay(expressions[bn].type);
    bool compare = op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    if (op == OP_LAND || op == OP_LOR) {
        require_conversion(an, types.fundamental(FT_BOOL)); require_conversion(bn, types.fundamental(FT_BOOL));
        return types.fundamental(FT_BOOL);
    }
    if (compare) {
        if (a == b && (scoped_enum(a) || fundamental(a, FT_NULLPTR_T))) return types.fundamental(FT_BOOL);
        bool comparable = pointer(a) && (conversion(bn, a).valid() || (pointer(b) && conversion(an, b).valid()));
        comparable |= pointer(b) && null_constant(an);
        if (comparable) return types.fundamental(FT_BOOL);
    }
    if (op == OP_PLUS || op == OP_MINUS) {
        if (pointer(a) && integral(b) && !scoped_enum(b)) return a;
        if (op == OP_PLUS && pointer(b) && integral(a) && !scoped_enum(a)) return b;
        if (op == OP_MINUS && pointer(a) && pointer(b) &&
            types.unqualified(types[a].child) == types.unqualified(types[b].child)) return types.fundamental(FT_LONG_INT);
    }
    TypeId result = arithmetic_type(a, b);
    if (op == OP_MOD || op == OP_AMP || op == OP_BOR || op == OP_XOR || op == OP_LSHIFT || op == OP_RSHIFT) {
        if (!integral(a) || !integral(b)) throw std::runtime_error("integral operands required");
    }
    if (op == OP_LSHIFT || op == OP_RSHIFT) return promote(a);
    return compare ? types.fundamental(FT_BOOL) : result;
}
Expression Analyzer::binary_expression(NodeId n, ScopeId s)
{
    NodeId an = ast[n].first, bn = ast[an].next;
    Expression a = expression(an, s), b = expression(bn, s), r;
    ETokenType op = ast[n].op;
    if (ast[n].kind == Kind::Conditional) {
        require_conversion(an, types.fundamental(FT_BOOL));
        NodeId cn = ast[bn].next;
        Expression c = expression(cn, s);
        if (b.type == c.type && b.category == c.category && b.category != ValueCategory::Prvalue) return b;
        TypeId bt = decay(b.type), ct = decay(c.type);
        if (bt == ct) r.type = bt;
        else if (pointer(bt) && conversion(cn, bt).valid()) r.type = bt;
        else if (pointer(ct) && conversion(bn, ct).valid()) r.type = ct;
        else r.type = arithmetic_type(bt, ct);
        return r;
    }
    if (op == OP_COMMA) return b;
    if (ast[n].kind == Kind::Assignment) {
        modifiable(an);
        if (op == OP_ASS) require_conversion(bn, a.type);
        else {
            ETokenType binary = OP_PLUS;
            switch (op) {
            case OP_PLUSASS: binary = OP_PLUS; break;
            case OP_MINUSASS: binary = OP_MINUS; break;
            case OP_STARASS: binary = OP_STAR; break;
            case OP_DIVASS: binary = OP_DIV; break;
            case OP_MODASS: binary = OP_MOD; break;
            case OP_BANDASS: binary = OP_AMP; break;
            case OP_BORASS: binary = OP_BOR; break;
            case OP_XORASS: binary = OP_XOR; break;
            case OP_LSHIFTASS: binary = OP_LSHIFT; break;
            case OP_RSHIFTASS: binary = OP_RSHIFT; break;
            default: throw std::runtime_error("unknown assignment operator");
            }
            TypeId result = builtin_binary(binary, an, bn);
            if (pointer(result) != pointer(a.type) || types[a.type].kind == TypeKind::Named)
                throw std::runtime_error("invalid compound assignment conversion");
        }
        r.type = a.type; r.category = ValueCategory::Lvalue;
        return r;
    }
    r.type = builtin_binary(op, an, bn); return r;
}
} }
