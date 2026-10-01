#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::schedule_fold_value(QueryId root)
{
    // Only the reduction spine needs a pack-sized stack. Source operands use
    // their normal evaluator, and completed facts retain the existing complete
    // query/evaluation-mode cache key. A short-circuited operand is never run.
    struct Frame { QueryId id; unsigned phase = 0; explicit Frame(QueryId i) : id(i) {} };
    std::vector<Frame> work; work.emplace_back(root);
    while (!work.empty()) {
        auto& frame = work.back(); auto id = frame.id;
        auto slot = query_value_index.get(query_value_key(id));
        if (slot && query_values[slot].revision == query_revisions.get(id) && query_values[slot].state == FactState::Success) { work.pop_back(); continue; }
        if (!fold_query_nodes.get(id)) { query_value(id); work.pop_back(); continue; }
        auto q = type_queries[id];
        if (!frame.phase) {
            ++fold_value_steps; frame.phase = 1;
            work.emplace_back(query_edges[q.offset]); continue;
        }
        if (frame.phase == 1 && q.kind == QueryKind::Binary) {
            frame.phase = 2; auto fact = query_fact(id); bool second = true;
            if (!fact.selected) {
                auto left = query_edges[q.offset];
                auto value = fact.expression.count == 2 ? constant_query_conversion(left,conversions[fact.expression.conversions]) :
                    constants[query_value(left)];
                second = value.valid && !(q.op == OP_LAND && !constant_truth(value)) && !(q.op == OP_LOR && constant_truth(value));
            }
            if (second) { work.emplace_back(query_edges[q.offset+1]); continue; }
        }
        struct Step {
            QueryId& active; QueryId saved;
            Step(QueryId& a, QueryId id) : active(a), saved(a) { active = id; }
            ~Step() { active = saved; }
        } step(fold_value_step,id);
        query_value(id); work.pop_back();
    }
}
} }
