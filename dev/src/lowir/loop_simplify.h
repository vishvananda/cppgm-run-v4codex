#pragma once
#include "lowir/model.h"
namespace lowir_model {
struct LoopStats {
    std::uint64_t candidates = 0, removed = 0, unrolled = 0, cloned = 0, declined = 0, fills = 0;
};
// Exact affine trip proof in a widened domain, including the final latch step.
bool constant_trip(Type, Operation, Operand start, Operand limit, Operand step,
    bool subtract, std::uint64_t& trips, Operand& final);
bool simplify_loops(Program&, unsigned level, std::uint64_t& work, LoopStats&);
SymbolId fill_runtime(Program&, SymbolId& cached, std::uint64_t& work);
}
