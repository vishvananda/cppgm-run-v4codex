#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
std::uint32_t Procedural::protect_deallocation(EntityId callee, const Operand* operands, unsigned count)
{
    close_expression_region();
    auto signature = sem.types[sem.entities[callee].type];
    ReleaseAction action{callee,static_cast<std::uint32_t>(release_operands.size()),count};
    for (unsigned j = 0; j < count; ++j) {
        auto t = type(sem.types.parameters[signature.offset+j]);
        auto slot = builder->add_slot(0,t);
        emit(Opcode::Store,t,{operands[j],Operand::slot(slot)});
        release_operands.push_back({slot,t});
    }
    release_actions.push_back(action);
    TemporaryState state; state.release = release_actions.size(); state.tail = live;
    state.depth = lifetime_state(live).depth+1;
    temporary_states.push_back(state);
    return live = 0x80000000u | temporary_states.size();
}
void Procedural::release_allocation(std::uint32_t id)
{
    // Allocation and argument identities are retained at evaluation time.
    // Unwind consumes saved slots, without repeating lookup or expressions.
    auto action = release_actions[id-1];
    auto begin = call_work.size(); call_work.push_back(Operand::symbol(symbol(action.function)));
    for (unsigned j = 0; j < action.count; ++j) {
        auto operand = release_operands[action.begin+j];
        call_work.push_back(emit(Opcode::Load,operand.type,{Operand::slot(operand.slot)}).operand);
    }
    emit(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
    call_work.resize(begin);
}
void Procedural::retire_deallocation(std::uint32_t state, std::uint32_t initial)
{
    close_expression_region();
    semantic::Index retired, cache; retired.put(state,1);
    live = retire_construction(live,initial,retired,cache);
}
} }
