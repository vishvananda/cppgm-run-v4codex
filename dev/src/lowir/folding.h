#pragma once
#include "lowir/model.h"
namespace lowir_model {
// Integer literal normalization and folding use target-width unsigned bits.
Operand normalize_integer(Operand, Type);
bool same_scalar(Operand, Operand);
// Some uses derive their width/ABI class from the operand rather than the
// instruction. Replacements at those uses must retain that typed carrier.
bool preserves_operand_type(const Program&, const Instruction&, unsigned argument, Operand);
bool fold_integer(const Instruction&, const Operand*, Operand&);
bool fold_floating(const Program&, const Instruction&, const Operand*, Operand&);
bool discardable(const Instruction&);
void propagate_call_constants(Program&, std::uint64_t& work);
void eliminate_local_expressions(Program&, const std::vector<bool>& call_cycles, std::uint64_t& work,
    bool edges = false, const std::vector<bool>* selected = nullptr);
void forward_local_slots(Program&, std::uint64_t& work);
bool split_local_objects(Program&, std::uint64_t& work);
void retire_unused_slots(Program&, std::uint64_t& work);
bool promote_scalar_slots(Program&, const std::vector<bool>& call_cycles, std::uint64_t& work);
void simplify_control(Program&, std::uint64_t& work);
void bypass_empty_jumps(Program&, std::uint64_t& work);
void simplify_diamond_values(Program&, std::uint64_t& work);
void simplify_scalars(Program&, std::uint64_t& work, const std::vector<bool>* selected = nullptr);
}
