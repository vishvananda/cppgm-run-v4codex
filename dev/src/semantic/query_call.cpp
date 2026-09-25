#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
QueryId Analyzer::call_query(NodeId n, ScopeId s)
{
    TypeQuery q; q.kind = QueryKind::Call; q.context = s;
    std::vector<QueryId> children; auto first = ast[n].first;
    while (!template_object_context_index.get(q.context) &&
        (scopes[q.context].kind == ScopeKind::Template || scopes[q.context].kind == ScopeKind::Block))
        q.context = scopes[q.context].parent;
    if (invoke_expression(n,s)) q.name = invoke_builtin;
    else children.push_back(expression_query(first,s,true));
    auto list = ast[first].next;
    if (ast[list].kind == Kind::BracedInit) {
        q.op = OP_LBRACE;
        children.push_back(expression_query(list,s));
    } else for (auto a = ast[list].first; a; a = ast[a].next) children.push_back(expression_query(a,s));
    if (!children.empty()) {
        auto callee = type_queries[children[0]];
        while (callee.kind == QueryKind::Parenthesized) callee = type_queries[query_edges[callee.offset]];
        if (callee.kind == QueryKind::Name && function_binding(callee.entity))
            for (auto e : candidates(callee.entity)) if (scopes[entities[e].owner].kind == ScopeKind::Class && !entities[e].is_static) {
                q.type = implicit_object_type(s); break;
            }
    }
    if (template_type_probe) for (auto child : children) if (!child) return 0;
    return intern_query(q,children);
}
TypeQueryFact Analyzer::query_call(const TypeQuery& q, const std::vector<TypeQueryFact>& children)
{
    if (children.empty()) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
    auto callee_id = query_edges[q.offset]; auto callee = type_queries[callee_id];
    bool parenthesized = callee.kind == QueryKind::Parenthesized;
    while (callee.kind == QueryKind::Parenthesized) {
        callee_id = query_edges[callee.offset]; callee = type_queries[callee_id];
    }
    Expression fn = children[0].expression;
    std::vector<Expression> args;
    for (unsigned i = 1; i < children.size(); ++i) args.push_back(children[i].expression);
    if (fn.form == ExpressionForm::PseudoDestructor) {
        if (!args.empty()) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        TypeQueryFact result; result.expression.type = types.fundamental(FT_VOID);
        result.expression.form = ExpressionForm::PseudoDestructor; return result;
    }
    if (callee.kind != QueryKind::TypeValue && (class_value(fn.type) || pattern_class_type(fn.type))) {
        TypeQuery call = q; call.op = OP_LPAREN; call.name = operator_name(OP_LPAREN); call.entity = 0;
        return query_operator(call,children);
    }
    if (!parenthesized && q.name != invoke_builtin && callee.kind == QueryKind::Name && callee.name) {
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
        if (q.op == OP_LBRACE) {
            auto c = query_list_conversion(query_edges[q.offset+1],constructed,true,q.value != 0);
            if (!c.valid() || !valid_query_list(c.materialization))
                return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            TypeQueryFact result; result.expression.type = value_type(constructed);
            result.expression.category = types[constructed].kind == TypeKind::LRef ? ValueCategory::Lvalue :
                types[constructed].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
            result.expression.conversions = conversions.size(); result.expression.count = 1;
            conversions.push_back(c); result.initialization = c.materialization; return result;
        }
        if (class_value(constructed)) {
            complete_class(types[constructed].entity);
            if (!entities[types[constructed].entity].complete || abstract_value(constructed))
                return incomplete_query(constructed);
            std::vector<NodeId> nodes(args.size(),0);
            Expression recipe;
            auto ctor = choose_constructor(constructed,nodes,&recipe,q.context,true,true,&args);
            if (!ctor) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
            if (deleted_transfer(ctor)) return TypeQueryFact::failed(TypeQueryFact::Failure::Deleted);
            auto access = ctor;
            while (members[entities[access].member_info].inherited_constructor)
                access = members[entities[access].member_info].inherited_constructor;
            if (!accessible(access,q.context,entities[access].owner) || !default_constructor_valid(ctor) ||
                (!q.value && !default_destruction_valid(constructed,q.context))) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            auto f = types[entities[ctor].type];
            std::vector<Conversion> chosen;
            for (unsigned i = 0; i < args.size(); ++i) {
                auto c = conversions[recipe.conversions+i];
                if (!valid_fixed_conversion(args[i],0,c,q.context)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            chosen.push_back(c);
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
                if (!c.valid()) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
                if (deleted_transfer(c.function)) return TypeQueryFact::failed(TypeQueryFact::Failure::Deleted);
                if (!valid_fixed_conversion(args[0],0,c,q.context)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
                TypeQueryFact result; result.expression.type = constructed; result.selected = c.function;
                result.expression.conversions = conversions.size(); result.expression.count = 1;
                conversions.push_back(c); return result;
            }
            if (args.size() > 1 || (!args.empty() && !standard_conversion(args[0],constructed).valid()))
                return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            TypeQueryFact result; result.expression.type = constructed; return result;
        }
    }
    if (callee.kind == QueryKind::Member || callee.kind == QueryKind::Destructor) {
        auto x = query_fact(query_edges[callee.offset]).expression;
        auto pointer_type = children[0].arrow ? arrow_chains[children[0].arrow].type : x.type;
        object = callee.op == OP_ARROW ? types[pointer_type].child : x.type;
        category = callee.op == OP_ARROW ? ValueCategory::Lvalue : x.category;
    }
    if (callee.kind == QueryKind::Name && fn.entity && function_binding(fn.entity)) {
        for (auto e : candidates(fn.entity)) if (entities[e].member_info) {
            auto implicit = q.type ? q.type : implicit_object_type(q.context);
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
        auto naming = object_uses[fn.object_use].naming_scope;
        auto choice = select_call(fn.entity,args,0,object,category,naming,
            callee.kind == QueryKind::Destructor ? 0 : callee.arguments,chosen);
        if (choice.failure == CallFailure::NoViable) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
        if (choice.failure == CallFailure::Ambiguous) return TypeQueryFact::failed(TypeQueryFact::Failure::Ambiguous);
        auto selected = choice.entity;
        if (deleted_transfer(selected))
            return TypeQueryFact::failed(TypeQueryFact::Failure::Deleted);
        if (!accessible(selected,q.context,naming,object)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        function_type = entities[selected].type; r.selected = selected;
        if (object && entities[selected].member_info && !entities[selected].is_static) {
            bool qualified = callee.kind == QueryKind::Destructor ? callee.count > 1 :
                callee.kind == QueryKind::Member ? callee.type != 0 : !callee.name && naming;
            record_member_receiver(r.expression,0,object,selected,naming,qualified,q.context,false);
        }
        for (unsigned i = 0; i < args.size(); ++i)
            if (!valid_fixed_conversion(args[i],0,chosen[i+(object!=0)],q.context)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        auto f = types[function_type];
        for (unsigned i = args.size(); i < f.count; ++i) {
            Conversion c; default_argument(selected,i,&c,DefaultReason::Recipe); chosen.push_back(c);
        }
        for (unsigned i = 0; i < f.count; ++i)
            if (abstract_value(types.parameters[f.offset+i])) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        auto count = chosen.size();
        r.expression.conversions = conversions.size(); r.expression.count = count;
        conversions.insert(conversions.end(),chosen.begin(),chosen.end());
    } else {
        function_type = fn.type;
        if (pointer(function_type)) function_type = types[function_type].child;
        auto f = types[function_type];
        if (f.kind != TypeKind::Function || args.size() < f.count || (!f.variadic && args.size() != f.count))
            return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
        for (unsigned i = 0; i < f.count; ++i)
            if (abstract_value(types.parameters[f.offset+i])) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        auto decay_conversion = standard_conversion(fn,decay(fn.type));
        if (!decay_conversion.valid()) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
        record_object(r.expression,0,0,0);
        object_uses[r.expression.object_use].callee_conversion = conversions.size();
        conversions.push_back(decay_conversion);
        std::vector<Conversion> chosen;
        for (unsigned i = 0; i < args.size(); ++i) {
            auto c = i < f.count ? conversion_value(args[i],types.parameters[f.offset+i]) : ellipsis_conversion_value(args[i]);
            if (!c.valid()) return TypeQueryFact::failed(TypeQueryFact::Failure::NoViable);
            if (!valid_fixed_conversion(args[i],0,c,q.context)) return TypeQueryFact::failed(TypeQueryFact::Failure::InvalidOperands);
            chosen.push_back(c);
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
