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
    TypeQueryFact r;
    TypeId function_type = 0;
    if (fn.entity && function_binding(fn.entity)) {
        struct Candidate { EntityId entity; unsigned offset; };
        std::vector<Candidate> viable; std::vector<Conversion> sequences;
        for (auto e : candidates(fn.entity)) {
            ++candidate_work;
            if (entities[e].template_info) {
                if (callee.arguments) {
                    auto pack = argument_packs[callee.arguments];
                    std::vector<TypeId> explicit_args(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
                    e = specialize(e,explicit_args);
                }
                if (e && entities[e].template_info) e = deduce_function(e,args);
            } else if (callee.arguments) continue;
            if (!e) continue;
            auto f = types[entities[e].type];
            if ((!f.variadic && args.size() > f.count) || (args.size() < f.count &&
                (!entities[e].defaults || !default_arguments[entities[e].defaults+args.size()]))) continue;
            unsigned begin = sequences.size(); bool valid = true;
            if (object) {
                Conversion c; c.rank = 0; c.target = object;
                if (entities[e].member_info && !entities[e].is_static) c = object_conversion(e,object,category);
                valid = c.valid(); sequences.push_back(c);
            } else if (entities[e].member_info && !entities[e].is_static) valid = false;
            for (unsigned i = 0; valid && i < args.size(); ++i) {
                Conversion c;
                if (i < f.count) c = conversion_value(args[i],types.parameters[f.offset+i]);
                else c = ellipsis_conversion_value(args[i]);
                valid = c.valid(); sequences.push_back(c);
            }
            if (valid) viable.push_back({e,begin}); else sequences.resize(begin);
        }
        if (viable.empty()) throw std::runtime_error("no viable function in type query");
        auto better_candidate = [&](unsigned a, unsigned b) {
            auto x = sequences.data()+viable[a].offset, y = sequences.data()+viable[b].offset;
            auto count = args.size()+(object!=0);
            if (better(x,y,count)) return true;
            if (better(y,x,count)) return false;
            return (!entities[viable[a].entity].specialization && entities[viable[b].entity].specialization) ||
                template_more_specialized(viable[a].entity,viable[b].entity);
        };
        unsigned best = 0;
        for (unsigned i = 1; i < viable.size(); ++i) if (better_candidate(i,best)) best = i;
        for (unsigned i = 0; i < viable.size(); ++i)
            if (i != best && viable[i].entity != viable[best].entity && !better_candidate(best,i))
                throw std::runtime_error("ambiguous function in type query");
        auto selected = viable[best].entity;
        if (entities[selected].member_info && members[entities[selected].member_info].deleted)
            throw std::runtime_error("deleted function in type query");
        check_access(selected,q.context,entities[selected].owner,object);
        function_type = entities[selected].type; r.selected = selected;
        auto begin = viable[best].offset; auto count = args.size()+(object!=0);
        r.expression.conversions = conversions.size(); r.expression.count = count;
        conversions.insert(conversions.end(),sequences.begin()+begin,sequences.begin()+begin+count);
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
