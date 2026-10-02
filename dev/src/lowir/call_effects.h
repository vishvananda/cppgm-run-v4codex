#pragma once
#include "lowir/model.h"
namespace lowir_model {
FunctionBoundaryMetadata call_boundary(const Program&, const Instruction&);
// Reverse call edges drive monotonic no-unwind publication and dead-region
// retirement. Uncertain handler stacks or exhausted work retain the region.
void simplify_call_regions(Program&, std::uint64_t& work);
}
