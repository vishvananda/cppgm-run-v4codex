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
    const auto constants = flow_constants(p,instructions,fallthrough_work);
    const auto handlers = flow_exception_targets(p,function,instructions.begin,fallthrough_work,fallthrough_edges);
    // Function lowering is contiguous. Scratch storage covers this function's
    // block identities only and dies before the next body is lowered.
    std::vector<unsigned char> seen(p.blocks.size()-first.index+1);
    std::vector<BlockId> pending;
    auto enqueue = [&](BlockId b) {
        ++fallthrough_edges;
        if (b.index < first.index || p.blocks[b.index-1].owner.index != function.index)
            throw std::logic_error("foreign fallthrough edge");
        auto& mark = seen[b.index-first.index];
        if (!mark) { mark = 1; pending.push_back(b); }
    };
    enqueue(first);
    while (!pending.empty()) {
        auto b = pending.back(); pending.pop_back();
        bool completes = true;
        auto body = p.blocks[b.index-1].instructions;
        for (auto i = body.begin; i < body.end(); ++i) {
            ++fallthrough_work;
            const auto& inst = p.instructions[i];
            const auto begin = inst.operands.begin;
            if (inst.opcode == Opcode::Call) {
                auto signature = inst.signature;
                auto callee = p.operands[begin];
                if (callee.kind == Operand::Symbol) {
                    const auto& symbol = p.symbols[callee.ref-1];
                    if (symbol.kind == lowir_model::Symbol::FunctionSymbol)
                        signature = p.functions[symbol.entity-1].signature;
                }
                if ((!signature || p.signatures[signature.index-1].boundary.unwind != ir_model::CUM_NO) && handlers[i-instructions.begin])
                    enqueue(handlers[i-instructions.begin]);
                if (signature && p.signatures[signature.index-1].boundary.returns == ir_model::CRM_NORETURN) {
                    completes = false; break;
                }
            }
            if ((inst.opcode == Opcode::Resume || inst.opcode == Opcode::Throw) && handlers[i-instructions.begin])
                enqueue(handlers[i-instructions.begin]);
            auto selector = inst.operands.count ? constants.get(p.operands[begin]) : FlowInteger();
            if (inst.opcode == Opcode::Branch && selector.known) {
                enqueue(BlockId(p.operands[begin+(selector.bits ? 1 : 2)].ref));
                continue;
            }
            if (inst.opcode == Opcode::Switch && selector.known) {
                const auto value = selector.bits;
                auto selected = p.operands[begin+1];
                for (auto j = begin+2; j < inst.operands.end(); j += 2)
                    if (p.operands[j].data.integer == value) { selected = p.operands[j+1]; break; }
                enqueue(BlockId(selected.ref)); continue;
            }
            if (inst.opcode != Opcode::Jump && inst.opcode != Opcode::Branch && inst.opcode != Opcode::Switch) continue;
            for (auto j = begin; j < inst.operands.end(); ++j)
                if (p.operands[j].kind == Operand::Label) enqueue(BlockId(p.operands[j].ref));
        }
        if (b == target && completes) return true;
    }
    return false;
}
} }
