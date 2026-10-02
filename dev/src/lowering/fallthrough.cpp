#include "lowering/procedural.h"
#include "lowering/flow_proof.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
bool Procedural::reachable_fallthrough(BlockId target)
{
    ++fallthrough_functions;
    const auto blocks = p.functions[function.index-1].blocks;
    const auto first = p.block_order[blocks.begin];
    lowir_model::Range instructions;
    instructions.begin = p.blocks[first.index-1].instructions.begin;
    instructions.count = p.instructions.size()-instructions.begin;
    // All arms returning can leave an isolated join. No path analysis is
    // needed when the continuation has no predecessor at all.
    bool incoming = target == first;
    for (auto n = instructions.begin; !incoming && n < instructions.end(); ++n) {
        ++fallthrough_work;
        const auto& i = p.instructions[n];
        if (i.opcode != Opcode::Jump && i.opcode != Opcode::Branch && i.opcode != Opcode::Switch &&
            i.opcode != Opcode::EhTry && i.opcode != Opcode::EhCleanup) continue;
        for (auto j = i.operands.begin; j < i.operands.end(); ++j)
            incoming |= p.operands[j].kind == Operand::Label && p.operands[j].ref == target.index;
    }
    if (!incoming) return false;
    const auto handlers = flow_exception_targets(p,function,instructions.begin,fallthrough_work,fallthrough_edges);
    return flow_reaches(p,function,target,instructions,handlers,fallthrough_work,fallthrough_edges);
}
} }
