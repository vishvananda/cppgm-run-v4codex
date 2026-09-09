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
    if (types[a.type].kind == TypeKind::Named) {
        std::vector<NodeId> args(1, operand);
        if (ast[n].kind == Kind::Postfix) args.push_back(0);
        if (operator_expression(n, s, op, args, r)) return r;
    }
    if (op == OP_AMP) {
        if (field_fact(a.entity).bit_field) throw std::runtime_error("address of bit-field");
        if (a.form == ExpressionForm::Overload) return a;
        if (a.entity) demand_specialization(a.entity);
        if (a.category == ValueCategory::Prvalue) throw std::runtime_error("address of rvalue");
        if (a.entity && entities[a.entity].member_info && !entities[a.entity].is_static) {
            r.type = types.member_pointer(scopes[entities[a.entity].owner].entity, entities[a.entity].type);
            demand_member(a.entity);
        } else r.type = types.compound(TypeKind::Pointer, a.type);
        return r;
    }
    TypeId t = decay(a.type);
    if (op == OP_STAR) {
        if (!pointer(t) || fundamental(types[t].child, FT_VOID)) throw std::runtime_error("invalid dereference");
        record_conversion(r, operand, conversion(operand, t));
        r.type = types[t].child; r.category = ValueCategory::Lvalue; return r;
    }
    if (op == OP_INC || op == OP_DEC) {
        modifiable(operand);
        if ((!arithmetic(a.type) && !object_pointer(t)) || types[a.type].kind == TypeKind::Named ||
            (op == OP_DEC && fundamental(a.type, FT_BOOL))) throw std::runtime_error("invalid increment operand");
        r.type = ast[n].kind == Kind::Postfix ? types.unqualified(a.type) : a.type;
        r.category = ast[n].kind == Kind::Postfix ? ValueCategory::Prvalue : ValueCategory::Lvalue;
        TypeId promoted = pointer(t) ? t : promote_expression(operand);
        if (ast[n].kind != Kind::Postfix) r.entity = a.entity;
        record_conversion(r, operand, conversion(operand, promoted));
        Conversion store; store.target = a.type; store.rank = 0;
        record_conversion(r, 0, store);
        return r;
    }
    if (op == OP_LNOT) { record_conversion(r, operand, boolean_conversion(operand)); r.type = types.fundamental(FT_BOOL); return r; }
    if (!(op == OP_PLUS && pointer(t)) && (!arithmetic(t) || (op == OP_COMPL && !integral(t))))
        throw std::runtime_error("invalid unary arithmetic");
    r.type = promote_expression(operand);
    record_conversion(r, operand, conversion(operand, r.type));
    return r;
}
TypeId Analyzer::builtin_binary(ETokenType op, NodeId an, NodeId bn, Expression& r)
{
    TypeId a = decay(expressions[an].type), b = decay(expressions[bn].type);
    bool compare = op == OP_EQ || op == OP_NE || op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE;
    if (op == OP_LAND || op == OP_LOR) {
        record_conversion(r, an, boolean_conversion(an)); record_conversion(r, bn, boolean_conversion(bn));
        return types.fundamental(FT_BOOL);
    }
    if (compare) {
        bool equality = op == OP_EQ || op == OP_NE;
        TypeId common = 0;
        if (a == b && (scoped_enum(a) || (equality && fundamental(a, FT_NULLPTR_T)))) common = a;
        if (pointer(a) && pointer(b)) common = composite_pointer(a, b);
        if (equality && pointer(a) && null_constant(bn)) common = a;
        if (equality && pointer(b) && null_constant(an)) common = b;
        if (common) {
            if (!equality && pointer(common) && types[types[common].child].kind == TypeKind::Function)
                throw std::runtime_error("ordered function pointer comparison");
            record_conversion(r, an, conversion(an, common)); record_conversion(r, bn, conversion(bn, common));
            return types.fundamental(FT_BOOL);
        }
    }
    if (op == OP_PLUS || op == OP_MINUS) {
        TypeId left = 0, right = 0, result = 0;
        if (object_pointer(a) && integral(b) && !scoped_enum(b)) { left = a; right = promote_expression(bn); result = a; }
        if (op == OP_PLUS && object_pointer(b) && integral(a) && !scoped_enum(a)) { left = promote_expression(an); right = b; result = b; }
        if (op == OP_MINUS && object_pointer(a) && object_pointer(b) &&
            types.unqualified(types[a].child) == types.unqualified(types[b].child)) {
            left = right = composite_pointer(a, b); result = types.fundamental(FT_LONG_INT);
        }
        if (result) {
            record_conversion(r, an, conversion(an, left)); record_conversion(r, bn, conversion(bn, right));
            return result;
        }
    }
    TypeId ap = promote_expression(an), bp = promote_expression(bn);
    TypeId result = arithmetic_type(ap, bp);
    if (op == OP_MOD || op == OP_AMP || op == OP_BOR || op == OP_XOR || op == OP_LSHIFT || op == OP_RSHIFT) {
        if (!integral(a) || !integral(b)) throw std::runtime_error("integral operands required");
    }
    bool shift = op == OP_LSHIFT || op == OP_RSHIFT;
    Conversion left = conversion(an, shift ? ap : result), right = conversion(bn, shift ? bp : result);
    bool offset = op == OP_PLUS || op == OP_MINUS;
    left.fold_widen = offset && facts[bn].value && (ast[bn].kind == Kind::Sizeof || expressions[bn].form == ExpressionForm::ConstantQuery);
    right.fold_widen = offset && facts[an].value && (ast[an].kind == Kind::Sizeof || expressions[an].form == ExpressionForm::ConstantQuery);
    record_conversion(r, an, left);
    record_conversion(r, bn, right);
    if (shift) return ap;
    return compare ? types.fundamental(FT_BOOL) : result;
}
Expression Analyzer::binary_expression(NodeId n, ScopeId s)
{
    NodeId an = ast[n].first, bn = ast[an].next;
    Expression a = expression(an, s), b = expression(bn, s), r;
    ETokenType op = ast[n].op;
    if (ast[n].kind != Kind::Conditional && (types[a.type].kind == TypeKind::Named || types[b.type].kind == TypeKind::Named) &&
        operator_expression(n, s, op, {an, bn}, r)) return r;
    if (ast[n].kind == Kind::Conditional) {
        NodeId cn = ast[bn].next;
        Expression c = expression(cn, s);
        record_conversion(r, an, boolean_conversion(an));
        TypeId common_class = 0;
        if (!(types[b.type].cv & ~types[c.type].cv) && derived_from(b.type, c.type)) common_class = c.type;
        else if (!(types[c.type].cv & ~types[b.type].cv) && derived_from(c.type, b.type)) common_class = b.type;
        if ((b.type == c.type || common_class) && b.category == c.category && b.category != ValueCategory::Prvalue) {
            r.type = common_class ? common_class : b.type; r.category = b.category;
            r.entity = b.entity == c.entity ? b.entity : 0;
            TypeId ref = types.compound(b.category == ValueCategory::Lvalue ? TypeKind::LRef : TypeKind::RRef, r.type);
            record_conversion(r, bn, conversion(bn, ref)); record_conversion(r, cn, conversion(cn, ref));
            return r;
        }
        TypeId bt = decay(b.type), ct = decay(c.type);
        if (bt == ct) r.type = bt;
        else if (pointer(bt) && pointer(ct)) r.type = composite_pointer(bt, ct);
        else if (pointer(bt) && null_constant(cn)) r.type = bt;
        else if (pointer(ct) && null_constant(bn)) r.type = ct;
        else r.type = arithmetic_type(bt, ct);
        if (!r.type) throw std::runtime_error("incompatible conditional operands");
        record_conversion(r, bn, conversion(bn, r.type)); record_conversion(r, cn, conversion(cn, r.type));
        return r;
    }
    if (op == OP_COMMA) {
        if (a.form == ExpressionForm::Overload || b.form == ExpressionForm::Overload)
            throw std::runtime_error("unresolved comma operand");
        return value_fact(b);
    }
    if (ast[n].kind == Kind::Assignment) {
        modifiable(an);
        if (op == OP_ASS) {
            Conversion left; left.target = types.compound(TypeKind::LRef, a.type); left.reference = true; left.rank = 0;
            record_conversion(r, an, left);
            record_conversion(r, bn, conversion(bn, a.type));
            auto index = expressions[bn].incoming;
            Conversion applied = conversions[index];
            apply_conversion(bn, applied); conversions[index] = applied;
        }
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
            TypeId result = builtin_binary(binary, an, bn, r);
            if (pointer(result) != pointer(a.type) || types[a.type].kind == TypeKind::Named)
                throw std::runtime_error("invalid compound assignment conversion");
            Conversion store; store.target = a.type; store.rank = types.unqualified(a.type) == result ? 0 : 2;
            record_conversion(r, 0, store);
        }
        r.type = a.type; r.category = ValueCategory::Lvalue; r.entity = a.entity;
        return r;
    }
    r.type = builtin_binary(op, an, bn, r); return r;
}
} }
