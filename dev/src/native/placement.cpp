#include "native/selection.h"
#include "native/abi.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
void Selector::initialize_values()
{
    // Only the compact ID-to-local-index table spans the unit. Placement facts
    // and live locations belong to this function and die after its emission.
    values.push_back(ValueState());
    auto insert = [&](unsigned id) {
        if (workspace.value_indices[id]) { ++state(id).writes; return; }
        f.frame_alignment = std::max(f.frame_alignment,p.values[id-1].type.alignment());
        workspace.value_indices[id] = values.size(); values.push_back(ValueState());
    };
    const auto& sig = p.signatures[source.signature.index-1];
    bool integer_parameter = false, float_parameter = false, numeric_conversion = false;
    for (unsigned k = sig.parameters.begin; k != sig.parameters.end(); ++k) {
        integer_parameter |= scalar_integer(p.parameters[k].type);
        float_parameter |= p.parameters[k].type.floating();
    }
    for (unsigned k = sig.parameters.begin; k != sig.parameters.end(); ++k) insert(p.parameters[k].value.index);
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        const auto& block = p.blocks[p.block_order[b].index-1];
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            numeric_conversion |= i.opcode == Opcode::Convert && i.source_type.floating() && i.type.integer();
            if (p.instructions[n].destination) insert(p.instructions[n].destination.index);
        }
    }
    // The canonical O0 mixed conversion boundary keeps FP inputs in explicit
    // parameter homes and integer inputs in dedicated incoming carriers. r8
    // is outside this conservative conversion placement pool.
    mixed_conversion_abi = integer_parameter && float_parameter && numeric_conversion;
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
    index_exception_clauses();
    first_clobber.fill(~0u);
    promote_parameters();
    control_edges();
    parameter_availability();
    unsigned epoch = 0;
    bool dynamic_stack = false, handlers = false;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& body = p.blocks[block_id-1];
        for (unsigned n = body.instructions.begin; n != body.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            dynamic_stack |= i.opcode == Opcode::StackAlloc;
            handlers |= (i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count;
            // Calls/bulk boundaries already have interval epochs. Fixed effects
            // share their owner with incoming-parameter CFG flow.
            unsigned clobbers = call_effect(i) ? 0 : this->clobbers(i);
            for (unsigned r = 0; r < 16; ++r) if (clobbers & (1u<<r)) first_clobber[r] = std::min(first_clobber[r],n+1);
            if (call_effect(i)) ++epoch;
            if (!i.destination) continue;
            auto& v = state(i.destination.index);
            v.definition = n+1; v.block = block_id; v.call_epoch = epoch;
            definitions.push_back(i.destination.index);
            if (i.opcode == Opcode::Const && scalar_integer(i.type) && v.writes == 1)
                v.location = Operand::imm(normalize(arg(i,0).data.integer, i.type));
        }
    }
    if (handlers) {
        if (f.host) {
            f.host_exception = home(0,Type::Ptr,true); f.host_selector = home(0,Type::I64,true);
            f.host_raw_selector = f.host_selector; f.host_raw_selector.displacement += 4;
        }
        else f.exception_base = home(0,Type::Ptr,true);
        if (dynamic_stack) f.stack_floor = home(0,Type::Ptr,true);
    }
    // Epochs at predecessor exits place phi uses on incoming edges.
    epoch = 0;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        unsigned id = p.block_order[b].index;
        for (unsigned n = p.blocks[id-1].instructions.begin; n != p.blocks[id-1].instructions.end(); ++n) {
            if (call_effect(p.instructions[n])) ++epoch;
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
            if (call_effect(i)) ++epoch;
        }
    }
    folds();
    for (unsigned v : definitions) {
        auto& s = state(v);
        if (s.writes > 1) continue;
        const auto& i = p.instructions[s.definition-1];
        s.single_edge = s.other_block && s.other_block != ~0u &&
            workspace.successor[s.block] == s.other_block && workspace.next_block[s.block] == s.other_block &&
            workspace.predecessor_count[s.other_block] == 1;
        if ((i.opcode == Opcode::Compare || (i.opcode == Opcode::Unary && i.operation == Operation::Not)) && s.uses == 1 && s.last == s.definition+1)
            s.compare_branch = p.instructions[s.last-1].opcode == Opcode::Branch;
        if (i.opcode != Opcode::Phi) continue;
        require(i.type.scalar() || i.type.kind() == Type::Object, "invalid native phi class");
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
    unsigned alignment = t == Type::I128 ? 16 : std::max(8u, t.alignment());
    require(alignment <= f.frame_alignment, "unplanned native frame alignment");
    f.frame_bytes = (f.frame_bytes + std::max(8u,t.bytes()) + alignment-1) & ~(std::uint64_t(alignment)-1);
    require(f.frame_bytes < 0x70000000, "native frame too large");
    std::int64_t offset = -std::int64_t(f.frame_bytes);
    f.frame.push_back({name,t,offset,temporary,false});
    Operand storage = Operand::mem(f.frame_base,offset); storage.temporary = temporary;
    storage.id = f.frame.size(); return storage;
}
void Selector::parameters()
{
    const auto& signature = p.signatures[source.signature.index-1];
    if (signature.boundary.arity == CAM_VARIADIC) save_variadic_registers();
    unsigned retained_parameters = 0;
    for (unsigned k = signature.parameters.begin; k != signature.parameters.end(); ++k) {
        const auto& param = p.parameters[k];
        const auto& v = state(param.value.index);
        if (scalar_integer(param.type) && v.uses && (v.crosses_block || v.crosses_call)) ++retained_parameters;
    }
    AbiCursor abi; abi.stack = abi.stack_origin = 16;
    if (indirect_return(f.result)) {
        abi.gp = 1; indirect_result = home(0,Type::Ptr,true);
        move(indirect_result,Operand::r(XR_RDI),Type::Ptr);
    }
    for (unsigned k = signature.parameters.begin; k != signature.parameters.end(); ++k) {
        const auto& param = p.parameters[k];
        bool vector = param.type.floating() && param.type != Type::F80;
        auto placement = abi.take(param.type,XR_RBP);
        Operand incoming = placement.parts[0];
        f.params.push_back({p.values[param.value.index-1].name,param.type,incoming,placement.count == 2 ? placement.parts[1] : Operand()});
        auto& v = state(param.value.index);
        if (v.writes > 1) {
            v.location = home(p.values[param.value.index-1].name,param.type,true);
            if (aggregate(param.type) && !placement.memory) {
                for (unsigned part = 0; part < placement.count; ++part)
                    move(fragment(v.location,part*8),placement.parts[part],abi_chunk_type(param.type,part));
            } else move(v.location,incoming,param.type);
            continue;
        }
        v.location = incoming;
        if (!v.uses) continue;
        if (param.type.scalar()) parameter_bytes += (param.type.bytes()+7)&~std::uint64_t(7);
        if (aggregate(param.type)) {
            if (!placement.memory) {
                v.location = home(p.values[param.value.index-1].name,param.type,true);
                f.frame.back().parameter = true;
                for (unsigned part = 0; part < placement.count; ++part)
                    move(fragment(v.location,part*8),placement.parts[part],abi_chunk_type(param.type,part));
            }
            continue;
        }
        if (placement.memory && scalar_integer(param.type)) {
            v.location = home(p.values[param.value.index-1].name,param.type,true);
            f.frame.back().parameter = true;
            // ABI homes are stable storage, not private reload-carry windows.
            v.location.temporary = false;
            move(Operand::r(XR_RAX),incoming,param.type);
            move(v.location,Operand::r(XR_RAX),param.type);
            continue;
        }
        bool retain = incoming.kind == Operand::Reg && !(vector && mixed_conversion_abi) &&
            (vector || v.last < first_clobber[incoming.reg]) && !v.crosses_block && !v.crosses_call;
        if (retain) {
            live_until[incoming.reg] = mixed_conversion_abi ? ~0u : v.last;
            normalize_register(incoming, param.type);
        } else if (incoming.kind == Operand::Reg) {
            if (!vector && v.crosses_block && v.crosses_call && retained_parameters >= 5) {
                v.location = home(p.values[param.value.index-1].name,param.type,true);
                f.frame.back().parameter = true;
                normalize_register(incoming,param.type); move(v.location,incoming,param.type);
                // This incoming carrier remains reserved while any path can
                // consume it. After clobbers, reads use the immutable home.
                live_until[incoming.reg] = ~0u;
                continue;
            }
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
            f.frame.back().parameter = vector;
            normalize_register(incoming,param.type); move(v.location,incoming,param.type);
        }
    }
    vararg_gp = abi.gp*8; vararg_fp = 48+abi.fp*16; vararg_stack = abi.stack;
    if (mixed_conversion_abi) live_until[XR_R8] = ~0u;
}
Operand Selector::allocate(unsigned id, Type t)
{
    auto& v = state(id);
    if (v.location.kind != Operand::None) return v.location;
    if (v.writes > 1 || aggregate(t)) return v.location = home(p.values[id-1].name,t,true);
    if (t.floating()) {
        f.scratch_bytes = 48;
        if (t != Type::F80 && !v.crosses_call && (!v.crosses_block || v.single_edge)) {
            const auto& definition = p.instructions[v.definition-1];
            if (definition.opcode == Opcode::Binary || definition.opcode == Opcode::Unary) {
                auto input = arg(definition,0);
                if (input.kind == lowir_model::Operand::Temporary) {
                    const auto& previous = state(root(input.ref));
                    auto loc = previous.location;
                    if (loc.kind == Operand::Reg && loc.reg >= xmm(0) && loc.reg < xmm(14) &&
                        previous.last == position && live_until[loc.reg] == position) {
                        ++stats.xmm_reuses;
                        live_until[loc.reg] = v.last; return v.location = loc;
                    }
                }
            }
            for (int reg = xmm(0); reg < xmm(14); ++reg) if (live_until[reg] < position) {
                live_until[reg] = v.last; return v.location = Operand::r(reg);
            }
        }
        return v.location = home(p.values[id-1].name,t,true);
    }
    require(scalar_integer(t), "native result class not implemented");
    if (v.crosses_block && !v.single_edge) return v.location = home(p.values[id-1].name,t,true);
    const auto& definition = p.instructions[v.definition-1];
    // A sole adjacent scalar compare/return consumes the ABI result before
    // any fixed-register setup can invalidate it. Other intervals use normal
    // placement, including calls, wide conversions and exceptional edges.
    if (definition.opcode == Opcode::Call && v.uses == 1 && v.last == position+1) {
        const auto& consumer = p.instructions[v.last-1];
        if ((consumer.opcode == Opcode::Compare && scalar_integer(consumer.type) && consumer.type == t) ||
            (consumer.opcode == Opcode::Return && consumer.type == t))
            return v.location = Operand::r(XR_RAX);
    }
    if (definition.opcode == Opcode::Binary || definition.opcode == Opcode::Unary) {
        auto input = arg(definition,0);
        if (input.kind == lowir_model::Operand::Temporary) {
            auto& previous = state(root(input.ref));
            auto loc = previous.location;
            bool survives = loc.reg == XR_RBX || loc.reg >= XR_R12;
            if (loc.kind == Operand::Reg && previous.last == position && live_until[loc.reg] == position &&
                (!v.crosses_call || survives) && loc.reg != XR_RAX && v.last < first_clobber[loc.reg]) {
                live_until[loc.reg] = v.last;
                return v.location = loc;
            }
        }
    }
    bool frame_input = definition.opcode == Opcode::Binary &&
        arg(definition,0).kind == lowir_model::Operand::Temporary &&
        state(root(arg(definition,0).ref)).location.kind == Operand::Memory;
    if (!frame_input && v.uses == 1 && (v.last == position+1 || (v.converted_boolean && v.last == position+2)) && p.instructions[v.last-1].opcode == Opcode::Return)
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
    // The O0 void output boundary retains parameter capacity (including the
    // lowered object-return ABI). Scalar-return and nonreturning functions
    // need only the frame actually consumed by their selected instructions.
    bool returns_void = false;
    if (f.result == Type()) for (const auto& i : f.instructions)
        returns_void |= i.op == Op::Return;
    f.stack_size = (std::max(returns_void ? parameter_bytes : 0,f.frame_bytes + saved*8 + f.scratch_bytes) + 15 +
        (f.frame_alignment > 16 ? f.frame_alignment-1 : 0)) & ~std::uint64_t(15);
    require(f.stack_size < 0x70000000,"native aligned frame too large");
    stats.frame_bytes += f.stack_size;
}
} // namespace native
