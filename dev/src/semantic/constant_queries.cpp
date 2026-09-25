#include "semantic/analyzer.h"
#include <algorithm>

namespace cppgm { namespace semantic {
std::uint32_t Analyzer::constant_query_arrow(QueryId id, std::uint32_t chain)
{
    auto type = query_fact(id).expression.type;
    auto value = pointer(type) ? constants[query_value(id)] : Constant();
    auto object = pointer(type) ? (value.valid ? value.bits : 0) : constant_query_object(id);
    return constant_arrow_value(object,chain);
}
std::uint32_t Analyzer::constant_query_object(QueryId id)
{
    if (auto known = constant_query_receivers.get(id)) return known;
    auto q = type_queries[id]; auto fact = query_fact(id);
    if (q.kind == QueryKind::Value && types[q.type].kind == TypeKind::LRef) return q.value;
    if (q.kind == QueryKind::Parenthesized) return constant_query_object(query_edges[q.offset]);
    if (q.kind == QueryKind::String) {
        auto literal = std::uint32_t(q.value);
        if (auto known = constant_literal_storage.get(literal)) return known;
        auto source = query_literal_sources.get(literal);
        auto result = constant_storage_address(fact.expression.type,Constant(),0,source,true);
        constant_literal_storage.put(literal,result); return result;
    }
    if (q.kind == QueryKind::Name || q.kind == QueryKind::QualifiedValue) return constant_entity_address(fact.expression.entity);
    if (q.kind == QueryKind::Member) {
        auto operand = query_edges[q.offset];
        auto parent = q.op == OP_ARROW ? constant_query_arrow(operand,fact.arrow) : constant_query_object(operand);
        if (!parent) return 0;
        if (!nonstatic_field(fact.expression.entity)) return constant_entity_address(fact.expression.entity);
        auto field = fact.expression.entity;
        auto use = object_uses[fact.expression.object_use];
        parent = constant_base_projection(constant_base_projection(parent,use.qualifier_adjustment),use.adjustment);
        auto address = constant_subobject(parent,entities[field].type,field);
        auto kind = types[entities[field].type].kind;
        if (kind == TypeKind::LRef || kind == TypeKind::RRef) {
            auto value = constant_read(address); return value.valid ? value.bits : 0;
        }
        return address;
    }
    if (!fact.selected && q.kind == QueryKind::Unary && q.op == OP_STAR) {
        auto value = constants[query_value(query_edges[q.offset])];
        return value.valid && pointer(value.type) ? value.bits : 0;
    }
    if (!fact.selected && q.kind == QueryKind::Binary && q.op == OP_LSQUARE) {
        auto left = query_edges[q.offset], right = query_edges[q.offset+1];
        auto begin = fact.expression.conversions;
        auto a = constant_query_conversion(left,conversions[begin]);
        auto b = constant_query_conversion(right,conversions[begin+1]);
        auto address = constant_pointer_binary(OP_PLUS,a,b);
        return address.valid ? address.bits : 0;
    }
    auto v = constants[query_value(id)];
    if (!v.valid) return 0;
    if (types[v.type].kind == TypeKind::LRef || types[v.type].kind == TypeKind::RRef) return v.bits;
    auto result = constant_storage_address(v.type,v);
    constant_query_receivers.put(id,result); return result;
}
Constant Analyzer::constant_query_conversion(QueryId source, Conversion c)
{
    if (c.kind == Conversion::Kind::QueryList) return constant_query_list(c.materialization);
    if (c.constant_forbidden || c.ellipsis_unavailable) return Constant();
    if (c.function && types[c.target].kind == TypeKind::MemberPointer) return Constant(c.target,c.function);
    if (c.kind == Conversion::Kind::User) {
        auto object = constant_query_object(source);
        object = constant_base_address(object,entities[scopes[entities[c.function].owner].entity].type);
        if (!object || members[entities[c.function].member_info].virtual_member) return Constant();
        return constant_result_conversion(execute_constant(c.function,{},object),c);
    }
    if (c.kind == Conversion::Kind::Construction) {
        if (c.ellipsis_object && !literal_type(value_type(c.target))) return Constant();
        auto material = conversion_objects[c.materialization];
        auto call = material.call;
        if (call.argument_count != 1) return Constant();
        auto v = constant_query_conversion(source,conversions[call.conversions]);
        if (!v.valid) return Constant();
        std::vector<Constant> args{v};
        auto f = types[entities[material.constructor].type];
        for (unsigned i = 1; c.ellipsis_object && i < f.count; ++i) {
            Conversion argument;
            auto n = default_argument(material.constructor,i,&argument,DefaultReason::Recipe);
            auto value = constant_node_conversion(n,argument,facts[n].scope);
            if (!value.valid) return Constant();
            args.push_back(value);
        }
        return constant_construct(material.constructor,args);
    }
    auto t = types[c.target]; auto from = query_fact(source).expression.type;
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef ||
        (t.kind == TypeKind::Pointer && (types[from].kind == TypeKind::Array || types[from].kind == TypeKind::Function))) {
        if (c.temporary) {
            auto scalar = c; scalar.target = t.child; scalar.reference = scalar.temporary = false;
            auto value = constant_query_conversion(source,scalar);
            return value.valid ? Constant(c.target,constant_storage_address(t.child,value)) : Constant();
        }
        auto address = constant_query_object(source);
        if (t.kind == TypeKind::Pointer && types[from].kind == TypeKind::Array) address = constant_subobject(address,t.child,0);
        if (class_value(t.child)) address = constant_base_address(address,t.child);
        return address ? Constant(c.target,address) : Constant();
    }
    return convert(constants[query_value(source)],c.target,true);
}
Constant Analyzer::constant_query_call(QueryId id)
{
    auto q = type_queries[id]; auto fact = query_fact(id);
    auto e = fact.selected;
    auto callee_id = query_edges[q.offset]; auto callee = type_queries[callee_id];
    while (callee.kind == QueryKind::Parenthesized) {
        callee_id = query_edges[callee.offset]; callee = type_queries[callee_id];
    }
    if (!e || fact.surrogate) {
        auto conversion = fact.surrogate ? fact.expression.conversions : object_uses[fact.expression.object_use].callee_conversion;
        auto value = conversion ? constant_query_conversion(callee_id,conversions[conversion]) : constants[query_value(callee_id)];
        if (value.valid && pointer(value.type) && value.bits)
            e = constant_storage[constant_addresses[value.bits].storage].entity;
        else return Constant();
    }
    if (!e || (!entities[e].constexpr_function && !synthetic_member(e))) return Constant();
    std::uint32_t object = 0;
    unsigned first_argument = q.kind == QueryKind::Call ? 1 : 0;
    if (entities[e].member_info && !entities[e].is_static && !constructor_member(e)) {
        if (members[entities[e].member_info].virtual_member) return Constant();
        if (q.kind == QueryKind::Call && callee.kind == QueryKind::Member) {
            auto receiver = query_edges[callee.offset];
            object = callee.op == OP_ARROW ? constant_query_arrow(receiver,query_fact(callee_id).arrow) : constant_query_object(receiver);
        } else { object = constant_query_object(callee_id); first_argument = 1; }
        if (fact.expression.object_use) {
            auto use = object_uses[fact.expression.object_use];
            object = constant_base_projection(constant_base_projection(object,use.qualifier_adjustment),use.adjustment);
        } else object = constant_base_address(object,entities[scopes[entities[e].owner].entity].type);
        if (!object) return Constant();
    } else if (q.kind == QueryKind::Call && callee.kind == QueryKind::Member) {
        auto receiver = query_edges[callee.offset];
        if (!(callee.op == OP_ARROW ? constant_query_arrow(receiver,query_fact(callee_id).arrow) : constant_query_object(receiver))) return Constant();
    }
    auto count = std::max<unsigned>(types[entities[e].type].count,q.count-first_argument);
    auto offset = fact.expression.count-count;
    std::vector<Constant> args;
    for (unsigned i = 0; i < count; ++i) {
        auto c = conversions[fact.expression.conversions+offset+i];
        Constant value;
        if (i+first_argument < q.count) value = constant_query_conversion(query_edges[q.offset+i+first_argument],c);
        else { auto node = default_argument(e,i,0,DefaultReason::Recipe); value = constant_node_conversion(node,c,facts[node].scope); }
        value = convert(value,c.target);
        if (!value.valid) return Constant();
        args.push_back(value);
    }
    if (constructor_member(e)) return constant_construct(e,args,q.count == 1);
    return execute_constant(e,args,object);
}
} }
