#pragma once
#include "lowir/model.h"
namespace lowir_model {
// Immutable admission over the input bodies. IDs, never spellings, identify
// callees. The cloner owns actual growth reservations and recursion checks.
struct InlinePolicy {
    std::vector<bool> eligible, single;
    std::vector<unsigned> growth;
    std::uint64_t unit_work = 0, function_work = 32768;
};
void expand_optional_calls(Program&, const InlinePolicy&);
bool inline_small_calls(Program&, unsigned level, std::uint64_t& work);
void prune_support_functions(Program&, std::uint64_t& work);
}
