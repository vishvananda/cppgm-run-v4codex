#pragma once
#include "lowir/model.h"
namespace cppgm { namespace lowering {
struct FlowInteger { std::uint64_t bits = 0; bool known = false; };
struct FlowConstants {
    std::uint32_t first = 0;
    std::vector<FlowInteger> values;
    FlowInteger get(const lowir_model::Operand& operand) const;
};
FlowConstants flow_constants(const lowir_model::Program&, lowir_model::Range, std::size_t& work);
std::vector<lowir_model::BlockId> flow_exception_targets(const lowir_model::Program&,
    lowir_model::FunctionId, std::uint32_t first_instruction, std::size_t& work, std::size_t& edges);
} }
