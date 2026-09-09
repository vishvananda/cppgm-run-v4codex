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
    if (fundamental(t, FT_WCHAR_T)) return types.fundamental(FT_INT);
    if (fundamental(t, FT_CHAR32_T)) return types.fundamental(FT_UNSIGNED_INT);
    return integral(t) && width(t) < 32 ? types.fundamental(FT_INT) : types.unqualified(t);
}
TypeId Analyzer::promote_expression(NodeId n)
{
    auto expression = expressions[n];
    auto field = field_fact(expression.entity);
    TypeId t = decay(expression.type);
    if (field.bit_field && types[t].kind != TypeKind::Named) {
        if (field.width < 32 || (field.width == 32 && !is_unsigned(t))) return types.fundamental(FT_INT);
        if (field.width == 32) return types.fundamental(FT_UNSIGNED_INT);
    }
    return promote(t);
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
    // Rank is distinct from width: both long and long long are 64-bit on LP64.
    unsigned ar = types[a].fundamental, br = types[b].fundamental;
    if (is_unsigned(a)) ar -= FT_UNSIGNED_CHAR;
    if (is_unsigned(b)) br -= FT_UNSIGNED_CHAR;
    if (is_unsigned(a) == is_unsigned(b)) return ar >= br ? a : b;
    TypeId u = is_unsigned(a) ? a : b, s = is_unsigned(a) ? b : a;
    unsigned ur = is_unsigned(a) ? ar : br, sr = is_unsigned(a) ? br : ar;
    if (ur >= sr) return u;
    if (width(s) > width(u)) return s;
    return types.fundamental(EFundamentalType(types[s].fundamental + FT_UNSIGNED_CHAR));
}
bool Analyzer::object_pointer(TypeId t)
{
    if (!pointer(t)) return false;
    Type child = types[types[t].child];
    if (fundamental(types[t].child, FT_VOID) || child.kind == TypeKind::Function) return false;
    if (child.kind == TypeKind::Array && !child.bound) return false;
    if (child.kind == TypeKind::Named && !entities[child.entity].complete) return false;
    return true;
}
TypeId Analyzer::composite_pointer(TypeId a, TypeId b)
{
    if (!pointer(a) || !pointer(b)) return 0;
    TypeId ac = types[a].child, bc = types[b].child, result = 0;
    unsigned cv = types[ac].cv | types[bc].cv;
    if (types.unqualified(ac) == types.unqualified(bc)) result = types.qualify(ac, cv);
    else if (pointer(ac) && pointer(bc)) {
        result = composite_pointer(ac, bc);
        if (result) result = types.qualify(result, cv | 1);
    } else if ((fundamental(ac, FT_VOID) && types[bc].kind != TypeKind::Function) ||
               (fundamental(bc, FT_VOID) && types[ac].kind != TypeKind::Function))
        result = types.qualify(types.fundamental(FT_VOID), cv);
    else if (derived_from(ac, bc)) result = types.qualify(bc, cv);
    else if (derived_from(bc, ac)) result = types.qualify(ac, cv);
    if (!result) return 0;
    // Recursive composition must not admit void/base conversions below the
    // first pointer level. Only qualification may differ there.
    unsigned added = 0;
    if (pointer(ac) && (!qualification(ac, result, added) || !qualification(bc, result, added))) return 0;
    return types.compound(TypeKind::Pointer, result);
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
bool Analyzer::similar_type(TypeId a, TypeId b)
{
    if (types[a].kind != types[b].kind) return false;
    if (pointer(a)) return similar_type(types[a].child, types[b].child);
    return types.unqualified(a) == types.unqualified(b);
}
Conversion Analyzer::conversion(NodeId n, TypeId to, bool user)
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
        if (field_fact(x.entity).bit_field) {
            if (target.kind != TypeKind::LRef || types[target.child].cv != 1) return c;
            c = conversion(n, types.unqualified(target.child), user);
            c.target = to; c.reference = true; c.temporary = true; return c;
        }
        unsigned added = 0;
        bool function_lvalue = types[from].kind == TypeKind::Function;
        bool category = target.kind == TypeKind::LRef ? x.category == ValueCategory::Lvalue : x.category != ValueCategory::Lvalue;
        bool const_binding = target.kind == TypeKind::LRef && types[target.child].cv == 1;
        if ((category || const_binding) && derived_from(from, target.child) && !(types[from].cv & ~types[target.child].cv)) {
            c.rank = 2; c.reference = true; c.derived = true; c.qualification = types[target.child].cv & ~types[from].cv;
            c.preference = x.category != ValueCategory::Lvalue && target.kind == TypeKind::LRef; return c;
        }
        if ((category || const_binding || function_lvalue) && qualification(from, target.child, added)) {
            c.rank = 0; c.reference = true; c.qualification = added;
            c.preference = function_lvalue ? target.kind == TypeKind::RRef : x.category != ValueCategory::Lvalue && target.kind == TypeKind::LRef; return c;
        }
        // A const, nonvolatile lvalue reference may bind a converted temporary.
        if ((target.kind == TypeKind::LRef && types[target.child].cv != 1) ||
            (types[from].cv & ~types[target.child].cv) ||
            (target.kind == TypeKind::RRef && types.unqualified(from) == types.unqualified(target.child))) return c;
        c = conversion(n, types.unqualified(target.child), user);
        c.target = to; c.reference = true; c.qualification = types[target.child].cv;
        c.temporary = types.unqualified(from) != types.unqualified(target.child);
        c.preference = target.kind == TypeKind::LRef;
        return c;
    }
    to = types.unqualified(to);
    from = decay(from);
    if (class_value(to) && class_value(from) && (to == from || derived_from(from,to)) && !empty_value(to)) {
        EntityId ctor = select_transfer(to,x.type,x.category,false);
        if (ctor) { c.function = ctor; c.kind = Conversion::Kind::Construction; c.rank = to == from ? 0 : 2; }
        return c;
    }
    if (to == from) { c.rank = 0; c.empty_copy = empty_value(to); return c; }
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
        c.rank = promote_expression(n) == to ? 1 : 2;
        return c;
    }
    if (user && types[to].kind == TypeKind::Named && entities[types[to].entity].class_info)
        return converting_constructor(n, to);
    return c;
}
void Analyzer::select_function(NodeId n, EntityId e)
{
    if (deleted_transfer(e))
        throw std::runtime_error("selected deleted member function");
    ScopeId naming = object_uses[expressions[n].object_use].naming_scope;
    TypeId object = 0;
    if (ast[n].kind == Kind::Member) {
        object = expressions[ast[n].first].type;
        if (ast[n].op == OP_ARROW) object = types[object].child;
    } else if (TypeId implicit = implicit_object_type(facts[n].scope)) object = types[implicit].child;
    check_access(e, facts[n].scope, naming, object);
    expressions[n].entity = e;
    expressions[n].form = ExpressionForm::Ordinary;
    expressions[n].type = entities[e].type;
    facts[n].type = entities[e].member_info ? members[entities[e].member_info].call_type : entities[e].type;
    facts[n].entity = e;
    if (destructor_member(e)) members[entities[e].member_info].retained_root = true;
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
void Analyzer::apply_conversion(NodeId n, Conversion& c)
{
    if (c.kind == Conversion::Kind::Construction) { materialize_conversion(n, c); return; }
    if (c.derived && c.kind != Conversion::Kind::Explicit) {
        TypeId from = expressions[n].type, to = types[c.target].child;
        if (pointer(from)) from = types[from].child;
        if (pointer(to)) to = types[to].child;
        check_base_access(from, to, facts[n].scope);
    }
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
void Analyzer::record_conversion(Expression& owner, NodeId n, Conversion c)
{
    if (!c.valid()) throw std::runtime_error("invalid operand conversion");
    if (n && c.kind == Conversion::Kind::Construction) materialize_conversion(n, c);
    // Materializing an operand can append its own constructor conversions.
    // Keep the owning operator's (bounded) operand slice contiguous.
    if (owner.count && owner.conversions + owner.count != conversions.size()) {
        std::vector<Conversion> prior(conversions.begin()+owner.conversions, conversions.begin()+owner.conversions+owner.count);
        owner.conversions = conversions.size();
        conversions.insert(conversions.end(),prior.begin(),prior.end());
    }
    if (!owner.count) owner.conversions = conversions.size();
    ++owner.count;
    if (n) {
        if (c.kind == Conversion::Kind::Construction) materialize_conversion(n, c);
        // Operand facts preserve the source-faithful dump type. The legacy
        // initializer/call adapter in apply_conversion has its own null view.
        if (c.derived && c.kind != Conversion::Kind::Explicit) {
            TypeId from = expressions[n].type;
            if (pointer(from)) from = types[from].child;
            TypeId to = types[c.target].child;
            if (pointer(to)) to = types[to].child;
            check_base_access(from, to, facts[n].scope);
        }
        if (c.function && c.kind != Conversion::Kind::Construction) select_function(n, c.function);
        if (expressions[n].entity) demand_specialization(expressions[n].entity);
        expressions[n].incoming = conversions.size();
    }
    conversions.push_back(c);
}
void Analyzer::record_call(Expression& owner, const std::vector<NodeId>& args, std::vector<Conversion>& selected)
{
    for (std::size_t j = 0; j < args.size(); ++j) if (args[j]) apply_conversion(args[j], selected[j]);
    owner.arguments = call_arguments.size(); owner.argument_count = args.size();
    owner.conversions = conversions.size(); owner.count = args.size();
    for (std::size_t j = 0; j < args.size(); ++j) if (args[j]) expressions[args[j]].incoming = conversions.size()+j;
    conversions.insert(conversions.end(), selected.begin(), selected.end());
    call_arguments.insert(call_arguments.end(), args.begin(), args.end());
}
Conversion Analyzer::boolean_conversion(NodeId n)
{
    Conversion c = conversion(n, types.fundamental(FT_BOOL));
    // Contextual bool conversion uses direct-initialization, which admits
    // nullptr_t; ordinary copy-initialization of bool still rejects it.
    if (fundamental(expressions[n].type, FT_NULLPTR_T)) {
        c.rank = 2; c.kind = Conversion::Kind::Contextual;
    }
    return c;
}
void Analyzer::initialize(NodeId n, TypeId target, ScopeId s)
{
    if (types[target].kind == TypeKind::Named && entities[types[target].entity].class_info && class_initialize(n, target, s)) return;
    if (ast[n].kind == Kind::Initializer) { initialize(ast[n].first, target, s); return; }
    if (aggregate_type(target) && (types[target].kind == TypeKind::Array || ast[n].kind == Kind::BracedInit ||
        ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::ParenInitializer)) {
        aggregate_initialization(n, target, s); return;
    }
    if (ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) {
        if (!ast[n].first) { facts[n].type = target; expressions[n].type = target; expressions[n].ready = true; return; }
        if (ast[n].first != ast[n].last) throw std::runtime_error("too many scalar initializers");
        initialize(ast[n].first, target, s);
        if (ast[n].kind == Kind::BracedInit) list_conversion(ast[n].first, target);
        facts[n].type = target;
        return;
    }
    expression(n, s);
    require_conversion(n, target);
}
} }
