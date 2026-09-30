#include "native/selection.h"
#include "native/abi.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
Instruction& Selector::emit(Op op, Type t, std::initializer_list<Operand> args)
{
    native::Instruction i(op,t);
    i.count = args.size(); i.debug = debug;
    std::copy(args.begin(),args.end(),i.args.begin());
    f.instructions.push_back(i);
    ++f.blocks.back().instructions.count;
    return f.instructions.back();
}
void Selector::begin_block(unsigned id, Name name)
{
    Block block; block.id = id; block.name = name;
    block.instructions.begin = f.instructions.size();
    f.blocks.push_back(block);
}
Type Selector::value_type(lowir_model::Operand o, Type fallback) const
{
    if (o.kind == lowir_model::Operand::Temporary) return p.values[o.ref-1].type;
    if (o.kind == lowir_model::Operand::Slot) return p.slots[o.ref-1].type;
    return fallback;
}
Type Selector::consumed_type(lowir_model::Operand o, Type context) const
{
    Type actual = value_type(o,context);
    return actual == Type::I128 && context.integer() && context != Type::I128 ? context : actual;
}
Operand Selector::value(lowir_model::Operand o, Type t)
{
    if (o.literal() && t.floating()) return Operand::floating(o,t,&p);
    if ((o.kind == lowir_model::Operand::Temporary || o.kind == lowir_model::Operand::Slot) && t.integer()) {
        Type actual = value_type(o,t);
        if (t == Type::I128 && actual.integer() && actual != Type::I128) {
            auto original = value(o,actual), storage = home(0,t,true);
            move(Operand::r(XR_R10),original,actual);
            move(fragment(storage,0),Operand::r(XR_R10),Type::I64);
            if (unsigned_type(actual)) move(fragment(storage,8),Operand::imm(0),Type::I64);
            else { emit(Op::Sar,Type::I64,{Operand::r(XR_R10),Operand::imm(63)}); move(fragment(storage,8),Operand::r(XR_R10),Type::I64); }
            return storage;
        }
        if (actual == Type::I128 && t != Type::I128) return fragment(value(o,actual),0);
    }
    switch (o.kind) {
    case lowir_model::Operand::Integer: {
        if (t != Type::I128) return Operand::imm(normalize(o.data.integer,t));
        Operand result; result.kind = Operand::WideImmediate; result.bits = o.data.integer;
        result.displacement = o.integer_high(); return result;
    }
    case lowir_model::Operand::Null: return Operand::imm(0);
    case lowir_model::Operand::Temporary:
        if (!state(root(o.ref)).definition && scalar_integer(p.values[root(o.ref)-1].type)) {
            unsigned index = workspace.value_indices[root(o.ref)]-1;
            const auto& incoming = f.params[index].location;
            const auto& home = state(root(o.ref)).location;
            if (incoming.kind == Operand::Reg && incoming.reg < 16 && home.kind == Operand::Memory &&
                home.id && f.frame[home.id-1].parameter && !(parameter_clobbers & (1u<<incoming.reg))) return incoming;
        }
        require(state(root(o.ref)).location.kind != Operand::None, "missing native value location");
        return state(root(o.ref)).location;
    case lowir_model::Operand::Symbol: return Operand::symbol(SymbolId(o.ref));
    case lowir_model::Operand::Slot: {
        Operand& loc = workspace.slots[o.ref];
        if (loc.kind == Operand::None) loc = home(p.slots[o.ref-1].name,p.slots[o.ref-1].type,false);
        return loc;
    }
    default: throw ParseError("native operand class not implemented");
    }
}
Operand Selector::in_register(Operand o, Type t, int reg)
{
    if (o.kind == Operand::Reg) return o;
    Operand dest = Operand::r(reg);
    move(dest,o,t);
    return dest;
}
void Selector::move(Operand to, Operand from, Type t)
{
    if (to.kind == Operand::Reg && from.kind == Operand::Reg && to.reg == from.reg) return;
    if (t.kind() == Type::Object) { object_move(to,from,t); return; }
    if (t == Type::I128) {
        move(fragment(to,0),fragment(from,0),Type::I64);
        move(fragment(to,8),fragment(from,8),Type::I64); return;
    }
    if (t.floating()) {
        f.scratch_bytes = 48;
        emit(Op::Fmov,t,{to,from}); return;
    }
    if (to.kind == Operand::Memory || (to.kind == Operand::Symbol && !to.address)) {
        if (from.kind != Operand::Reg && from.kind != Operand::Immediate) {
            // A folded indexed destination may still consume the reserved r10
            // address carrier. Loading its value must not overwrite that index.
            int scratch = to.reg == XR_R10 || to.index == XR_R10 ? XR_RAX : XR_R10;
            from = in_register(from,t,scratch);
        }
        emit(Op::Store,t,{to,from});
    } else if (from.kind == Operand::Symbol && from.address && tls_symbol(from.id)) {
        tls_address(to,from);
    } else if (from.address) {
        from.address = false;
        emit(from.kind == Operand::Symbol ? Op::Mov : Op::Lea,Type::Ptr,{to,from});
    } else if (from.kind == Operand::Memory || from.kind == Operand::Symbol) {
        // Adjacent compiler-created stores carry their normalized scalar value.
        // No instruction or observable memory operation can intervene here.
        if (from.temporary && !f.instructions.empty()) {
            const auto& previous = f.instructions.back();
            if (previous.op == Op::Store && previous.type == t && previous.args[0].temporary &&
                previous.args[0].reg == from.reg && previous.args[0].displacement == from.displacement &&
                previous.args[1].kind == Operand::Reg) {
                auto carrier = previous.args[1]; ++stats.scratch_carried_reloads;
                if (carrier.reg != to.reg) emit(Op::Mov,t,{to,carrier});
                return;
            }
        }
        emit(Op::Load,t,{to,from});
    } else emit(Op::Mov,t,{to,from});
}
Operand Selector::memory(lowir_model::Operand o, int scratch)
{
    if (o.kind == lowir_model::Operand::Slot) return value(o,Type::Ptr);
    if (o.kind == lowir_model::Operand::Symbol) {
        if (!tls_symbol(o.ref)) return Operand::symbol(SymbolId(o.ref),false);
        tls_address(Operand::r(scratch),Operand::symbol(SymbolId(o.ref)));
        return Operand::mem(scratch);
    }
    Operand base = value(o,Type::Ptr);
    if (value_type(o,Type::Ptr).kind() == Type::Object) return base;
    if (base.kind == Operand::Symbol && tls_symbol(base.id)) {
        tls_address(Operand::r(scratch),base); return Operand::mem(scratch);
    }
    if (base.address) { base.address = false; return base; }
    base = in_register(base,Type::Ptr,scratch);
    return Operand::mem(base.reg);
}
void Selector::normalize_register(Operand o, Type t)
{
    if (!scalar_integer(t) || t.width() >= 64) return;
    emit(unsigned_type(t) ? Op::ExtendUnsigned : Op::ExtendSigned,t,{o,o});
}
void Selector::select(const lowir_model::Instruction& i)
{
    debug = i.debug;
    if (i.destination && state(i.destination.index).alias) {
        // An adjacent elided Boolean conversion still owns its source step.
        // Attribute the producing normalization to that step while preserving
        // the comparison/set locations. Do not relabel intervening work.
        if (i.opcode == Opcode::Convert && i.debug.file && position > 1 && !f.instructions.empty()) {
            const auto& previous = p.instructions[position-2];
            auto op = f.instructions.back().op;
            if (previous.destination && root(previous.destination.index) == root(i.destination.index) &&
                (previous.opcode == Opcode::Compare || previous.opcode == Opcode::Convert) &&
                (op == Op::ExtendUnsigned || op == Op::Mov || op == Op::Store))
                f.instructions.back().debug = i.debug;
        }
        return;
    }
    switch (i.opcode) {
    case Opcode::Const:
        if (i.type.floating() || i.type == Type::I128) state(i.destination.index).location = value(arg(i,0),i.type);
        else require(scalar_integer(i.type), "native wide constant not implemented");
        break;
    case Opcode::Phi: break;
    case Opcode::Addr: {
        if (arg(i,0).kind == lowir_model::Operand::Symbol && tls_symbol(arg(i,0).ref)) {
            if (state(i.destination.index).uses)
                move(allocate(i.destination.index,Type::Ptr),value(arg(i,0),Type::Ptr),Type::Ptr);
            break;
        }
        Operand address = memory(arg(i,0)); address.address = true;
        // A constant symbol address never needs a spill home. When a control-
        // flow interval has no retained register, materialize at its consumers.
        if (address.kind == Operand::Symbol && !state(i.destination.index).address_only && state(i.destination.index).uses &&
            (!state(i.destination.index).crosses_block || state(i.destination.index).single_edge)) {
            auto dest = allocate(i.destination.index,Type::Ptr);
            move(dest,address,Type::Ptr);
        } else state(i.destination.index).location = address;
        break;
    }
    case Opcode::Copy: {
        auto src = value(arg(i,0),i.type);
        if (aggregate(i.type)) { move(allocate(i.destination.index,i.type),src,i.type); break; }
        if (i.type.floating() || value_type(arg(i,0),i.type).floating()) {
            convert_to(allocate(i.destination.index,i.type),src,value_type(arg(i,0),i.type),i.type); break;
        }
        if (src.kind == Operand::Immediate) state(i.destination.index).location = Operand::imm(normalize(src.bits,i.type));
        else if (src.address) state(i.destination.index).location = src;
        else {
            auto dest = allocate(i.destination.index,i.type);
            auto result = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
            move(result,src,consumed_type(arg(i,0),i.type));
            normalize_register(result,i.type); move(dest,result,i.type);
        }
        break;
    }
    case Opcode::Load: {
        require(i.type.scalar() || i.type.kind() == Type::Object, "invalid native load class");
        auto m = memory(arg(i,0));
        if (state(i.destination.index).folded_load) { state(i.destination.index).location = m; break; }
        auto dst = allocate(i.destination.index,i.type);
        if (i.type.floating() || aggregate(i.type)) { move(dst,m,i.type); break; }
        auto reg = dst.kind == Operand::Reg ? dst : Operand::r(XR_R10);
        emit(Op::Load,i.type,{reg,m}); move(dst,reg,i.type); break;
    }
    case Opcode::Store: {
        if (arg(i,1).kind == lowir_model::Operand::Slot && !workspace.slot_facts[arg(i,1).ref].observed) break;
        if (arg(i,1).kind == lowir_model::Operand::Slot && workspace.slot_facts[arg(i,1).ref].stored && !workspace.slot_facts[arg(i,1).ref].escape) break;
        require(i.type.scalar() || i.type.kind() == Type::Object, "invalid native store class");
        auto m = memory(arg(i,1));
        Type st = consumed_type(arg(i,0),i.type);
        if (i.type == Type::I128 && st.integer() && st != i.type &&
            (m.reg == XR_R10 || m.index == XR_R10)) {
            // Widening an actual value uses r10 before its final stores. Freeze
            // the selected address first, including both base and scaled index.
            emit(Op::Lea,Type::Ptr,{Operand::r(XR_R11),m}); m = Operand::mem(XR_R11);
        }
        auto src = value(arg(i,0),i.type);
        if (i.type.floating() || st.floating()) { convert_to(m,src,st,i.type); break; }
        if (aggregate(i.type)) { move(m,src,i.type); break; }
        if (src.kind == Operand::Memory && st != i.type)
            src = in_register(src,st,m.reg == XR_R10 || m.index == XR_R10 ? XR_RAX : XR_R10);
        move(m,src,i.type); break;
    }
    case Opcode::Index: index(i); break;
    case Opcode::Unary: case Opcode::Binary: arithmetic(i); break;
    case Opcode::Compare: compare(i,state(i.destination.index).compare_branch); break;
    case Opcode::Convert: conversion(i); break;
    case Opcode::Call: call(i); break;
    case Opcode::VaStart: case Opcode::VaArg: variadic(i); break;
    case Opcode::CopyObject: case Opcode::ZeroInit: bulk(i); break;
    case Opcode::AtomicLoad: case Opcode::AtomicStore: case Opcode::AtomicAddFetch:
    case Opcode::AtomicExchange: case Opcode::AtomicCompareExchange:
    case Opcode::AtomicThreadFence: case Opcode::AtomicSignalFence: atomic(i); break;
    case Opcode::Jump: case Opcode::Branch: case Opcode::Switch:
    case Opcode::Return: case Opcode::Unreachable: control(i); break;
    case Opcode::EhTry: case Opcode::EhCleanup: case Opcode::EhEnd:
    case Opcode::Throw: case Opcode::Resume: case Opcode::Exception:
    case Opcode::StackAlloc: runtime(i); break;
    default: throw ParseError(std::string("native operation not implemented: ")+spelling(i.opcode));
    }
}
Function Selector::run()
{
    initialize_values();
    f.symbol = source.symbol; f.debug = source.debug;
    f.result = p.signatures[source.signature.index-1].result;
    for (unsigned k = source.slots.begin; k != source.slots.end(); ++k)
        f.frame_alignment = std::max(f.frame_alignment,p.slots[p.slot_order[k].index-1].type.alignment());
    // Overaligned addressable storage has a stable base separate from the
    // incoming ABI frame. Reserve it before assigning any parameter/value.
    if (f.frame_alignment > 16) {
        f.frame_base = XR_R15; f.preserved |= 1u<<XR_R15;
        live_until[XR_R15] = ~0u;
    }
    require(f.result == Type() || f.result.scalar() || f.result.kind() == Type::Object, "invalid native result class");
    block_id = p.block_order[source.blocks.begin].index;
    for (unsigned k = p.signatures[source.signature.index-1].parameters.begin;
         k != p.signatures[source.signature.index-1].parameters.end(); ++k)
        state(p.parameters[k].value.index).block = block_id;
    next_label = p.blocks.size()+1;
    analyze();
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& block = p.blocks[block_id-1];
        parameter_clobbers = workspace.parameter_clobbers[block_id];
        begin_block(block_id,block.name);
        if (b == source.blocks.begin) parameters();
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n) {
            position = n+1;
            select(p.instructions[n]);
            parameter_clobbers |= clobbers(p.instructions[n]);
        }
        workspace.parameter_exits[block_id] = parameter_clobbers;
    }
    debug = DebugLocation();
    for (const auto& edge : edge_blocks) {
        // A deferred transfer executes at this predecessor's exit, independent
        // of the source block that happened to be selected last.
        parameter_clobbers = workspace.parameter_exits[edge.pred];
        begin_block(edge.label,0);
        edge_transfers(edge.pred,edge.target);
        emit(Op::Jump,Type(),{Operand::label(edge.target)});
    }
    carry_reloads();
    finish_frame();
    ++stats.functions; stats.instructions += f.instructions.size();
    return std::move(f);
}
} // namespace native
