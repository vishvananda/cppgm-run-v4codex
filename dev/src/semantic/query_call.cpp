#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::query_call(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    auto callee_id = query_edges[q.offset]; auto callee = type_queries[callee_id];
    Expression fn = children[0].expression;
    std::vector<Expression> args;
    for (unsigned i = 1; i < children.size(); ++i) args.push_back(children[i].expression);
    if (callee.kind == QueryKind::Name && callee.name) {
        bool adl = !fn.entity || function_binding(fn.entity);
        if (fn.entity && adl) for (auto e : candidates(fn.entity)) {
            auto kind = scopes[entities[e].owner].kind;
            if (kind == ScopeKind::Class || kind == ScopeKind::Block || kind == ScopeKind::Function) adl = false;
        }
        if (adl) {
            std::vector<TypeId> types;
            for (auto x : args) if (x.type) types.push_back(x.type);
            fn.entity = merge_lookup(fn.entity,associated_type_lookup(callee.name,std::move(types)));
        }
    }
    TypeId constructed = 0, object = 0;
    ValueCategory category = ValueCategory::Lvalue;
    if (callee.kind == QueryKind::TypeValue) {
        constructed = fn.type;
        if (class_value(constructed)) {
            complete_class(types[constructed].entity); reject_abstract(constructed);
            if (args.empty()) {
                default_constructor(constructed,q.context,false);
                TypeQueryFact result; result.expression.type = constructed; return result;
            }
            fn.entity = class_facts[entities[types[constructed].entity].class_info].constructor;
            object = constructed;
        } else {
            if (args.size() > 1 || (!args.empty() && !standard_conversion(args[0],constructed).valid()))
                throw std::runtime_error("invalid scalar type-query construction");
            TypeQueryFact result; result.expression.type = constructed; return result;
        }
    }
    if (callee.kind == QueryKind::Member) {
        auto x = query_fact(query_edges[callee.offset]).expression;
        object = callee.op == OP_ARROW ? types[x.type].child : x.type;
        category = callee.op == OP_ARROW ? ValueCategory::Lvalue : x.category;
    }
    if (callee.kind == QueryKind::Name && fn.entity && function_binding(fn.entity)) {
        for (auto e : candidates(fn.entity)) if (entities[e].member_info) {
            auto implicit = implicit_object_type(q.context);
            if (implicit) object = types[implicit].child;
            break;
        }
    }
    TypeQueryFact r;
    TypeId function_type = 0;
    if (fn.entity && function_binding(fn.entity)) {
        std::vector<Conversion> chosen;
        auto choice = select_call(fn.entity,args,0,object,category,0,callee.arguments,chosen);
        if (choice.failure == CallFailure::NoViable) throw std::runtime_error("no viable function in type query");
        if (choice.failure == CallFailure::Ambiguous) throw std::runtime_error("ambiguous function in type query");
        auto selected = choice.entity;
        if (entities[selected].member_info && members[entities[selected].member_info].deleted)
            throw std::runtime_error("deleted function in type query");
        check_access(selected,q.context,entities[selected].owner,object);
        function_type = entities[selected].type; r.selected = selected;
        auto count = chosen.size();
        r.expression.conversions = conversions.size(); r.expression.count = count;
        conversions.insert(conversions.end(),chosen.begin(),chosen.end());
    } else {
        function_type = fn.type;
        if (pointer(function_type)) function_type = types[function_type].child;
        auto f = types[function_type];
        if (f.kind != TypeKind::Function || args.size() < f.count || (!f.variadic && args.size() != f.count))
            throw std::runtime_error("invalid indirect type-query call");
        for (unsigned i = 0; i < f.count; ++i)
            if (!conversion_value(args[i],types.parameters[f.offset+i]).valid()) throw std::runtime_error("invalid type-query argument");
    }
    auto returned = constructed ? constructed : types[function_type].child;
    r.expression.type = value_type(returned);
    if (!constructed && types[returned].kind == TypeKind::LRef) r.expression.category = ValueCategory::Lvalue;
    if (!constructed && types[returned].kind == TypeKind::RRef) r.expression.category = ValueCategory::Xvalue;
    return r;
}
} }
