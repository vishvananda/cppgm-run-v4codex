#include "native/selection.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
void Selector::analyze_instruction(const lowir_model::Instruction& i, unsigned epoch)
{
    for (unsigned n = i.operands.begin; n != i.operands.end(); ++n) {
        const auto& a = p.operands[n];
        if (a.kind != lowir_model::Operand::Temporary) continue;
        auto& v = state(a.ref);
        ++stats.value_visits;
        ++v.uses;
        v.last = std::max(v.last, position);
        v.crosses_block |= v.block != block_id || i.opcode == Opcode::Phi;
        v.crosses_call |= v.call_epoch != epoch;
    }
}
void Selector::analyze()
{
    unsigned epoch = 0;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& body = p.blocks[block_id-1];
        for (unsigned n = body.instructions.begin; n != body.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            if (i.opcode == Opcode::Call || i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) ++epoch;
            if (!i.destination) continue;
            auto& v = state(i.destination.index);
            v.definition = n+1; v.block = block_id; v.call_epoch = epoch;
            definitions.push_back(i.destination.index);
            if (i.opcode == Opcode::Const && scalar_integer(i.type))
                v.location = Operand::imm(normalize(arg(i,0).data.integer, i.type));
        }
    }
    epoch = 0;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& body = p.blocks[block_id-1];
        for (unsigned n = body.instructions.begin; n != body.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            position = n+1;
            analyze_instruction(i, epoch);
            if (i.opcode == Opcode::Call || i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) ++epoch;
        }
    }
    for (unsigned v : definitions) {
        auto& s = state(v);
        const auto& i = p.instructions[s.definition-1];
        if (i.opcode == Opcode::Compare && s.uses == 1 && s.last == s.definition+1)
            s.compare_branch = p.instructions[s.last-1].opcode == Opcode::Branch;
        if (i.opcode != Opcode::Phi) continue;
        require(scalar_integer(i.type), "native phi class not implemented");
        s.location = home(p.values[v-1].name, i.type, true);
        for (unsigned k = 0; k < i.operands.count; k += 2) {
            EdgeMove move;
            move.pred = arg(i,k).ref; move.target = s.block;
            move.destination = v; move.source = arg(i,k+1);
            move.staging = home(0, i.type, true);
            edge_moves.push_back(move);
        }
    }
    std::sort(edge_moves.begin(), edge_moves.end(), [](const EdgeMove& a, const EdgeMove& b) {
        return a.pred != b.pred ? a.pred < b.pred : a.target < b.target;
    });
}
Operand Selector::home(Name name, Type t, bool temporary)
{
    unsigned alignment = std::max(8u, t.alignment());
    require(alignment <= 16, "overaligned native stack storage not implemented");
    f.frame_bytes = (f.frame_bytes + std::max(8u,t.bytes()) + alignment-1) & ~(std::uint64_t(alignment)-1);
    require(f.frame_bytes < 0x70000000, "native frame too large");
    std::int64_t offset = -std::int64_t(f.frame_bytes);
    f.frame.push_back({name,t,offset,temporary});
    return Operand::mem(XR_RBP, offset);
}
void Selector::parameters()
{
    static const int registers[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
    const auto& signature = p.signatures[source.signature.index-1];
    unsigned ordinal = 0;
    for (unsigned k = signature.parameters.begin; k != signature.parameters.end(); ++k, ++ordinal) {
        const auto& param = p.parameters[k];
        require(scalar_integer(param.type), "native parameter ABI class not implemented");
        Operand incoming = ordinal < 6 ? Operand::r(registers[ordinal]) : Operand::mem(XR_RBP,16+(ordinal-6)*8);
        f.params.push_back({p.values[param.value.index-1].name,param.type,incoming});
        auto& v = state(param.value.index);
        v.location = incoming;
        if (!v.uses) continue;
        // rcx/rdx are fixed-effect scratch. Other argument registers are retained
        // only inside one block and without calls or bulk-memory clobbers.
        bool retain = ordinal < 6 && incoming.reg != XR_RCX && incoming.reg != XR_RDX &&
            !v.crosses_block && !v.crosses_call;
        if (retain) {
            live_until[incoming.reg] = v.last;
            normalize_register(incoming, param.type);
        } else if (ordinal < 6) {
            v.location = home(p.values[param.value.index-1].name,param.type,true);
            move(v.location,incoming,param.type);
        }
    }
}
Operand Selector::allocate(unsigned id, Type t)
{
    auto& v = state(id);
    if (v.location.kind != Operand::None) return v.location;
    require(scalar_integer(t), "native result class not implemented");
    if (v.crosses_block) return v.location = home(p.values[id-1].name,t,true);
    if (v.uses == 1 && v.last == position+1 && p.instructions[v.last-1].opcode == Opcode::Return)
        return v.location = Operand::r(XR_RAX);
    static const int pool[] = {XR_R8,XR_R9,XR_RDI,XR_RSI,XR_RBX,XR_R12,XR_R13,XR_R14,XR_R15};
    for (int reg : pool) {
        bool preserved = reg == XR_RBX || reg >= XR_R12;
        if (v.crosses_call && !preserved) continue;
        if (live_until[reg] >= position) continue;
        live_until[reg] = v.last;
        if (preserved) f.preserved |= 1u << reg;
        return v.location = Operand::r(reg);
    }
    return v.location = home(p.values[id-1].name,t,true);
}
void Selector::finish_frame()
{
    // Reserve save homes below ordinary slots. RBP is always established before
    // accessing any frame operand; calls see a 16-byte-aligned stack.
    unsigned saved = __builtin_popcount(f.preserved);
    f.stack_size = (f.frame_bytes + saved*8 + 15) & ~std::uint64_t(15);
    stats.frame_bytes += f.stack_size;
}
} // namespace native
