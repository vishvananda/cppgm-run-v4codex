#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
TypeQueryFact Analyzer::query_call(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    auto callee_id = query_edges[q.offset]; auto callee = type_queries[callee_id];
    Expression fn = children[0].expression;
    std::vector<Expression> args;
    for (unsigned i = 1; i < children.size(); ++i) args.push_back(children[i].expression);
    if (callee.kind != QueryKind::TypeValue && (class_value(fn.type) || pattern_class_type(fn.type))) {
        TypeQuery call = q; call.op = OP_LPAREN; call.name = operator_name(OP_LPAREN); call.entity = 0;
        return query_operator(call,children);
    }
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
    TypeId object = 0;
    ValueCategory category = ValueCategory::Lvalue;
    if (callee.kind == QueryKind::TypeValue) {
        auto constructed = fn.type;
        if (class_value(constructed)) {
            complete_class(types[constructed].entity); reject_abstract(constructed);
            std::vector<NodeId> nodes(args.size(),0);
            Expression recipe;
            auto ctor = choose_constructor(constructed,nodes,&recipe,q.context,true,true,&args);
            if (!ctor || deleted_transfer(ctor)) throw std::runtime_error("invalid query constructor");
            auto access = ctor;
            while (members[entities[access].member_info].inherited_constructor)
                access = members[entities[access].member_info].inherited_constructor;
            check_access(access,q.context,entities[access].owner);
            check_default_constructor(ctor);
            default_destructor(constructed,q.context,false);
            auto f = types[entities[ctor].type];
            std::vector<Conversion> chosen;
            for (unsigned i = 0; i < args.size(); ++i) {
                auto c = conversions[recipe.conversions+i];
                check_fixed_conversion(args[i],0,c,q.context); chosen.push_back(c);
            }
            for (unsigned i = args.size(); i < f.count; ++i) {
                Conversion c; default_argument(ctor,i,&c,DefaultReason::Recipe); chosen.push_back(c);
            }
            TypeQueryFact result; result.expression.type = constructed; result.selected = ctor;
            result.expression.conversions = conversions.size(); result.expression.count = chosen.size();
            conversions.insert(conversions.end(),chosen.begin(),chosen.end()); return result;
        } else {
            if (integral(constructed) && args.size() == 1 && class_value(args[0].type)) {
                auto c = conversion_function_value(args[0],constructed,true);
                if (!c.valid() || deleted_transfer(c.function)) throw std::runtime_error("invalid scalar constant conversion");
                check_access(c.function,q.context,entities[c.function].owner,args[0].type);
                TypeQueryFact result; result.expression.type = constructed; result.selected = c.function;
                result.expression.conversions = conversions.size(); result.expression.count = 1;
                conversions.push_back(c); return result;
            }
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
            else {
                // A retained member body has a symbolic object identity and
                // cv, with fixed base edges, before its class has a layout.
                auto context = template_object_context(q.context);
                if (context.owner && context.available)
                    object = types.qualify(types.named(context.owner),context.cv);
            }
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
        if (object && entities[selected].member_info && !entities[selected].is_static) {
            Expression value; value.type = object; value.category = category;
            check_fixed_conversion(value,0,chosen[0],q.context);
        }
        for (unsigned i = 0; i < args.size(); ++i)
            check_fixed_conversion(args[i],0,chosen[i+(object!=0)],q.context);
        auto f = types[function_type];
        for (unsigned i = args.size(); i < f.count; ++i) {
            Conversion c; default_argument(selected,i,&c,DefaultReason::Recipe); chosen.push_back(c);
        }
        for (unsigned i = 0; i < f.count; ++i) reject_abstract(types.parameters[f.offset+i]);
        auto count = chosen.size();
        r.expression.conversions = conversions.size(); r.expression.count = count;
        conversions.insert(conversions.end(),chosen.begin(),chosen.end());
    } else {
        function_type = fn.type;
        if (pointer(function_type)) function_type = types[function_type].child;
        auto f = types[function_type];
        if (f.kind != TypeKind::Function || args.size() < f.count || (!f.variadic && args.size() != f.count))
            throw std::runtime_error("invalid indirect type-query call");
        std::vector<Conversion> chosen;
        for (unsigned i = 0; i < args.size(); ++i) {
            auto c = i < f.count ? conversion_value(args[i],types.parameters[f.offset+i]) : ellipsis_conversion_value(args[i]);
            check_fixed_conversion(args[i],0,c,q.context); chosen.push_back(c);
        }
        r.expression.conversions = conversions.size(); r.expression.count = chosen.size();
        conversions.insert(conversions.end(),chosen.begin(),chosen.end());
    }
    auto returned = types[function_type].child;
    r.expression.type = value_type(returned);
    if (types[returned].kind == TypeKind::LRef) r.expression.category = ValueCategory::Lvalue;
    if (types[returned].kind == TypeKind::RRef) r.expression.category = ValueCategory::Xvalue;
    return r;
}
} }
