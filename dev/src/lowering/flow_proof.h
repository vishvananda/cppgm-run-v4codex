#pragma once
#include "lowir/model.h"
namespace cppgm { namespace lowering {
struct FlowInteger {
    enum State : unsigned char { Pending, Constant, Varying } state;
    std::uint64_t bits;
    FlowInteger(State s = Pending, std::uint64_t v = 0) : state(s), bits(v) {}
};
FlowInteger flow_integer(const lowir_model::Instruction&, FlowInteger, FlowInteger);
bool flow_reaches(const lowir_model::Program&, lowir_model::FunctionId, lowir_model::BlockId,
    lowir_model::Range, const std::vector<lowir_model::BlockId>&, std::size_t&, std::size_t&);
std::vector<lowir_model::BlockId> flow_exception_targets(const lowir_model::Program&,
    lowir_model::FunctionId, std::uint32_t first_instruction, std::size_t& work, std::size_t& edges);
} }
