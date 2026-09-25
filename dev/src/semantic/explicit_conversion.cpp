#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
Conversion Analyzer::explicit_builtin_conversion(Expression x, TypeId to, ETokenType op, ScopeId s, NodeId operand)
{
    auto target = types[to];
    auto invalid = [&]() { Conversion c; c.target = to; return c; };
    auto inverse = [&](TypeId derived, TypeId base) {
        auto path = base_steps(derived,types[base].entity);
        if (!base_adjustments[path].total) return 0u;
        if (auto old = base_adjustments[path].inverse) return old;
        BaseAdjustment reverse = base_adjustments[path]; reverse.total = 0-reverse.total;
        auto result = base_adjustments.size(); base_adjustments[path].inverse = result;
        base_adjustments.push_back(reverse); return unsigned(result);
    };
    auto downcast = [&](TypeId derived, TypeId base) {
        auto path = base_path(derived,types[base].entity);
        if (!path || !base_adjustments[path].edge || base_adjustments[path].ambiguous) return false;
        for (auto p = path; p; p = base_adjustments[p].next)
            if (bases[base_adjustments[p].edge].virtual_base) return false;
        return true;
    };
    bool cstyle = op == OP_LPAREN;
    bool reinterpret = op == KW_REINTERPET_CAST || cstyle;
    bool cv_cast = op == KW_CONST_CAST;
    Conversion c; c.target = to; c.rank = 2; c.kind = Conversion::Kind::Explicit;
    c.constant_forbidden = op == KW_REINTERPET_CAST;
    // Ordinary expressions and substitution queries use the same selected
    // direct-initialization sequence, including conversion functions and
    // reference-bound scalar temporaries ([expr.static.cast]/4).
    bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
    bool bit_field = field_fact(x.entity).bit_field;
    if (ref && bit_field && (cv_cast || op == KW_REINTERPET_CAST)) return invalid();
    if ((ref || class_value(x.type)) && !cv_cast && op != KW_REINTERPET_CAST && !fundamental(to,FT_VOID)) {
        Expression value = x;
        if (ref && bit_field && target.kind == TypeKind::RRef) {
            value.category = ValueCategory::Prvalue; value.entity = 0;
        }
        Conversion selected = standard_conversion(value,to,operand);
        if (!selected.valid() && class_value(value.type))
            selected = conversion_function_value(value,to,true);
        bool related = ref && (types.unqualified(value.type) == types.unqualified(target.child) ||
            derived_from(value.type,target.child) || derived_from(target.child,value.type));
        if (ref && !related && !selected.valid()) selected = conversion_value(value,to,true,operand);
        if (selected.valid() && (selected.kind == Conversion::Kind::User || (ref && (!related || bit_field)))) {
            if (bit_field && ref) selected.temporary = true;
            return selected;
        }
        if (ref && bit_field) return invalid();
    }
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        unsigned added = 0;
        bool compatible = (cv_cast || cstyle) ? similar_type(x.type, target.child) : qualification(x.type, target.child, added);
        if (!cv_cast && op != KW_REINTERPET_CAST && !(types[x.type].cv & ~types[target.child].cv)) {
            if (derived_from(x.type, target.child)) {
                auto path = base_path(x.type,types[target.child].entity);
                if (!path || base_adjustments[path].ambiguous) return invalid();
                compatible = true; c.derived = true;
                c.adjustment = base_steps(x.type, types[target.child].entity);
                if (!cstyle && !base_accessible(types[x.type].entity,types[target.child].entity,s)) return invalid();
            } else if (derived_from(target.child, x.type)) {
                if (!downcast(target.child,x.type)) return invalid();
                compatible = true; c.derived = true; c.adjustment = inverse(target.child,x.type);
                if (!cstyle && !base_accessible(types[target.child].entity,types[x.type].entity,s)) return invalid();
            }
        }
        if (op == KW_REINTERPET_CAST && x.category != ValueCategory::Prvalue &&
            !(types[x.type].cv & ~types[target.child].cv)) compatible = true;
        if (!compatible) return invalid();
        if (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue && !(types[target.child].cv & 1))
            return invalid();
        if (cv_cast && (x.category == ValueCategory::Prvalue || types[x.type].kind == TypeKind::Function))
            return invalid();
        c.reference = true; return c;
    }
    if (fundamental(to, FT_VOID) && !cv_cast && op != KW_REINTERPET_CAST) {
        if (x.form == ExpressionForm::Overload) return invalid();
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
            return invalid();
        return c;
    }
    bool enum_cast = (integral(from) && integral(to)) || (arithmetic(from) && integral(to));
    bool pointer_cast = false;
    if (pointer(from) && pointer(to)) {
        TypeId a = types[from].child, b = types[to].child;
        if (op != KW_REINTERPET_CAST && derived_from(a,b)) {
            auto path = base_path(a,types[b].entity);
            if (!path || base_adjustments[path].ambiguous) return invalid();
        }
        bool preserves_cv = !(types[a].cv & ~types[b].cv);
        pointer_cast = (cstyle || preserves_cv) && (reinterpret ||
            (fundamental(a, FT_VOID) && types[b].kind != TypeKind::Function) || derived_from(b, a));
        if (pointer_cast && op != KW_REINTERPET_CAST && derived_from(b, a)) {
            if (!downcast(b,a)) return invalid();
            c.derived = true; c.adjustment = inverse(b,a);
            if (!cstyle && !base_accessible(types[b].entity,types[a].entity,s)) return invalid();
        }
    }
    bool integer_pointer = reinterpret && ((pointer(from) && integral(to) && width(to) >= 64) || (integral(from) && pointer(to)));
    if ((enum_cast && op != KW_REINTERPET_CAST) || pointer_cast || integer_pointer ||
        (op == KW_REINTERPET_CAST && integral(from) && from == types.unqualified(to))) {
        c.constant_forbidden |= integer_pointer || (pointer_cast && !c.derived);
        return c;
    }
    return invalid();
}
} }
