#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::fundamental(TypeId t, EFundamentalType f) const
{
    return t && types[t].kind == TypeKind::Fundamental && types[t].fundamental == f;
}
bool Analyzer::pointer(TypeId t) const { return types[t].kind == TypeKind::Pointer; }
TypeId Analyzer::value_type(TypeId t)
{
    return types[t].kind == TypeKind::LRef || types[t].kind == TypeKind::RRef ? types[t].child : t;
}
TypeId Analyzer::decay(TypeId t)
{
    if (!t) throw std::runtime_error("unresolved overload in value context");
    t = value_type(t);
    if (types[t].kind == TypeKind::Array) return types.compound(TypeKind::Pointer, types[t].child);
    if (types[t].kind == TypeKind::Function) return types.compound(TypeKind::Pointer, t);
    return types.unqualified(t);
}
bool Analyzer::arithmetic(TypeId t) const
{
    return (integral(t) && !scoped_enum(t)) || (types[t].kind == TypeKind::Fundamental &&
        types[t].fundamental >= FT_FLOAT && types[t].fundamental <= FT_LONG_DOUBLE);
}
TypeId Analyzer::promote(TypeId t)
{
    if (types[t].kind == TypeKind::Named && integral(t) && !scoped_enum(t)) t = entities[types[t].entity].underlying;
    return integral(t) && width(t) < 32 ? types.fundamental(FT_INT) : types.unqualified(t);
}
TypeId Analyzer::arithmetic_type(TypeId a, TypeId b)
{
    if (!arithmetic(a) || !arithmetic(b)) throw std::runtime_error("arithmetic operands required");
    a = promote(a); b = promote(b);
    if (!integral(a) || !integral(b)) {
        if (fundamental(a, FT_LONG_DOUBLE) || fundamental(b, FT_LONG_DOUBLE)) return types.fundamental(FT_LONG_DOUBLE);
        if (fundamental(a, FT_DOUBLE) || fundamental(b, FT_DOUBLE)) return types.fundamental(FT_DOUBLE);
        return types.fundamental(FT_FLOAT);
    }
    if (width(a) != width(b)) return width(a) > width(b) ? a : b;
    if (is_unsigned(a) != is_unsigned(b)) return is_unsigned(a) ? a : b;
    return types[a].fundamental > types[b].fundamental ? a : b;
}
bool Analyzer::null_constant(NodeId n)
{
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (fundamental(expressions[n].type, FT_NULLPTR_T)) return true;
    if (ast[n].kind != Kind::Literal || !integral(expressions[n].type)) return false;
    const syntax::LiteralValue& lit = ast.literals[ast[n].literal];
    if (lit.kind == LiteralKind::string || lit.kind == LiteralKind::character) return false;
    Constant v = evaluate(n, facts[n].scope);
    return v.valid && !v.bits;
}
bool Analyzer::qualification(TypeId from, TypeId to, unsigned& added, bool intermediate_const)
{
    Type a = types[from], b = types[to];
    if (a.kind != b.kind || (a.cv & ~b.cv)) return false;
    if (a.cv != b.cv) {
        if (!intermediate_const) return false;
        added |= b.cv & ~a.cv;
    }
    if (a.kind == TypeKind::Pointer || a.kind == TypeKind::Array) {
        if (a.kind == TypeKind::Array && a.bound != b.bound) return false;
        return qualification(a.child, b.child, added,
            intermediate_const && (a.kind == TypeKind::Array || (b.cv & 1)));
    }
    return types.unqualified(from) == types.unqualified(to);
}
Conversion Analyzer::conversion(NodeId n, TypeId to)
{
    ++conversion_work;
    Conversion c; c.target = to;
    Expression x = expressions[n];
    Type target = types[to];
    bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
    if (x.form == ExpressionForm::Overload) {
        TypeId ft = ref || pointer(to) || target.kind == TypeKind::MemberPointer ? target.child : to;
        if (types[ft].kind != TypeKind::Function) return c;
        for (EntityId e : candidates(x.entity)) {
            if (entities[e].type != ft) continue;
            if (target.kind == TypeKind::MemberPointer && scopes[entities[e].owner].entity != target.entity) continue;
            if (c.function) return Conversion();
            c.function = e;
        }
        if (c.function) { c.rank = 0; c.reference = ref; c.preference = target.kind == TypeKind::RRef; }
        return c;
    }
    TypeId from = x.type;
    if (!from) return c;
    if (ref) {
        unsigned added = 0;
        bool function_lvalue = types[from].kind == TypeKind::Function;
        bool category = target.kind == TypeKind::LRef ? x.category == ValueCategory::Lvalue : x.category != ValueCategory::Lvalue;
        if (category && derived_from(from, target.child) && !(types[from].cv & ~types[target.child].cv)) {
            c.rank = 2; c.reference = true; c.derived = true; c.qualification = types[target.child].cv & ~types[from].cv; return c;
        }
        if ((category || function_lvalue) && qualification(from, target.child, added)) {
            c.rank = 0; c.reference = true; c.qualification = added; c.preference = function_lvalue && target.kind == TypeKind::RRef; return c;
        }
        // A const, nonvolatile lvalue reference may bind a converted temporary.
        if ((target.kind == TypeKind::LRef && types[target.child].cv != 1) ||
            (types[from].cv & ~types[target.child].cv) ||
            (target.kind == TypeKind::RRef && types.unqualified(from) == types.unqualified(target.child))) return c;
        c = conversion(n, types.unqualified(target.child));
        c.target = to; c.reference = true; c.qualification = types[target.child].cv;
        c.temporary = types.unqualified(from) != types.unqualified(target.child);
        c.preference = target.kind == TypeKind::LRef;
        return c;
    }
    to = types.unqualified(to);
    from = decay(from);
    if (to == from) { c.rank = 0; return c; }
    if ((pointer(to) || fundamental(to, FT_NULLPTR_T)) && null_constant(n)) { c.rank = 2; return c; }
    if (fundamental(to, FT_BOOL) && pointer(from)) { c.rank = 3; return c; }
    if (pointer(from) && pointer(to)) {
        unsigned added = 0;
        if (qualification(types[from].child, types[to].child, added)) {
            c.rank = 0; c.qualification = added; return c;
        }
        Type a = types[types[from].child], b = types[types[to].child];
        if (derived_from(types[from].child, types[to].child) && !(a.cv & ~b.cv)) {
            c.rank = 2; c.derived = true; c.qualification = b.cv & ~a.cv; return c;
        }
        if (fundamental(types[to].child, FT_VOID) && a.kind != TypeKind::Function && !(a.cv & ~b.cv)) {
            c.rank = 2; c.qualification = b.cv & ~a.cv; return c;
        }
    }
    if (arithmetic(from) && arithmetic(to) && types[to].kind != TypeKind::Named) {
        c.rank = promote(from) == to ? 1 : 2;
        return c;
    }
    return c;
}
void Analyzer::select_function(NodeId n, EntityId e)
{
    expressions[n].entity = e;
    expressions[n].form = ExpressionForm::Ordinary;
    expressions[n].type = entities[e].type;
    facts[n].type = entities[e].member_info ? members[entities[e].member_info].call_type : entities[e].type;
    facts[n].entity = e;
    demand_member(e);
    demand_specialization(e);
    if (ast[n].kind == Kind::Parenthesized || (ast[n].kind == Kind::Unary && ast[n].op == OP_AMP)) {
        select_function(ast[n].first, e);
        if (ast[n].kind == Kind::Unary) {
            expressions[n].type = entities[e].member_info && !entities[e].is_static ?
                types.member_pointer(scopes[entities[e].owner].entity, entities[e].type) : types.compound(TypeKind::Pointer, entities[e].type);
            expressions[n].category = ValueCategory::Prvalue;
            facts[n].type = expressions[n].type;
        }
    }
}
void Analyzer::apply_conversion(NodeId n, Conversion c)
{
    if (c.function) select_function(n, c.function);
    if (expressions[n].entity) demand_specialization(expressions[n].entity);
    TypeId target = types.unqualified(c.target);
    if (ast[n].kind == Kind::Literal && (pointer(target) || fundamental(target, FT_NULLPTR_T)) && null_constant(n)) facts[n].type = target;
}
void Analyzer::require_conversion(NodeId n, TypeId target)
{
    Conversion c = conversion(n, target);
    if (!c.valid()) throw std::runtime_error("invalid implicit conversion");
    apply_conversion(n, c);
    expressions[n].incoming = conversions.size();
    conversions.push_back(c);
}
void Analyzer::initialize(NodeId n, TypeId target, ScopeId s)
{
    if (ast[n].kind == Kind::Initializer) { initialize(ast[n].first, target, s); return; }
    if (ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::BracedInit) {
        if (types[target].kind == TypeKind::Array) {
            for (NodeId c = ast[n].first; c; c = ast[c].next) initialize(c, types[target].child, s);
            facts[n].type = target; expressions[n].type = target;
            expressions[n].category = ValueCategory::Lvalue; expressions[n].ready = true;
            return;
        }
        if (!ast[n].first) { facts[n].type = target; return; }
        if (ast[n].first != ast[n].last) throw std::runtime_error("too many scalar initializers");
        initialize(ast[n].first, target, s);
        facts[n].type = target;
        return;
    }
    expression(n, s);
    require_conversion(n, target);
}
} }
