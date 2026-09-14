#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::conversion_nonthrowing(const Conversion& c)
{
    if (c.kind == Conversion::Kind::User) {
        if (!function_nonthrowing(c.function)) return false;
        if (class_value(types[entities[c.function].type].child) &&
            !type_destructor_nonthrowing(types[entities[c.function].type].child)) return false;
    }
    if (c.kind == Conversion::Kind::Construction) {
        if (!function_nonthrowing(c.function)) return false;
        if (!type_destructor_nonthrowing(value_type(c.target))) return false;
    }
    if (c.kind == Conversion::Kind::List || c.kind == Conversion::Kind::ListPlan) {
        auto plan = c.kind == Conversion::Kind::List ? list_objects[c.materialization].plan : c.materialization;
        auto constructor = list_plans[plan].constructor;
        if (constructor && !function_nonthrowing(constructor)) return false;
    }
    return true;
}
bool Analyzer::expression_nonthrowing(NodeId n)
{
    if (!n) return true;
    if (auto known = expression_exception_facts.get(n)) return known == 2;
    ++exception_work;
    auto node = ast[n]; auto x = expressions[n];
    // These operands are unevaluated, including nested noexcept. Their type
    // and validity have already been checked by semantic construction.
    if (node.kind == Kind::Sizeof || node.kind == Kind::SizeofPack || node.kind == Kind::TypeTrait) return true;
    bool result = node.kind != Kind::Throw && node.kind != Kind::New && node.kind != Kind::Delete;
    auto callee = facts[n].entity;
    bool call = (node.kind == Kind::Call && x.form != ExpressionForm::Cast &&
        x.form != ExpressionForm::ListValue && x.form != ExpressionForm::PseudoDestructor) ||
        x.form == ExpressionForm::OperatorCall || (callee && constructor_member(callee));
    if (call) result &= callee && function_nonthrowing(callee);
    if (x.category == ValueCategory::Prvalue && class_value(x.type)) result &= type_destructor_nonthrowing(x.type);
    for (unsigned i = 0; i < x.count; ++i) result &= conversion_nonthrowing(conversions[x.conversions+i]);
    for (unsigned i = 0; i < x.argument_count; ++i) {
        auto arg = call_argument(x,i);
        if (arg && arg != n) result &= expression_nonthrowing(arg);
    }
    for (auto child = node.first; child; child = ast[child].next) result &= expression_nonthrowing(child);
    expression_exception_facts.put(n,result ? 2 : 1); return result;
}
bool Analyzer::query_nonthrowing(QueryId id)
{
    if (auto known = query_exception_facts.get(id)) return known == 2;
    auto fact = query_fact(id); auto q = type_queries[id];
    if (fact.dependent) throw std::logic_error("dependent exception effect demand");
    ++exception_work;
    if (q.kind == QueryKind::Sizeof || q.kind == QueryKind::SizeofPack) return true;
    bool result = q.kind != QueryKind::New;
    if (fact.selected) result &= function_nonthrowing(fact.selected);
    else if (q.kind == QueryKind::Call && type_queries[query_edges[q.offset]].kind != QueryKind::TypeValue) result = false;
    auto x = fact.expression;
    if (x.category == ValueCategory::Prvalue && class_value(x.type)) result &= type_destructor_nonthrowing(x.type);
    for (unsigned i = 0; i < x.count; ++i) result &= conversion_nonthrowing(conversions[x.conversions+i]);
    for (unsigned i = 0; i < q.count; ++i) result &= query_nonthrowing(query_edges[q.offset+i]);
    if (q.kind == QueryKind::Call && fact.selected) {
        auto f = types[entities[fact.selected].type];
        for (unsigned i = q.count-1; i < f.count; ++i)
            result &= expression_nonthrowing(default_argument(fact.selected,i,0,DefaultReason::Recipe));
    }
    query_exception_facts.put(id,result ? 2 : 1); return result;
}
bool Analyzer::default_constructor_nonthrowing(EntityId e)
{
    auto state = BooleanFact(default_exception_facts.get(e));
    if (state == BooleanFact::True || state == BooleanFact::False) return state == BooleanFact::True;
    if (state == BooleanFact::Failure) throw std::runtime_error("failed default constructor exception fact");
    if (state == BooleanFact::Active) throw std::logic_error("recursive default constructor exception fact");
    auto cls = scopes[entities[e].owner].entity;
    require_destructor_class(cls);
    default_exception_facts.put(e,unsigned(BooleanFact::Active));
    ++unevaluated_depth;
    try {
        ++exception_work;
        auto scope = entities[cls].scope;
        auto subobject = [&](TypeId type, NodeId initializer) {
            ++exception_work;
            if (initializer) {
                initialize(initializer,type,scope);
                return expression_nonthrowing(initializer);
            }
            while (types[type].kind == TypeKind::Array) type = types[type].child;
            if (!class_value(type)) return true;
            auto ctor = default_constructor(type,scope,false);
            bool value = function_nonthrowing(ctor);
            auto f = types[entities[ctor].type];
            for (unsigned i = 0; i < f.count; ++i) {
                Conversion c;
                auto arg = default_argument(ctor,i,&c,DefaultReason::Recipe);
                value &= expression_nonthrowing(arg) && conversion_nonthrowing(c);
            }
            return value;
        };
        bool result = true;
        auto info = entities[cls].class_info;
        for (auto b = class_facts[info].first_base; b; b = bases[b].next)
            result &= subobject(entities[bases[b].base].type,0);
        for (auto d = scopes[scope].first_decl; d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == scope) {
                if (entities[cls].key == KW_UNION && !entities[field].initializer) continue;
                result &= subobject(entities[field].type,entities[field].initializer);
            }
        }
        default_exception_facts.put(e,unsigned(result ? BooleanFact::True : BooleanFact::False));
        --unevaluated_depth; return result;
    } catch (const UnavailableSemanticFact&) {
        --unevaluated_depth; default_exception_facts.put(e,unsigned(BooleanFact::NotStarted)); throw;
    } catch (...) { --unevaluated_depth; default_exception_facts.put(e,unsigned(BooleanFact::Failure)); throw; }
}
} }
