#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::record_deferred_function_use(EntityId target)
{
    // A constexpr-demanded body is checked under the caller's unevaluated
    // depth. Its potentially evaluated uses belong to the body, while deeper
    // sizeof/decltype operands remain unevaluated even if that body is emitted.
    if (!current_function || !target || unevaluated_depth != body_evaluation_depth) return;
    auto identity = key(current_function,target);
    auto id = deferred_function_use_index.get(identity);
    if (!id) {
        DeferredFunctionUse use; use.target = target;
        use.next = deferred_function_use_heads.get(current_function);
        id = deferred_function_uses.size(); deferred_function_uses.push_back(use);
        deferred_function_use_index.put(identity,id); deferred_function_use_heads.put(current_function,id);
    }
    auto& use = deferred_function_uses[id];
    use.direct |= entities[target].member_info && members[entities[target].member_info].emission_reference;
    if (use.processed && use.direct) members[entities[target].member_info].emission_reference = true;
    if (!use.queued && (entities[current_function].emission & Entity::Used)) {
        use.queued = true; deferred_function_use_queue.push_back(id);
    }
}
void Analyzer::activate_deferred_function_uses(EntityId owner)
{
    for (auto id = deferred_function_use_heads.get(owner); id; id = deferred_function_uses[id].next) {
        auto& use = deferred_function_uses[id];
        if (!use.queued) { use.queued = true; deferred_function_use_queue.push_back(id); }
    }
}
} }
