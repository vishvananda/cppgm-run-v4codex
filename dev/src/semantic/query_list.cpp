#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::store_query_arguments(Expression& call, const std::vector<QueryId>& args, const std::vector<Conversion>& chosen)
{
    call.inputs = CallInputs::Query; call.arguments = query_edges.size(); call.argument_count = args.size();
    query_edges.insert(query_edges.end(),args.begin(),args.end());
    call.conversions = conversions.size(); call.count = chosen.size();
    conversions.insert(conversions.end(),chosen.begin(),chosen.end());
}
Conversion Analyzer::query_list_conversion(QueryId list, TypeId to, bool direct)
{
    TypeQuery q; q.kind = QueryKind::ListInitialization; q.type = to;
    q.context = type_queries[list].context; q.value = direct;
    auto fact = query_fact(intern_query(q,{list}));
    Conversion c; c.target = to; c.kind = Conversion::Kind::QueryList;
    if (fact.state == FactState::Failure || !fact.initialization) return c;
    c.materialization = fact.initialization;
    auto plan = list_plans[c.materialization]; c.rank = plan.rank; c.function = plan.constructor;
    c.reference = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    c.temporary = c.reference && !plan.direct_binding;
    if (c.reference) {
        c.qualification = types[value_type(to)].cv; c.preference = types[to].kind == TypeKind::LRef;
        if (plan.direct_binding) c.preference = conversions[plan.call.conversions].preference;
    }
    return c;
}
std::uint32_t Analyzer::query_list_aggregate(const std::vector<QueryId>& args, unsigned& cursor, TypeId to, ScopeId scope)
{
    ListPlan plan; plan.target = to; plan.scope = scope; plan.aggregate = true;
    std::vector<QueryId> inputs; std::vector<Conversion> chosen; std::vector<ListField> fields;
    TypeQuery empty; empty.kind = QueryKind::List; empty.context = scope;
    auto omitted = intern_query(empty,{});
    auto add = [&](TypeId type, EntityId field, std::uint64_t index, std::uint64_t count) {
        auto source = cursor < args.size() ? args[cursor] : omitted;
        auto value = query_fact(source).expression;
        auto c = conversion_value(value,type);
        if (c.valid()) { if (cursor < args.size()) ++cursor; }
        else if (type_queries[source].kind != QueryKind::List && aggregate_type(type)) {
            auto begin = cursor;
            c.kind = Conversion::Kind::QueryList; c.target = type;
            c.materialization = query_list_aggregate(args,cursor,type,scope);
            c.rank = list_plans[c.materialization].rank;
            if (begin == cursor) return false;
        }
        if (!c.valid()) return false;
        inputs.push_back(source); chosen.push_back(c);
        ListField f; f.field = field; f.type = type; f.index = index; f.count = count; fields.push_back(f); return true;
    };
    bool valid = true; auto target = types[to]; unsigned rank = class_value(to) ? 5 : 0;
    if (target.kind == TypeKind::Array) {
        if (!target.bound) valid = false;
        std::uint64_t index = 0;
        while (valid && cursor < args.size() && index < target.bound) valid = add(target.child,0,index++,1);
        if (valid && index < target.bound) valid = add(target.child,0,index,target.bound-index);
        for (auto c : chosen) if (rank < c.rank) rank = c.rank;
    } else {
        for (auto d = scopes[entities[target.entity].scope].first_decl; valid && d; d = declarations[d].next) {
            auto field = declarations[d].entity;
            if (!nonstatic_field(field)) continue;
            valid = add(initialized_field_type(to,field),field,0,1);
            if (entities[target.entity].key == KW_UNION) break;
        }
    }
    store_query_arguments(plan.call,inputs,chosen); plan.explicit_count = inputs.size();
    plan.fields = list_fields.size(); list_fields.insert(list_fields.end(),fields.begin(),fields.end());
    plan.state = FactState::Success; if (valid) plan.rank = rank;
    auto id = list_plans.size(); list_plans.push_back(plan); return id;
}
TypeQueryFact Analyzer::query_list_initialization(QueryId id)
{
    auto q = type_queries[id]; auto list = query_edges[q.offset];
    if (q.value & 2) {
        q.value &= 1;
        auto formation = query_fact(intern_query(q,{list}));
        if (formation.state == FactState::Failure) return formation;
        return validate_query_list(formation.initialization) ? formation :
            TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
    }
    auto target = types[q.type]; auto t = value_type(q.type);
    bool ref = target.kind == TypeKind::LRef || target.kind == TypeKind::RRef;
    auto list_query = type_queries[list];
    std::vector<QueryId> args(query_edges.begin()+list_query.offset,query_edges.begin()+list_query.offset+list_query.count);
    ListPlan plan; plan.target = q.type; plan.scope = q.context; plan.direct = q.value;
    if (args.size() == 1 && ref && type_queries[args[0]].kind != QueryKind::List) {
        auto c = standard_conversion(query_fact(args[0]).expression,q.type);
        if (c.valid() && c.reference && !c.temporary) {
            plan.direct_binding = true; plan.rank = c.rank; store_query_arguments(plan.call,args,{c});
        }
    }
    auto leaf = t;
    while (types[leaf].kind == TypeKind::Array) leaf = types[leaf].child;
    if (!plan.direct_binding && (!ref || target.kind == TypeKind::RRef || types[leaf].cv == 1)) {
        if (class_value(t)) {
            complete_class(types[t].entity);
            if (!entities[types[t].entity].complete) return incomplete_query(t);
        }
        if (aggregate_type(t)) {
            unsigned cursor = 0; plan = list_plans[query_list_aggregate(args,cursor,t,q.context)];
            plan.target = q.type; plan.direct = q.value;
            if (cursor != args.size()) plan.rank = 255;
        } else if (class_value(t)) {
            std::vector<Expression> values;
            for (auto a : args) values.push_back(query_fact(a).expression);
            Expression recipe;
            plan.constructor = choose_constructor(types.unqualified(t),std::vector<NodeId>(args.size(),0),&recipe,q.context,true,true,&values);
            if (plan.constructor || recipe.form == ExpressionForm::Overload) plan.rank = 5;
            std::vector<Conversion> chosen;
            if (plan.constructor) {
                for (unsigned i = 0; i < args.size(); ++i) chosen.push_back(conversions[recipe.conversions+i]);
                auto m = members[entities[plan.constructor].member_info];
                plan.zero = args.empty() && m.synthetic && !m.defaulted_late;
            }
            store_query_arguments(plan.call,args,chosen); plan.explicit_count = args.size();
        } else if (!fundamental(t,FT_VOID) && types[t].kind != TypeKind::Function) {
            if (args.empty()) plan.rank = 0;
            else if (args.size() == 1) {
                auto c = conversion_value(query_fact(args[0]).expression,t);
                plan.rank = c.rank; store_query_arguments(plan.call,args,{c}); plan.explicit_count = 1;
            }
        }
    }
    plan.query = id; plan.state = FactState::Success;
    TypeQueryFact result; result.expression.type = t;
    result.initialization = list_plans.size(); list_plans.push_back(plan); return result;
}
bool Analyzer::valid_query_list(std::uint32_t id)
{
    auto plan = list_plans[id];
    if (!plan.query) return validate_query_list(id);
    auto q = type_queries[plan.query]; q.value |= 2;
    return query_fact(intern_query(q,{query_edges[q.offset]})).state != FactState::Failure;
}
bool Analyzer::validate_query_list(std::uint32_t id)
{
    auto plan = list_plans[id];
    if (plan.validation == FactState::Success) return true;
    if (plan.validation == FactState::Failure || plan.validation == FactState::Active) return false;
    list_plans[id].validation = FactState::Active;
    auto check = [&]() {
        if (plan.rank == 255) return false;
        auto t = value_type(plan.target);
        if (!plan.direct_binding && (abstract_value(t) || !default_destruction_valid(t,plan.scope))) return false;
        if (plan.constructor) {
            auto ctor = plan.constructor, access = ctor;
            while (members[entities[access].member_info].inherited_constructor)
                access = members[entities[access].member_info].inherited_constructor;
            if (deleted_transfer(ctor) || !accessible(access,plan.scope,entities[access].owner) ||
                (!plan.direct && members[entities[ctor].member_info].explicit_constructor)) return false;
            check_default_constructor(ctor);
            auto f = types[entities[ctor].type];
            std::vector<QueryId> args; std::vector<Conversion> chosen;
            for (unsigned i = 0; i < plan.call.argument_count; ++i) {
                args.push_back(query_edges[plan.call.arguments+i]); chosen.push_back(conversions[plan.call.conversions+i]);
            }
            for (unsigned i = args.size(); i < f.count; ++i) {
                Conversion c; auto n = default_argument(ctor,i,&c,DefaultReason::Recipe);
                args.push_back(expression_query(n,facts[n].scope)); chosen.push_back(c);
            }
            store_query_arguments(plan.call,args,chosen); list_plans[id].call = plan.call;
        } else if (class_value(t) && !plan.aggregate && !plan.direct_binding) return false;
        for (unsigned i = 0; i < plan.call.argument_count; ++i) {
            auto source = query_edges[plan.call.arguments+i]; auto value = query_fact(source).expression;
            auto c = conversions[plan.call.conversions+i];
            if (!valid_fixed_conversion(value,0,c,plan.scope)) return false;
            if (i < plan.explicit_count && c.kind != Conversion::Kind::QueryList && !plan.direct_binding) {
                Constant constant;
                if (narrowing_needs_value(value.type,value_type(c.target))) constant = constants[query_value(source)];
                if (narrowing_conversion(value.type,value_type(c.target),constant)) return false;
            }
            conversions[plan.call.conversions+i] = c;
        }
        return true;
    };
    bool valid = check(); list_plans[id].validation = valid ? FactState::Success : FactState::Failure; return valid;
}
Constant Analyzer::constant_query_list(std::uint32_t id)
{
    auto plan = list_plans[id]; if (!valid_query_list(id)) return Constant();
    plan = list_plans[id];
    auto argument = [&](unsigned i) { return constant_query_conversion(query_edges[plan.call.arguments+i],conversions[plan.call.conversions+i]); };
    if (plan.direct_binding) return argument(0);
    Constant value;
    if (plan.aggregate) {
        std::vector<EvaluatedPart> parts;
        for (unsigned i = 0; i < plan.call.argument_count; ++i) {
            auto field = list_fields[plan.fields+i]; EvaluatedPart p;
            p.selector = field.field ? field.field : field.index; p.count = field.count; p.value = argument(i); parts.push_back(p);
        }
        value = evaluated_object(value_type(plan.target),parts);
    } else if (plan.constructor) {
        std::vector<Constant> args;
        for (unsigned i = 0; i < plan.call.argument_count; ++i) args.push_back(argument(i));
        value = constant_construct(plan.constructor,args,plan.zero);
    } else if (plan.call.argument_count) value = argument(0);
    else value = constant_zero(value_type(plan.target));
    if (value.valid && (types[plan.target].kind == TypeKind::LRef || types[plan.target].kind == TypeKind::RRef))
        return Constant(plan.target,constant_storage_address(value.type,value));
    return value;
}
} }
