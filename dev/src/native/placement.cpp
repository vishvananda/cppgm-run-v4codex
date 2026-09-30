#include "native/selection.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
void Selector::initialize_values()
{
    // Only the compact ID-to-local-index table spans the unit. Placement facts
    // and live locations belong to this function and die after its emission.
    values.push_back(ValueState());
    auto insert = [&](unsigned id) {
        workspace.value_indices[id] = values.size(); values.push_back(ValueState());
    };
    const auto& sig = p.signatures[source.signature.index-1];
    for (unsigned k = sig.parameters.begin; k != sig.parameters.end(); ++k) insert(p.parameters[k].value.index);
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        const auto& block = p.blocks[p.block_order[b].index-1];
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n)
            if (p.instructions[n].destination) insert(p.instructions[n].destination.index);
    }
}
void Selector::analyze_instruction(const lowir_model::Instruction& i, unsigned epoch)
{
    if (i.destination && state(i.destination.index).alias) return;
    for (unsigned n = i.operands.begin; n != i.operands.end(); ++n) {
        const auto& a = p.operands[n];
        if (a.kind != lowir_model::Operand::Temporary) continue;
        auto& v = state(root(a.ref));
        ++stats.value_visits;
        ++v.uses;
        unsigned use_block = block_id, use_position = position, use_epoch = epoch;
        if (i.opcode == Opcode::Phi) {
            use_block = p.operands[n-1].ref;
            use_position = p.blocks[use_block-1].instructions.end();
            use_epoch = workspace.block_epochs[use_block];
        }
        v.last = std::max(v.last,use_position);
        v.crosses_block |= v.block != use_block;
        if (v.block != use_block) {
            if (!v.other_block) v.other_block = use_block;
            else if (v.other_block != use_block) v.other_block = ~0u;
        }
        v.crosses_call |= v.call_epoch != use_epoch;
        unsigned index = n-i.operands.begin;
        bool address = (i.opcode == Opcode::Load && index == 0) || (i.opcode == Opcode::Store && index == 1) ||
            (i.opcode == Opcode::Index && index == 0) || i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit;
        v.address_only &= address;
    }
}
void Selector::analyze()
{
    first_clobber.fill(~0u);
    promote_parameters();
    control_edges();
    unsigned epoch = 0;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& body = p.blocks[block_id-1];
        for (unsigned n = body.instructions.begin; n != body.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            unsigned clobbers = 0;
            if (i.opcode == Opcode::Binary && (i.operation == Operation::Div || i.operation == Operation::Udiv || i.operation == Operation::Mod || i.operation == Operation::Umod)) clobbers |= 1u<<XR_RDX;
            if (i.opcode == Opcode::Binary && (i.operation == Operation::Shl || i.operation == Operation::Shr || i.operation == Operation::Ushr) && !arg(i,1).literal()) clobbers |= 1u<<XR_RCX;
            if (i.opcode == Opcode::AtomicCompareExchange) clobbers |= 1u<<XR_RCX;
            for (unsigned r = 0; r < 16; ++r) if (clobbers & (1u<<r)) first_clobber[r] = std::min(first_clobber[r],n+1);
            if (i.opcode == Opcode::Call || i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) ++epoch;
            if (!i.destination) continue;
            auto& v = state(i.destination.index);
            v.definition = n+1; v.block = block_id; v.call_epoch = epoch;
            definitions.push_back(i.destination.index);
            if (i.opcode == Opcode::Const && scalar_integer(i.type))
                v.location = Operand::imm(normalize(arg(i,0).data.integer, i.type));
        }
    }
    // Epochs at predecessor exits place phi uses on incoming edges.
    epoch = 0;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        unsigned id = p.block_order[b].index;
        for (unsigned n = p.blocks[id-1].instructions.begin; n != p.blocks[id-1].instructions.end(); ++n) {
            auto op = p.instructions[n].opcode;
            if (op == Opcode::Call || op == Opcode::CopyObject || op == Opcode::ZeroInit) ++epoch;
        }
        workspace.block_epochs[id] = epoch;
    }
    aliases();
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
    folds();
    for (unsigned v : definitions) {
        auto& s = state(v);
        const auto& i = p.instructions[s.definition-1];
        s.single_edge = s.other_block && s.other_block != ~0u &&
            workspace.successor[s.block] == s.other_block && workspace.next_block[s.block] == s.other_block &&
            workspace.predecessor_count[s.other_block] == 1;
        if ((i.opcode == Opcode::Compare || (i.opcode == Opcode::Unary && i.operation == Operation::Not)) && s.uses == 1 && s.last == s.definition+1)
            s.compare_branch = p.instructions[s.last-1].opcode == Opcode::Branch;
        if (i.opcode != Opcode::Phi) continue;
        require(scalar_integer(i.type) || i.type.floating(), "native phi class not implemented");
        s.location = home(p.values[v-1].name, i.type, true);
        for (unsigned k = 0; k < i.operands.count; k += 2) {
            EdgeMove move;
            move.pred = arg(i,k).ref; move.target = s.block;
            move.destination = v; move.source = arg(i,k+1);
            if (move.source.kind == lowir_model::Operand::Temporary) {
                unsigned input = root(move.source.ref);
                auto& origin = state(input);
                if (origin.block == move.target && origin.definition &&
                    p.instructions[origin.definition-1].opcode == Opcode::Phi && input != v)
                    move.staging = home(0,i.type,true);
            }
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
    Operand storage = Operand::mem(XR_RBP,offset); storage.temporary = temporary; return storage;
}
void Selector::parameters()
{
    static const int registers[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
    const auto& signature = p.signatures[source.signature.index-1];
    if (signature.boundary.arity == CAM_VARIADIC) save_variadic_registers();
    unsigned gp = 0, fp = 0, stack = 16;
    for (unsigned k = signature.parameters.begin; k != signature.parameters.end(); ++k) {
        const auto& param = p.parameters[k];
        require(scalar_integer(param.type) || param.type.floating(), "native parameter ABI class not implemented");
        bool vector = param.type == Type::F32 || param.type == Type::F64;
        Operand incoming;
        if (vector && fp < 8) incoming = Operand::r(xmm(fp++));
        else if (scalar_integer(param.type) && gp < 6) incoming = Operand::r(registers[gp++]);
        else {
            unsigned alignment = std::max(8u,param.type.alignment());
            stack = (stack+alignment-1)&~(alignment-1);
            incoming = Operand::mem(XR_RBP,stack); stack += std::max(8u,param.type.bytes());
        }
        f.params.push_back({p.values[param.value.index-1].name,param.type,incoming});
        auto& v = state(param.value.index);
        v.location = incoming;
        if (!v.uses) continue;
        bool retain = incoming.kind == Operand::Reg &&
            (vector || v.last < first_clobber[incoming.reg]) && !v.crosses_block && !v.crosses_call;
        if (retain) {
            live_until[incoming.reg] = v.last;
            normalize_register(incoming, param.type);
        } else if (incoming.kind == Operand::Reg) {
            if (!vector) {
                int preserved = -1;
                for (int r : {XR_RBX,XR_R12,XR_R13,XR_R14,XR_R15}) if (!live_until[r]) { preserved = r; break; }
                if (preserved >= 0) {
                    v.location = Operand::r(preserved);
                    f.preserved |= 1u<<preserved; live_until[preserved] = ~0u;
                    move(v.location,incoming,param.type); normalize_register(v.location,param.type); continue;
                }
            }
            v.location = home(p.values[param.value.index-1].name,param.type,true);
            normalize_register(incoming,param.type); move(v.location,incoming,param.type);
        }
    }
    vararg_gp = gp*8; vararg_fp = 48+fp*16; vararg_stack = stack;
}
Operand Selector::allocate(unsigned id, Type t)
{
    auto& v = state(id);
    if (v.location.kind != Operand::None) return v.location;
    if (t.floating()) {
        f.scratch_bytes = 48;
        if (t != Type::F80 && !v.crosses_call && (!v.crosses_block || v.single_edge)) {
            for (int reg = xmm(0); reg < xmm(14); ++reg) if (live_until[reg] < position) {
                live_until[reg] = v.last; return v.location = Operand::r(reg);
            }
        }
        return v.location = home(p.values[id-1].name,t,true);
    }
    require(scalar_integer(t), "native result class not implemented");
    if (v.crosses_block && !v.single_edge) return v.location = home(p.values[id-1].name,t,true);
    const auto& definition = p.instructions[v.definition-1];
    if (definition.opcode == Opcode::Binary || definition.opcode == Opcode::Unary) {
        auto input = arg(definition,0);
        if (input.kind == lowir_model::Operand::Temporary) {
            auto& previous = state(root(input.ref));
            auto loc = previous.location;
            bool survives = loc.reg == XR_RBX || loc.reg >= XR_R12;
            if (loc.kind == Operand::Reg && previous.last == position && live_until[loc.reg] == position && (!v.crosses_call || survives) && loc.reg != XR_RAX) {
                live_until[loc.reg] = v.last;
                return v.location = loc;
            }
        }
    }
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
    f.stack_size = (f.frame_bytes + saved*8 + f.scratch_bytes + 15) & ~std::uint64_t(15);
    stats.frame_bytes += f.stack_size;
}
} // namespace native
