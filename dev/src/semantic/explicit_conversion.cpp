#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Conversion Analyzer::explicit_builtin_conversion(Expression x, TypeId to, ETokenType op, ScopeId s, NodeId operand)
{
    auto target = types[to];
    bool cstyle = op == OP_LPAREN;
    bool reinterpret = op == KW_REINTERPET_CAST || cstyle;
    bool cv_cast = op == KW_CONST_CAST;
    Conversion c; c.target = to; c.rank = 2; c.kind = Conversion::Kind::Explicit;
    c.constant_forbidden = op == KW_REINTERPET_CAST;
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        unsigned added = 0;
        bool compatible = (cv_cast || cstyle) ? similar_type(x.type, target.child) : qualification(x.type, target.child, added);
        if (!cv_cast && op != KW_REINTERPET_CAST && !(types[x.type].cv & ~types[target.child].cv)) {
            if (derived_from(x.type, target.child)) {
                compatible = true; c.derived = true;
                c.adjustment = base_steps(x.type, types[target.child].entity);
                if (!cstyle) check_base_access(x.type, target.child, s);
            } else if (derived_from(target.child, x.type)) {
                compatible = true; c.derived = true;
                if (!cstyle) check_base_access(target.child, x.type, s);
            }
        }
        if (op == KW_REINTERPET_CAST && x.category != ValueCategory::Prvalue &&
            !(types[x.type].cv & ~types[target.child].cv)) compatible = true;
        if (!compatible) throw std::runtime_error("invalid reference cast");
        if (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue && !(types[target.child].cv & 1))
            throw std::runtime_error("invalid lvalue cast");
        if (cv_cast && (x.category == ValueCategory::Prvalue || types[x.type].kind == TypeKind::Function))
            throw std::runtime_error("invalid const reference cast");
        c.reference = true; return c;
    }
    if (fundamental(to, FT_VOID) && !cv_cast && op != KW_REINTERPET_CAST) {
        if (x.form == ExpressionForm::Overload) throw std::runtime_error("discarded unresolved overload");
        c.kind = Conversion::Kind::Discarded; return c;
    }
    if (!cv_cast && op != KW_REINTERPET_CAST) {
        Conversion standard = fundamental(to, FT_BOOL) ? (operand ? boolean_conversion(operand) : boolean_conversion_value(x)) : (operand ? conversion(operand,to) : conversion_value(x,to));
        if (standard.valid()) {
            if ((cstyle && standard.derived) || (arithmetic(x.type) && arithmetic(to))) standard.kind = Conversion::Kind::Explicit;
            return standard;
        }
    }
    TypeId from = decay(x.type);
    if (cv_cast) {
        if (!pointer(from) || !pointer(to) || types[types[from].child].kind == TypeKind::Function || !similar_type(from, to))
            throw std::runtime_error("invalid const cast");
        return c;
    }
    bool enum_cast = (integral(from) && integral(to)) || (arithmetic(from) && integral(to));
    bool pointer_cast = false;
    if (pointer(from) && pointer(to)) {
        TypeId a = types[from].child, b = types[to].child;
        bool preserves_cv = !(types[a].cv & ~types[b].cv);
        pointer_cast = (cstyle || preserves_cv) && (reinterpret ||
            (fundamental(a, FT_VOID) && types[b].kind != TypeKind::Function) || derived_from(b, a));
        if (pointer_cast && op != KW_REINTERPET_CAST && derived_from(b, a)) {
            c.derived = true;
            if (!cstyle) check_base_access(b, a, s);
        }
    }
    bool integer_pointer = reinterpret && ((pointer(from) && integral(to) && width(to) >= 64) || (integral(from) && pointer(to)));
    if ((enum_cast && op != KW_REINTERPET_CAST) || pointer_cast || integer_pointer ||
        (op == KW_REINTERPET_CAST && integral(from) && from == types.unqualified(to))) {
        c.constant_forbidden |= integer_pointer || (pointer_cast && !c.derived);
        return c;
    }
    throw std::runtime_error("invalid explicit cast");
}
} }
