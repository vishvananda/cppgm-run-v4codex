#pragma once
#include "lowir/model.h"
namespace lowir_model {
// Integer literal normalization and folding use target-width unsigned bits.
Operand normalize_integer(Operand, Type);
bool same_scalar(Operand, Operand);
bool fold_integer(const Instruction&, const Operand*, Operand&);
bool discardable(const Instruction&);
void eliminate_local_expressions(Program&, std::uint64_t& work);
void forward_local_slots(Program&, std::uint64_t& work);
void promote_scalar_slots(Program&, std::uint64_t& work);
void simplify_control(Program&, std::uint64_t& work);
void bypass_empty_jumps(Program&, std::uint64_t& work);
void propagate_edge_facts(Program&, std::uint64_t& work);
void simplify_scalars(Program&, std::uint64_t& work);
}
