#include "native/selection.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
unsigned Selector::clobbers(const lowir_model::Instruction& i) const
{
    if (i.opcode == Opcode::Call || i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit)
        return (1u<<XR_RDI)|(1u<<XR_RSI)|(1u<<XR_RDX)|(1u<<XR_RCX)|(1u<<XR_R8)|(1u<<XR_R9);
    if (i.type == Type::I128 && i.opcode >= Opcode::AtomicLoad && i.opcode <= Opcode::AtomicCompareExchange)
        return (1u<<XR_RCX)|(1u<<XR_RDX);
    if (i.type == Type::I128 && i.operation == Operation::Mul) return 1u<<XR_RDX;
    if (i.opcode == Opcode::Binary) {
        if (i.operation == Operation::Div || i.operation == Operation::Udiv || i.operation == Operation::Mod || i.operation == Operation::Umod) return 1u<<XR_RDX;
        if ((i.operation == Operation::Shl || i.operation == Operation::Shr || i.operation == Operation::Ushr) && !arg(i,1).literal()) return 1u<<XR_RCX;
    }
    return i.opcode == Opcode::AtomicCompareExchange ? 1u<<XR_RCX : 0;
}
void Selector::parameter_availability()
{
    // Fewer than five parameters cannot exhaust the preserved-register budget
    // that selects incoming-carrier/home placement. Do no unused CFG analysis.
    if (p.signatures[source.signature.index-1].parameters.count < 5) return;
    // Monotonic six-bit flow facts. A block is queued only when a predecessor
    // contributes a new clobber bit; at most six changes per edge. Incoming
    // homes remain valid through joins and loop backedges.
    unsigned count = source.blocks.count;
    auto& local = workspace.block_local;
    std::vector<unsigned> effects(count), pending;
    std::vector<bool> queued(count,true);
    for (unsigned b = 0; b < count; ++b) { local[p.block_order[source.blocks.begin+b].index] = b; pending.push_back(b); }
    for (unsigned b = 0; b < count; ++b) {
        const auto& block = p.blocks[p.block_order[source.blocks.begin+b].index-1];
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n) effects[b] |= clobbers(p.instructions[n]);
    }
    for (unsigned at = 0; at < pending.size(); ++at) {
        ++stats.parameter_flow_visits;
        unsigned b = pending[at], id = p.block_order[source.blocks.begin+b].index;
        queued[b] = false;
        unsigned out = workspace.parameter_clobbers[id] | effects[b];
        const auto& term = p.instructions[p.blocks[id-1].instructions.end()-1];
        for (unsigned k = 0; k < term.operands.count; ++k) {
            auto a = arg(term,k);
            if (a.kind != lowir_model::Operand::Label) continue;
            auto& incoming = workspace.parameter_clobbers[a.ref];
            if ((incoming|out) == incoming) continue;
            incoming |= out;
            unsigned to = local[a.ref];
            if (!queued[to]) { queued[to] = true; pending.push_back(to); }
        }
    }
}
} // namespace native
