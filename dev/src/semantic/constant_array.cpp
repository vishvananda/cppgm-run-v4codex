#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::constant_array_plan_valid(std::uint32_t plan)
{
    if (!plan) return false;
    if (auto known = constant_array_plans.get(plan)) return known >= 2;
    ++constant_array_work;
    auto action = initializers[plan];
    bool valid = false;
    bool copyable = !(types[action.type].cv & 2);
    if (action.kind == InitKind::String) valid = true;
    else if (action.kind == InitKind::Scalar)
        valid = static_value(action.source,action.type).kind != StaticValue::Invalid;
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
void Analyzer::prepare_constant_array(EntityId e)
{
    // A declaration owns one checked initializer plan. Repeated omitted array
    // elements share one action; neither validation nor classification expands
    // the bound. Only the explicit LowIR data writer visits emitted elements.
    if (constant_arrays.get(e)) return;
    auto plan = initializer_plan(entities[e].initializer,entities[e].type);
    if (!entities[e].initializer || !constant_array_plan_valid(plan))
        throw std::runtime_error("nonconstant constexpr array initializer");
    // Volatile subobjects still need their ordinary observable stores.
    if (constant_array_plans.get(plan) == 3) constant_arrays.put(e,plan);
}
} }
