#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::casts_away_qualifiers(TypeId from, TypeId to)
{
    // Compare qualification decompositions, ignoring the terminal type and
    // the member aspect of pointers ([expr.const.cast]/8-11). Adding a deep
    // qualifier also requires const on each intervening destination pointer.
    bool intermediate_const = true;
    for (;;) {
        auto a = types[from], b = types[to];
        if ((a.cv & ~b.cv & 3) || ((a.cv ^ b.cv) & 4) ||
            ((a.cv ^ b.cv) & 3 && !intermediate_const)) return true;
        bool ap = a.kind == TypeKind::Pointer || a.kind == TypeKind::MemberPointer;
        bool bp = b.kind == TypeKind::Pointer || b.kind == TypeKind::MemberPointer;
        if (a.kind == TypeKind::Array && b.kind == TypeKind::Array) {
            from = a.child; to = b.child; continue;
        }
        if (!ap || !bp) return false;
        intermediate_const = intermediate_const && (b.cv & 1);
        from = a.child; to = b.child;
    }
}
Conversion Analyzer::explicit_builtin_conversion(Expression x, TypeId to, ETokenType op, ScopeId s, NodeId operand)
{
    to = types.signature(to);
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
    if (reinterpret && !ref && !fundamental(to,FT_VOID) && (vector_kind(types[x.type].kind) || vector_kind(target.kind))) {
        auto representation_type = [&](TypeId type) {
            return vector_kind(types[type].kind) || (types[type].kind == TypeKind::Fundamental && integral(type));
        };
        if (representation_type(x.type) && representation_type(to) && size(x.type) == size(to)) {
            c.kind = Conversion::Kind::Representation; c.constant_forbidden = true;
            return c;
        }
        return invalid();
    }
    bool bit_field = field_fact(x.entity).bit_field;
    if (ref && bit_field && (cv_cast || op == KW_REINTERPET_CAST)) return invalid();
    // [expr.cast]/4 selects const_cast before static_cast/reinterpret_cast.
    // This is an identity-preserving conversion, including nested pointers
    // and pointers to data members, and remains usable in constant evaluation.
    if (!ref && (cv_cast || cstyle)) {
        auto from = decay(x.type);
        bool cv_pointer = pointer(from) && pointer(to) && types[target.child].kind != TypeKind::Function;
        bool cv_member = types[from].kind == TypeKind::MemberPointer && target.kind == TypeKind::MemberPointer &&
            types[target.child].kind != TypeKind::Function;
        if ((cv_pointer || cv_member) && similar_type(from,to)) return c;
        if (cv_cast) return invalid();
    }
    if ((ref || class_value(x.type)) && !cv_cast && op != KW_REINTERPET_CAST && !fundamental(to,FT_VOID)) {
        Expression value = x;
        if (ref && bit_field && target.kind == TypeKind::RRef) {
            value.category = ValueCategory::Prvalue; value.entity = 0;
        }
        bool related = ref && (types.unqualified(value.type) == types.unqualified(target.child) ||
            derived_from(value.type,target.child) || derived_from(target.child,value.type));
        Conversion selected = standard_conversion(value,to,operand);
        // [expr.static.cast]/2-3 binds a related glvalue directly. In
        // particular an lvalue-to-rvalue-reference cast is not an implicit
        // reference conversion and must not select a user conversion first.
        if (!selected.valid() && !related)
            selected = direct_initialization_conversion(value,to,operand);
        if (selected.valid() && (selected.kind == Conversion::Kind::User || (ref && (!related || bit_field)))) {
            if (bit_field && ref) selected.temporary = true;
            return selected;
        }
        if (ref && bit_field) return invalid();
    }
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef) {
        unsigned added = 0;
        bool compatible = (cv_cast || cstyle) ? similar_type(x.type, target.child) : qualification(x.type, target.child, added);
        if (!cv_cast && op != KW_REINTERPET_CAST && (cstyle || !casts_away_qualifiers(x.type,target.child))) {
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
        if (reinterpret && (!compatible || op == KW_REINTERPET_CAST) && x.category != ValueCategory::Prvalue &&
            (cstyle || !casts_away_qualifiers(x.type,target.child))) {
            compatible = true; c.constant_forbidden = true;
        }
        if (!compatible) return invalid();
        if (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue && !(types[target.child].cv & 1))
            return invalid();
        if (cv_cast && (types[x.type].kind == TypeKind::Function ||
            (target.kind == TypeKind::LRef && x.category != ValueCategory::Lvalue) ||
            (x.category == ValueCategory::Prvalue && !class_value(x.type)))) return invalid();
        c.reference = true; return c;
    }
    if (fundamental(to, FT_VOID) && !cv_cast && op != KW_REINTERPET_CAST) {
        if (x.form == ExpressionForm::Overload) return invalid();
        c.kind = Conversion::Kind::Discarded; return c;
    }
    if (!cv_cast && op != KW_REINTERPET_CAST) {
        if (complex_type(x.type) && arithmetic(to) && types[to].kind != TypeKind::Named) return c;
        Conversion standard = fundamental(to, FT_BOOL) ? (operand ? boolean_conversion(operand) : boolean_conversion_value(x)) : (operand ? conversion(operand,to) : conversion_value(x,to));
        if (standard.valid()) {
            if ((cstyle && standard.derived) || (arithmetic(x.type) && arithmetic(to))) standard.kind = Conversion::Kind::Explicit;
            return standard;
        }
    }
    TypeId from = decay(x.type);
    if (types[from].kind == TypeKind::MemberPointer && target.kind == TypeKind::MemberPointer) {
        unsigned added = 0;
        bool member_types = qualification(types[from].child,target.child,added) ||
            (cstyle && similar_type(types[from].child,target.child));
        auto derived = entities[types[from].entity].type, base = entities[target.entity].type;
        if (member_types && op != KW_REINTERPET_CAST && downcast(base,derived)) {
            if (!cstyle && !base_accessible(target.entity,types[from].entity,s)) return invalid();
            c.derived = true; c.adjustment = base_steps(base,types[from].entity); return c;
        }
        if (member_types && op != KW_REINTERPET_CAST && downcast(derived,base)) {
            if (!cstyle && !base_accessible(types[from].entity,target.entity,s)) return invalid();
            c.derived = true; c.adjustment = inverse(derived,base); return c;
        }
        if (op == KW_REINTERPET_CAST && member_types) return c;
        return invalid();
    }
    bool enum_cast = (integral(from) && integral(to)) || (arithmetic(from) && integral(to));
    bool pointer_cast = false;
    if (pointer(from) && pointer(to)) {
        TypeId a = types[from].child, b = types[to].child;
        if (op != KW_REINTERPET_CAST && derived_from(a,b)) {
            auto path = base_path(a,types[b].entity);
            if (!path || base_adjustments[path].ambiguous) return invalid();
        }
        bool preserves_cv = !casts_away_qualifiers(a,b);
        pointer_cast = (cstyle || preserves_cv) && (reinterpret ||
            (fundamental(a, FT_VOID) && types[b].kind != TypeKind::Function) || derived_from(b, a));
        if (pointer_cast && op != KW_REINTERPET_CAST && derived_from(a,b)) {
            c.derived = true; c.adjustment = base_steps(a,types[b].entity);
            if (!cstyle && !base_accessible(types[a].entity,types[b].entity,s)) return invalid();
        }
        if (pointer_cast && op != KW_REINTERPET_CAST && derived_from(b, a)) {
            if (!downcast(b,a)) return invalid();
            c.derived = true; c.adjustment = inverse(b,a);
            if (!cstyle && !base_accessible(types[b].entity,types[a].entity,s)) return invalid();
        }
    }
    if (reinterpret && address_value(from) && address_value(to) && (block_pointer(from) || block_pointer(to))) pointer_cast = true;
    bool integer_pointer = reinterpret && ((address_value(from) && integral(to) && width(to) >= 64) || (integral(from) && address_value(to)));
    if ((enum_cast && op != KW_REINTERPET_CAST) || pointer_cast || integer_pointer ||
        (op == KW_REINTERPET_CAST && integral(from) && from == types.unqualified(to))) {
        c.constant_forbidden |= integer_pointer || (pointer_cast && !c.derived);
        return c;
    }
    return invalid();
}
} }
