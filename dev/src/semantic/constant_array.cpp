#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
std::uint32_t Analyzer::constant_array_child(std::uint32_t plan, Constant index)
{
    if (!plan || !index.valid || !integral(index.type)) return 0;
    auto action = initializers[plan]; auto type = types[action.type];
    if (type.kind != TypeKind::Array || index.bits >= type.bound || action.kind != InitKind::Group) return 0;
    auto range_id = constant_array_indices.get(plan);
    if (!range_id) {
        ConstantArrayIndex range; range.first = constant_array_children.size();
        for (auto child = action.first; child; child = initializers[child].next) constant_array_children.push_back(child);
        range.count = constant_array_children.size()-range.first;
        range_id = constant_array_ranges.size(); constant_array_ranges.push_back(range); constant_array_indices.put(plan,range_id);
    }
    auto range = constant_array_ranges[range_id];
    auto begin = constant_array_children.begin()+range.first, end = begin+range.count;
    auto found = std::upper_bound(begin,end,index.bits,[&](std::uint64_t at,std::uint32_t child){ return at < initializers[child].index; });
    if (found == begin) return 0;
    auto child = *--found;
    return index.bits-initializers[child].index < initializers[child].count ? child : 0;
}
std::uint32_t Analyzer::constant_array_projection(NodeId n, ScopeId s)
{
    using syntax::Kind;
    if (ast[n].kind == Kind::Parenthesized) return constant_array_projection(ast[n].first,s);
    if (ast[n].kind == Kind::IdExpression) return constant_arrays.get(expressions[n].entity);
    if (ast[n].kind != Kind::Subscript) return 0;
    auto base = ast[n].first, index = ast[base].next;
    if (types[expressions[base].type].kind != TypeKind::Array) std::swap(base,index);
    return constant_array_child(constant_array_projection(base,s),evaluate(index,s));
}
Constant Analyzer::constant_array_element(std::uint32_t plan, Constant index)
{
    if (!plan || !index.valid || !integral(index.type) || index.bits >= types[initializers[plan].type].bound) return Constant();
    auto action = initializers[plan];
    if (action.kind == InitKind::String) {
        auto literal = ast[action.source].literal;
        if (index.bits < ast.literals[literal].elements) return literal_element(literal,index);
        return convert(Constant(types.fundamental(FT_INT),0),types[action.type].child);
    }
    auto child = constant_array_child(plan,index);
    if (!child) return Constant();
    action = initializers[child];
    if (action.kind == InitKind::Value) return convert(Constant(types.fundamental(FT_INT),0),action.type,true);
    if (action.kind != InitKind::Scalar) return Constant();
    auto value = static_value(action.source,action.type);
    if (value.kind == StaticValue::Floating) return floating_constant(action.type,value.floating);
    return value.kind == StaticValue::Integer ? Constant(action.type,value.bits) : Constant();
}
bool Analyzer::constant_array_plan_valid(std::uint32_t plan)
{
    if (!plan) return false;
    if (auto known = constant_array_plans.get(plan)) return known >= 2;
    ++constant_array_work;
    auto action = initializers[plan];
    bool valid = false;
    bool copyable = !(types[action.type].cv & 2);
    if (action.kind == InitKind::Constructor) {
        auto value = constant_initialize(action.source,action.type,facts[action.source].scope);
        valid = value.valid && constant_persistent(value);
    }
    else if (action.kind == InitKind::String) valid = true;
    else if (action.kind == InitKind::Scalar) {
        auto value = static_value(action.source,action.type);
        auto kind = types[action.type].kind;
        bool reference = kind == TypeKind::LRef || kind == TypeKind::RRef;
        bool address = reference || kind == TypeKind::Pointer;
        valid = value.kind != StaticValue::Invalid;
        if (value.kind == StaticValue::Address) {
            auto object = entities[value.entity];
            valid = address && !object.thread_local_storage && (object.is_static ||
                object.kind == EntityKind::Function || scopes[object.owner].kind == ScopeKind::Namespace);
        } else if (value.kind == StaticValue::String) valid = address;
        else if (reference) valid = false;
        else if (kind == TypeKind::Pointer)
            valid = value.kind == StaticValue::Integer && !value.bits;
    }
    else if (action.kind == InitKind::Value)
        valid = !class_value(action.type) && types[action.type].kind != TypeKind::MemberPointer &&
            !value_constructor(action.type);
    else if (action.kind == InitKind::Group) {
        valid = true;
        for (auto c = action.first; valid && c; c = initializers[c].next) {
            valid = constant_array_plan_valid(c);
            copyable &= constant_array_plans.get(c) == 3;
        }
    }
    if (valid && class_value(action.type)) valid = trivial_destructor(action.type);
    constant_array_plans.put(plan,valid ? (copyable ? 3 : 2) : 1); return valid;
}
void Analyzer::prepare_constant_array(EntityId e, bool required)
{
    // A declaration owns one checked initializer plan. Repeated omitted array
    // elements share one action; neither validation nor classification expands
    // the bound. Only the explicit LowIR data writer visits emitted elements.
    if (constant_arrays.get(e)) return;
    if (!required) {
        TypeId leaf = entities[e].type;
        while (types[leaf].kind == TypeKind::Array) leaf = types[leaf].child;
        // The automatic-data rule covers trivial scalar arrays. Class arrays
        // keep their selected construction and lifetime actions.
        if (class_value(leaf) || (types[leaf].cv & 2)) return;
    }
    auto plan = initializer_plan(entities[e].initializer,entities[e].type);
    if (!entities[e].initializer || !constant_array_plan_valid(plan)) {
        if (required) throw std::runtime_error("nonconstant constexpr array initializer");
        return;
    }
    // Volatile subobjects still need their ordinary observable stores.
    if (constant_array_plans.get(plan) == 3) constant_arrays.put(e,plan);
}
} }
