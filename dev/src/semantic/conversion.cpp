#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
namespace {
unsigned object_cv(const Types& types, TypeId type)
{
    // [basic.type.qualifier]/5: an array has its element's qualification.
    while (types[type].kind == TypeKind::Array) type = types[type].child;
    // Atomic storage is part of type identity, not cv-qualification.
    return types[type].cv & 3;
}
}

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
    return types.non_atomic(types.unqualified(t));
}
bool Analyzer::arithmetic(TypeId t) const
{
    return complex_type(t) || (integral(t) && !scoped_enum(t)) || floating_type(t);
}
TypeId Analyzer::promote(TypeId t)
{
    // Scoped enumerations never undergo integral promotion [conv.prom]. In
    // particular, a narrow underlying type does not promote a switch value.
    if (scoped_enum(t)) return types.non_atomic(types.unqualified(t));
    if (types[t].kind == TypeKind::Named && integral(t) && !scoped_enum(t)) t = entities[types[t].entity].underlying;
    if (types[t].kind == TypeKind::Fundamental && bit_integer_kind(types[t].fundamental)) return types.non_atomic(types.unqualified(t));
    if (fundamental(t, FT_WCHAR_T)) return types.fundamental(FT_INT);
    if (fundamental(t, FT_CHAR32_T)) return types.fundamental(FT_UNSIGNED_INT);
    return integral(t) && width(t) < 32 ? types.fundamental(FT_INT) : types.non_atomic(types.unqualified(t));
}
TypeId Analyzer::promote_expression(NodeId n)
{
    auto expression = expressions[n];
    auto field = field_fact(expression.entity);
    TypeId t = decay(expression.type);
    if (field.bit_field && types[t].kind != TypeKind::Named && !bit_integer_kind(types[t].fundamental)) {
        if (field.width < 32 || (field.width == 32 && !is_unsigned(t))) return types.fundamental(FT_INT);
        if (field.width == 32) return types.fundamental(FT_UNSIGNED_INT);
    }
    return promote(t);
}
TypeId Analyzer::arithmetic_type(TypeId a, TypeId b)
{
    if (!arithmetic(a) || !arithmetic(b)) throw std::runtime_error("arithmetic operands required");
    a = promote(a); b = promote(b);
    if (complex_type(a) || complex_type(b)) {
        auto component = arithmetic_type(complex_type(a) ? complex_component(a) : a,complex_type(b) ? complex_component(b) : b);
        if (types[component].fundamental > FT_LONG_DOUBLE) throw std::runtime_error("unsupported extended complex component");
        return types.fundamental(EFundamentalType(FT_COMPLEX_FLOAT + types[component].fundamental - FT_FLOAT));
    }
    if (!integral(a) || !integral(b)) {
        if (integral(a)) return b;
        if (integral(b)) return a;
        auto af=types[a].fundamental, bf=types[b].fundamental;
        auto ar=floating_precision(af), br=floating_precision(bf);
        if (ar!=br) return ar>br?a:b;
        // Extended fixed-width types have greater subrank than their ordinary
        // representation; the non-x type wins an equal-format extended pair.
        auto subrank=[](EFundamentalType f) { return f==FT_FLOAT32 || f==FT_FLOAT64 || f==FT_STDFLOAT128?2:f==FT_FLOAT32X || f==FT_FLOAT64X?1:0; };
        return subrank(af)>=subrank(bf)?a:b;
    }
    // Rank is distinct from width: both long and long long are 64-bit on LP64.
    unsigned ar = types[a].fundamental, br = types[b].fundamental;
    if (width(a) == 128) ar = FT_LONG_LONG_INT+1;
    else if (is_unsigned(a)) ar -= FT_UNSIGNED_CHAR;
    if (width(b) == 128) br = FT_LONG_LONG_INT+1;
    else if (is_unsigned(b)) br -= FT_UNSIGNED_CHAR;
    // Bit-precise types rank below ordinary integers of equal width.
    // Preserve long/long-long rank within the ordinary group.
    ar = width(a)*16 + (bit_integer_kind(types[a].fundamental) ? 0 : ar+1);
    br = width(b)*16 + (bit_integer_kind(types[b].fundamental) ? 0 : br+1);
    if (is_unsigned(a) == is_unsigned(b)) return ar >= br ? a : b;
    TypeId u = is_unsigned(a) ? a : b, s = is_unsigned(a) ? b : a;
    unsigned ur = is_unsigned(a) ? ar : br, sr = is_unsigned(a) ? br : ar;
    if (ur >= sr) return u;
    if (width(s) > width(u)) return s;
    if (bit_integer_kind(types[s].fundamental)) return types.bit_integer(width(s),true);
    return types.fundamental(width(s) == 128 ? FT_UINT128 : EFundamentalType(types[s].fundamental + FT_UNSIGNED_CHAR));
}
bool Analyzer::object_pointer(TypeId t)
{
    if (!pointer(t)) return false;
    Type child = types[types[t].child];
    if (fundamental(types[t].child, FT_VOID) || child.kind == TypeKind::Function) return false;
    if (child.kind == TypeKind::Array && child.unknown_bound) return false;
    if (child.kind == TypeKind::Named && !entities[child.entity].complete) return false;
    return true;
}
TypeId Analyzer::composite_pointer(TypeId a, TypeId b)
{
    if (types[a].kind == TypeKind::MemberPointer && types[b].kind == TypeKind::MemberPointer) {
        auto ac = types[a].child, bc = types[b].child;
        TypeId child = 0;
        if (types[ac].kind == TypeKind::Function || types[bc].kind == TypeKind::Function) {
            if (ac != bc) return 0;
            child = ac;
        } else if (types.unqualified(ac) == types.unqualified(bc))
            child = types.qualify(ac,types[ac].cv | types[bc].cv);
        else if (pointer(ac) && pointer(bc)) {
            child = composite_pointer(ac,bc);
            if (child) child = types.qualify(child,types[ac].cv | types[bc].cv | 1);
        }
        if (!child) return 0;
        unsigned added = 0;
        if (!qualification(ac,child,added) || !qualification(bc,child,added)) return 0;
        auto ae = types[a].entity, be = types[b].entity;
        auto owner = ae == be || derived_from(entities[ae].type,entities[be].type) ? ae :
            derived_from(entities[be].type,entities[ae].type) ? be : 0;
        return owner ? types.member_pointer(owner,child) : 0;
    }
    if (block_pointer(a) || block_pointer(b))
        return block_pointer(a) && block_pointer(b) && types[a].child == types[b].child ? types.unqualified(a) : 0;
    if (!pointer(a) || !pointer(b)) return 0;
    TypeId ac = types[a].child, bc = types[b].child, result = 0;
    unsigned cv = object_cv(types,ac) | object_cv(types,bc);
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
    if (a.kind == TypeKind::Function || b.kind == TypeKind::Function) return from == to;
    if (a.kind != b.kind || ((a.cv ^ b.cv) & 4) || (a.cv & ~b.cv)) return false;
    if (a.cv != b.cv) {
        if (!intermediate_const) return false;
        added |= b.cv & ~a.cv;
    }
    if (a.kind == TypeKind::Pointer || a.kind == TypeKind::Array || a.kind == TypeKind::MemberPointer) {
        if (a.kind == TypeKind::MemberPointer && a.entity != b.entity) return false;
        if (a.kind == TypeKind::Array && (a.bound != b.bound || a.unknown_bound != b.unknown_bound)) return false;
        return qualification(a.child, b.child, added,
            intermediate_const && (a.kind == TypeKind::Array || (b.cv & 1)));
    }
    return types.unqualified(from) == types.unqualified(to);
}
bool Analyzer::similar_type(TypeId a, TypeId b)
{
    if (types[a].kind != types[b].kind || ((types[a].cv ^ types[b].cv) & 4)) return false;
    if (types[a].kind == TypeKind::Array)
        return types[a].bound == types[b].bound && types[a].unknown_bound == types[b].unknown_bound && similar_type(types[a].child,types[b].child);
    if (pointer(a) || types[a].kind == TypeKind::MemberPointer)
        return (pointer(a) || types[a].entity == types[b].entity) && similar_type(types[a].child, types[b].child);
    return types.unqualified(a) == types.unqualified(b);
}
Conversion Analyzer::standard_conversion(Expression x, TypeId to, NodeId n)
{
    if (intrinsic_function(x.entity) >= Intrinsic::SourceFile && intrinsic_function(x.entity) <= Intrinsic::SourceColumn &&
        types[x.type].kind == TypeKind::Function) return Conversion();
    ++conversion_work;
    Conversion c; c.target = to;
    Type target = types[to];
    bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
    if (x.form == ExpressionForm::Overload) {
        TypeId ft = ref || pointer(to) || target.kind == TypeKind::MemberPointer ? target.child : to;
        if (types[ft].kind != TypeKind::Function) return c;
        std::vector<EntityId> matching;
        for (EntityId e : candidates(x.entity)) {
            ++candidate_work;
            if (direct_intrinsic(e)) continue;
            if (definitions && entities[e].template_info) e = deduce_target(e,ft);
            if (e) require_deduced_return(e);
            if (!e || entities[e].type != ft) continue;
            bool member = entities[e].member_info && !entities[e].is_static;
            if (member != (target.kind == TypeKind::MemberPointer)) continue;
            matching.push_back(e);
        }
        auto preferred = [&](EntityId a, EntityId b) {
            return (!entities[a].specialization && entities[b].specialization) || template_more_specialized(a,b);
        };
        for (auto e : matching) if (!c.function || preferred(e,c.function)) c.function = e;
        // Do not reject a tied prefix: a later candidate can dominate both.
        for (auto e : matching) if (e != c.function && !preferred(c.function,e)) return Conversion();
        if (c.function) { c.rank = 0; c.reference = ref; c.preference = target.kind == TypeKind::RRef; }
        return c;
    }
    TypeId from = x.type;
    if (!from) return c;
    if (types[from].kind == TypeKind::Function && direct_intrinsic(x.entity)) return c;
    // A function-to-pointer/reference conversion selects the declaration even
    // without an overload set. This owns demand for static member addresses.
    if (types[from].kind == TypeKind::Function && x.entity && entities[x.entity].kind == EntityKind::Function &&
        (ref || pointer(to)) && target.child == from &&
        (!entities[x.entity].member_info || entities[x.entity].is_static)) {
        c.rank = 0; c.function = x.entity; c.reference = ref;
        c.preference = target.kind == TypeKind::RRef; return c;
    }
    if (ref) {
        if (field_fact(x.entity).bit_field) {
            if (target.kind != TypeKind::LRef || object_cv(types,target.child) != 1) return c;
            c = standard_conversion(x, types.unqualified(target.child), n);
            c.target = to; c.reference = true; c.temporary = true; return c;
        }
        unsigned added = 0;
        bool function_lvalue = types[from].kind == TypeKind::Function;
        bool category = target.kind == TypeKind::LRef ? x.category == ValueCategory::Lvalue : x.category != ValueCategory::Lvalue;
        bool const_binding = target.kind == TypeKind::LRef && object_cv(types,target.child) == 1;
        if ((category || const_binding) && derived_from(from, target.child) && !(object_cv(types,from) & ~object_cv(types,target.child))) {
            c.adjustment = base_path(from,types[target.child].entity);
            if (base_adjustments[c.adjustment].ambiguous) return c;
            c.adjustment = base_steps(from,types[target.child].entity);
            c.rank = 2; c.reference = true; c.derived = true; c.qualification = object_cv(types,target.child) & ~object_cv(types,from);
            c.preference = x.category != ValueCategory::Lvalue && target.kind == TypeKind::LRef; return c;
        }
        if ((category || const_binding || function_lvalue) && qualification(from, target.child, added)) {
            c.rank = 0; c.reference = true; c.qualification = added;
            c.preference = function_lvalue ? target.kind == TypeKind::RRef : x.category != ValueCategory::Lvalue && target.kind == TypeKind::LRef; return c;
        }
        // A const, nonvolatile lvalue reference may bind a converted temporary.
        auto source_object = from, target_object = target.child;
        while (types[source_object].kind == TypeKind::Array && types[target_object].kind == TypeKind::Array &&
            types[source_object].bound == types[target_object].bound && types[source_object].unknown_bound == types[target_object].unknown_bound) {
            source_object = types[source_object].child; target_object = types[target_object].child;
        }
        bool related = types.unqualified(source_object) == types.unqualified(target_object) || derived_from(from,target.child);
        if ((target.kind == TypeKind::LRef && object_cv(types,target.child) != 1) ||
            (related && (object_cv(types,from) & ~object_cv(types,target.child))) ||
            (related && target.kind == TypeKind::RRef)) return c;
        c = standard_conversion(x, types.unqualified(target.child), n);
        c.target = to; c.reference = true; c.qualification = object_cv(types,target.child);
        c.temporary = types.unqualified(from) != types.unqualified(target.child);
        c.preference = target.kind == TypeKind::LRef;
        return c;
    }
    to = types.unqualified(to);
    from = decay(from);
    if (class_value(to) && class_value(from) && (to == from || derived_from(from,to))) {
        c = transfer_initialization(x,c.target,InitializationMode::Copy);
        return c;
    }
    if (to == from) { c.rank = 0; return c; }
    if ((address_value(to) || types[to].kind == TypeKind::MemberPointer || fundamental(to, FT_NULLPTR_T)) && (fundamental(x.type,FT_NULLPTR_T) || x.null_pointer_constant || (n && null_constant(n)))) { c.rank = 2; return c; }
    if (types[from].kind == TypeKind::MemberPointer && types[to].kind == TypeKind::MemberPointer) {
        unsigned added = 0;
        if (!qualification(types[from].child,types[to].child,added)) return c;
        if (types[from].entity == types[to].entity) { c.rank = 0; c.qualification = added; return c; }
        auto derived = entities[types[to].entity].type, base = entities[types[from].entity].type;
        if (!derived_from(derived,base)) return c;
        auto path = base_path(derived,types[from].entity);
        if (base_adjustments[path].ambiguous) return c;
        for (auto at = path; at; at = base_adjustments[at].next)
            if (bases[base_adjustments[at].edge].virtual_base) return c;
        c.adjustment = base_steps(derived,types[from].entity);
        c.rank = 2; c.derived = true; c.qualification = added; return c;
    }
    if (fundamental(to, FT_BOOL) && (address_value(from) || types[from].kind == TypeKind::MemberPointer)) { c.rank = 3; return c; }
    if (block_pointer(from) && pointer(to) && fundamental(types[to].child,FT_VOID) && !(types[types[to].child].cv & 4)) {
        c.rank = 2; return c;
    }
    if (pointer(from) && pointer(to)) {
        unsigned added = 0;
        if (qualification(types[from].child, types[to].child, added)) {
            c.rank = 0; c.qualification = added; return c;
        }
        Type a = types[types[from].child], b = types[types[to].child];
        a.cv = object_cv(types,types[from].child); b.cv = object_cv(types,types[to].child);
        if (derived_from(types[from].child, types[to].child) && !(a.cv & ~b.cv)) {
            c.adjustment = base_path(types[from].child,b.entity);
            if (base_adjustments[c.adjustment].ambiguous) return c;
            c.adjustment = base_steps(types[from].child,b.entity);
            c.rank = 2; c.derived = true; c.qualification = b.cv & ~a.cv; return c;
        }
        if (fundamental(types[to].child, FT_VOID) && a.kind != TypeKind::Function && !(a.cv & ~b.cv)) {
            c.rank = 2; c.qualification = b.cv & ~a.cv; return c;
        }
    }
    if (complex_type(from) && !complex_type(to) && !fundamental(to,FT_BOOL)) return c;
    if (arithmetic(from) && arithmetic(to) && types[to].kind != TypeKind::Named) {
        c.rank = (n ? promote_expression(n) : promote(from)) == to ? 1 : 2;
        return c;
    }
    return c;
}
void Analyzer::select_function(NodeId n, EntityId e, bool direct)
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
    auto value = expressions[n];
    value.entity = e; value.form = ExpressionForm::Ordinary; value.type = entities[e].type;
    expressions.set(n,value);
    { auto& published = facts.edit(n); published.type = entities[e].member_info ? members[entities[e].member_info].call_type : entities[e].type;
    published.entity = e; }
    use_selected_function(e,direct);
    if (ast[n].kind == Kind::Parenthesized || (ast[n].kind == Kind::Unary && ast[n].op == OP_AMP)) {
        select_function(ast[n].first, e, direct);
        if (ast[n].kind == Kind::Unary) {
            value = expressions[n]; value.type = entities[e].member_info && !entities[e].is_static ?
                types.member_pointer(scopes[entities[e].owner].entity, entities[e].type) : types.compound(TypeKind::Pointer, entities[e].type);
            value.category = ValueCategory::Prvalue; expressions.set(n,value);
            facts.edit(n).type = value.type;
        }
    }
}
void Analyzer::use_selected_function(EntityId e, bool direct)
{
    if (direct_intrinsic(e) && !direct)
        throw std::runtime_error("compiler intrinsic requires a direct call");
    if (discarded_statement()) return;
    if (destructor_member(e)) members[entities[e].member_info].retained_root = true;
    if (direct && entities[e].member_info) members[entities[e].member_info].emission_reference = true;
    demand_member(e);
    demand_specialization(e);
}
void Analyzer::apply_conversion(NodeId n, Conversion& c)
{
    if (c.ellipsis_object && unevaluated_depth && unevaluated_depth != body_evaluation_depth &&
        (!active_default_fact || unevaluated_depth > 1)) {
        if (!valid_fixed_conversion(expressions[n],n,c,facts[n].scope,true))
            throw std::runtime_error("invalid unevaluated ellipsis argument");
        return;
    }
    if (c.ellipsis_object && !c.empty_copy && c.kind != Conversion::Kind::Construction &&
        expressions[n].category != ValueCategory::Prvalue)
        throw std::runtime_error("class ellipsis argument has no value transfer");
    if (n && c.reference && !c.temporary) observe_scalar(n,c.storage_write);
    if (c.kind == Conversion::Kind::ListPlan) { prepare_list(n,c); return; }
    if (c.kind == Conversion::Kind::List) return;
    if (c.kind == Conversion::Kind::User) { prepare_user_conversion(n,c); return; }
    if (c.kind == Conversion::Kind::Construction) { materialize_conversion(n, c); return; }
    if (c.derived && c.kind != Conversion::Kind::Explicit) {
        TypeId from = expressions[n].type, to = types[c.target].child;
        if (pointer(from)) from = types[from].child;
        if (pointer(to)) to = types[to].child;
        if (types[c.target].kind == TypeKind::MemberPointer) {
            to = entities[types[from].entity].type; from = entities[types[c.target].entity].type;
        }
        check_base_access(from, to, facts[n].scope);
    }
    if (c.function) select_function(n, c.function);
    if (expressions[n].entity) demand_specialization(expressions[n].entity);
    TypeId target = types.unqualified(c.target);
    if (ast[n].kind == Kind::Literal && (pointer(target) || fundamental(target, FT_NULLPTR_T)) && null_constant(n)) facts.edit(n).type = target;
}
void Analyzer::require_conversion(NodeId n, TypeId target, bool direct)
{
    auto occurrence = ast.nodes.occurrences[n];
    auto retained = occurrence.context && !direct ? template_statement_conversions.get(occurrence.source) : 0;
    if (retained && conversions[retained].target == target) ++statement_conversion_uses;
    else retained = retained_initialization(n,target);
    Conversion c = retained && conversions[retained].target == target ? copy_conversion_recipe(conversions[retained]) :
        direct ? direct_initialization_conversion(expressions[n],target,n) : conversion(n, target);
    if (!c.valid()) throw std::runtime_error("invalid implicit conversion");
    apply_conversion(n, c);
    expressions.incoming(n,conversions.size());
    conversions.push_back(c);
}
void Analyzer::record_conversion(Expression& owner, NodeId n, Conversion c)
{
    if (n && c.reference && !c.temporary) observe_scalar(n,c.storage_write);
    if (!c.valid()) throw std::runtime_error("invalid operand conversion");
    if (c.kind == Conversion::Kind::ListPlan) prepare_list(n,c);
    if (n && c.kind == Conversion::Kind::Construction) materialize_conversion(n, c);
    if (n && c.kind == Conversion::Kind::User) prepare_user_conversion(n,c);
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
            if (types[c.target].kind == TypeKind::MemberPointer) {
                to = entities[types[from].entity].type; from = entities[types[c.target].entity].type;
            }
            check_base_access(from, to, facts[n].scope);
        }
        if (c.function && c.kind != Conversion::Kind::Construction && c.kind != Conversion::Kind::User && c.kind != Conversion::Kind::List) select_function(n, c.function);
        if (expressions[n].entity) demand_specialization(expressions[n].entity);
        expressions.incoming(n,conversions.size());
    }
    conversions.push_back(c);
}
void Analyzer::record_call(Expression& owner, const std::vector<NodeId>& args, std::vector<Conversion>& selected)
{
    for (const auto& c : selected) if (!c.valid()) throw std::logic_error("call lacks a valid argument conversion");
    for (std::size_t j = 0; j < args.size(); ++j) if (args[j] || selected[j].kind == Conversion::Kind::ListPlan) apply_conversion(args[j], selected[j]);
    store_call(owner,args,selected);
    for (std::size_t j = 0; j < args.size(); ++j) if (args[j]) expressions.incoming(args[j],owner.conversions+j);
}
void Analyzer::store_call(Expression& owner, const std::vector<NodeId>& args, const std::vector<Conversion>& selected)
{
    owner.inputs = CallInputs::Concrete;
    owner.arguments = call_arguments.size(); owner.argument_count = args.size();
    owner.conversions = conversions.size(); owner.count = args.size();
    conversions.insert(conversions.end(), selected.begin(), selected.end());
    call_arguments.insert(call_arguments.end(), args.begin(), args.end());
}
Conversion Analyzer::boolean_conversion(NodeId n)
{
    return boolean_conversion_value(expressions[n],n);
}
Conversion Analyzer::boolean_conversion_value(Expression source, NodeId n)
{
    Conversion c = class_value(source.type) ? conversion_function_value(source,types.fundamental(FT_BOOL),true) : conversion_value(source, types.fundamental(FT_BOOL),true,n);
    // Contextual bool conversion uses direct-initialization, which admits
    // nullptr_t; ordinary copy-initialization of bool still rejects it.
    if (fundamental(source.type, FT_NULLPTR_T)) {
        c.rank = 2; c.kind = Conversion::Kind::Contextual;
    }
    return c;
}
void Analyzer::initialize(NodeId n, TypeId target, ScopeId s, InitializationMode mode)
{
    s = expanded_scope(n,s);
    if (source_builtins_present) remember_source_site(n,s);
    auto source = n;
    while (ast[source].kind == Kind::Initializer) source = ast[source].first;
    expand_expression_list(source,s);
    if (ast[n].kind == Kind::Initializer && (ast[n].flags & 1)) mode = InitializationMode::Copy;
    if (class_value(target)) {
        complete_class(types[target].entity);
        if (class_initialize(n, target, s, mode)) return;
    }
    if (ast[n].kind == Kind::Initializer) { initialize(ast[n].first, target, s, mode); return; }
    if (ast[n].kind == Kind::BracedInit && (types[target].kind == TypeKind::LRef || types[target].kind == TypeKind::RRef)) {
        expression(n,s); require_conversion(n,target); return;
    }
    if (ast[n].kind == Kind::BracedInit && complex_type(target) && ast[n].first != ast[n].last) {
        expression(n,s); require_conversion(n,target); return;
    }
    if ((aggregate_type(target) || vector_kind(types[target].kind)) && (types[target].kind == TypeKind::Array || ast[n].kind == Kind::BracedInit ||
        ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::ParenInitializer)) {
        aggregate_initialization(n, target, s); return;
    }
    if (ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) {
        if (!ast[n].first) {
            facts.edit(n).type = target; auto value = expressions[n]; value.type = target; value.ready = true;
            expressions.set(n,value); return;
        }
        if (ast[n].first != ast[n].last) throw std::runtime_error("too many scalar initializers");
        if (ast[n].kind != Kind::BracedInit && ast[ast[n].first].kind != Kind::BracedInit && class_value(expression(ast[n].first,s).type) && !class_value(value_type(target)))
            require_conversion(ast[n].first,target,true);
        else initialize(ast[n].first, target, s, mode);
        if (ast[n].kind == Kind::BracedInit) list_conversion(ast[n].first, target);
        facts.edit(n).type = target;
        return;
    }
    expression(n, s);
    require_conversion(n, target);
}
} }
