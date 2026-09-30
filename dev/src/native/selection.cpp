#include "native/selection.h"
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
Operand Selector::value(lowir_model::Operand o, Type t)
{
    switch (o.kind) {
    case lowir_model::Operand::Integer: return Operand::imm(normalize(o.data.integer,t));
    case lowir_model::Operand::Null: return Operand::imm(0);
    case lowir_model::Operand::Temporary:
        require(state(o.ref).location.kind != Operand::None, "missing native value location");
        return state(o.ref).location;
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
    if (to.kind == Operand::Memory || (to.kind == Operand::Symbol && !to.address)) {
        if (from.kind != Operand::Reg && from.kind != Operand::Immediate)
            from = in_register(from,t,XR_R10);
        emit(Op::Store,t,{to,from});
    } else if (from.address) {
        from.address = false;
        emit(Op::Lea,Type::Ptr,{to,from});
    } else if (from.kind == Operand::Memory || from.kind == Operand::Symbol) {
        emit(Op::Load,t,{to,from});
    } else emit(Op::Mov,t,{to,from});
}
Operand Selector::memory(lowir_model::Operand o, int scratch)
{
    if (o.kind == lowir_model::Operand::Slot) return value(o,Type::Ptr);
    if (o.kind == lowir_model::Operand::Symbol) return Operand::symbol(SymbolId(o.ref),false);
    Operand base = value(o,Type::Ptr);
    if (base.address) { base.address = false; return base; }
    base = in_register(base,Type::Ptr,scratch);
    return Operand::mem(base.reg);
}
void Selector::normalize_register(Operand o, Type t)
{
    if (t.width() >= 64) return;
    emit(unsigned_type(t) ? Op::ExtendUnsigned : Op::ExtendSigned,t,{o,o});
}
void Selector::select(const lowir_model::Instruction& i)
{
    debug = i.debug;
    switch (i.opcode) {
    case Opcode::Const:
        require(scalar_integer(i.type), "native floating/wide constant not implemented"); break;
    case Opcode::Phi: break;
    case Opcode::Addr: {
        Operand address = memory(arg(i,0)); address.address = true;
        state(i.destination.index).location = address;
        break;
    }
    case Opcode::Copy: {
        auto src = value(arg(i,0),i.type);
        if (src.kind == Operand::Immediate || src.address) state(i.destination.index).location = src;
        else move(allocate(i.destination.index,i.type),src,value_type(arg(i,0),i.type));
        break;
    }
    case Opcode::Load: {
        require(scalar_integer(i.type), "native load class not implemented");
        auto m = memory(arg(i,0));
        auto dst = allocate(i.destination.index,i.type);
        auto reg = dst.kind == Operand::Reg ? dst : Operand::r(XR_R10);
        emit(Op::Load,i.type,{reg,m}); move(dst,reg,i.type); break;
    }
    case Opcode::Store: {
        require(scalar_integer(i.type), "native store class not implemented");
        auto m = memory(arg(i,1));
        auto src = value(arg(i,0),i.type);
        Type st = value_type(arg(i,0),i.type);
        if (src.kind == Operand::Memory && st != i.type) src = in_register(src,st,XR_R10);
        move(m,src,i.type); break;
    }
    case Opcode::Index: index(i); break;
    case Opcode::Unary: case Opcode::Binary: arithmetic(i); break;
    case Opcode::Compare: compare(i,state(i.destination.index).compare_branch); break;
    case Opcode::Convert: conversion(i); break;
    case Opcode::Call: call(i); break;
    case Opcode::CopyObject: case Opcode::ZeroInit: bulk(i); break;
    case Opcode::AtomicLoad: case Opcode::AtomicStore: case Opcode::AtomicAddFetch:
    case Opcode::AtomicExchange: case Opcode::AtomicCompareExchange:
    case Opcode::AtomicThreadFence: case Opcode::AtomicSignalFence: atomic(i); break;
    case Opcode::Jump: case Opcode::Branch: case Opcode::Switch:
    case Opcode::Return: case Opcode::Unreachable: control(i); break;
    default: throw ParseError(std::string("native operation not implemented: ")+spelling(i.opcode));
    }
}
Function Selector::run()
{
    f.symbol = source.symbol; f.debug = source.debug;
    f.result = p.signatures[source.signature.index-1].result;
    require(f.result == Type() || scalar_integer(f.result), "native return ABI class not implemented");
    block_id = p.block_order[source.blocks.begin].index;
    for (unsigned k = p.signatures[source.signature.index-1].parameters.begin;
         k != p.signatures[source.signature.index-1].parameters.end(); ++k)
        state(p.parameters[k].value.index).block = block_id;
    next_label = p.blocks.size()+1;
    analyze();
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        block_id = p.block_order[b].index;
        const auto& block = p.blocks[block_id-1];
        begin_block(block_id,block.name);
        if (b == source.blocks.begin) parameters();
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n) {
            position = n+1;
            select(p.instructions[n]);
        }
    }
    debug = DebugLocation();
    for (const auto& edge : edge_blocks) {
        begin_block(edge.label,0);
        edge_transfers(edge.pred,edge.target);
        emit(Op::Jump,Type(),{Operand::label(edge.target)});
    }
    finish_frame();
    ++stats.functions; stats.instructions += f.instructions.size();
    return std::move(f);
}
} // namespace native
