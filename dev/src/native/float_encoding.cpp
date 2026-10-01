#include "native/encoding.h"
namespace native {
using lowir_model::require;
Operand Encoder::scratch(unsigned offset) const
{
    require(function && function->scratch_bytes >= 48,"missing floating scratch frame");
    return Operand::mem(function->frame_base,-std::int64_t(function->frame_bytes +
        (function->frame_base == XR_RBP ? 8*__builtin_popcount(function->preserved) : 0)+function->scratch_bytes)+offset);
}
void Encoder::sse(unsigned opcode, Type type, int reg, Operand rm)
{
    if (rm.kind == Operand::Reg) rm.reg -= 16;
    form(opcode,32,reg-16,rm,0,0,type == Type::F32 ? 0xf3 : 0xf2);
}
void Encoder::fmove(Operand to, Operand from, Type t)
{
    if (to.kind == Operand::Reg && from.kind == Operand::Reg && to.reg == from.reg) return;
    if (t.vector()) {
        require(t.bytes() == 8 || t.bytes() == 16,"unsupported vector register width");
        auto reg = to.kind == Operand::Reg ? to.reg : from.reg;
        auto memory = to.kind == Operand::Reg ? from : to;
        if (memory.kind == Operand::Reg) memory.reg -= 16;
        form(to.kind == Operand::Reg ? 0x0f10 : 0x0f11,32,reg-16,memory,0,0,t.bytes() == 8 ? 0xf2 : 0);
        return;
    }
    if (t == Type::F80) { x87_load(from,t); x87_store(to,t); return; }
    if (from.kind == Operand::Floating) {
        mov(Operand::r(XR_RAX),Operand::imm(from.bits));
        if (to.kind == Operand::Reg) form(0x0f6e,t == Type::F64 ? 64 : 32,to.reg-16,Operand::r(XR_RAX),0,0,0x66);
        else store(to,Operand::r(XR_RAX),t == Type::F32 ? Type::U32 : Type::I64);
    } else if (to.kind == Operand::Reg) sse(0x0f10,t,to.reg,from);
    else {
        if (from.kind != Operand::Reg) { fmove(Operand::r(xmm(15)),from,t); from = Operand::r(xmm(15)); }
        sse(0x0f11,t,from.reg,to);
    }
}
void Encoder::x87_load(Operand from, Type t, unsigned offset)
{
    if (from.kind == Operand::Floating) {
        auto storage = scratch(offset);
        mov(Operand::r(XR_RAX),Operand::imm(from.bits)); store(storage,Operand::r(XR_RAX),Type::I64);
        if (t == Type::F80) { storage.displacement += 8; store(storage,Operand::imm(from.displacement),Type::U16); }
        from = scratch(offset);
    } else if (from.kind == Operand::Reg) {
        require(t != Type::F80,"f80 value cannot occupy an XMM register");
        fmove(scratch(offset),from,t); from = scratch(offset);
    }
    form(t == Type::F32 ? 0xd9 : t == Type::F64 ? 0xdd : 0xdb,32,t == Type::F80 ? 5 : 0,from);
}
void Encoder::x87_store(Operand to, Type t)
{
    bool reg = to.kind == Operand::Reg;
    form(t == Type::F32 ? 0xd9 : t == Type::F64 ? 0xdd : 0xdb,32,t == Type::F80 ? 7 : 3,reg ? scratch(32) : to);
    if (reg) fmove(to,scratch(32),t);
}
void Encoder::float_compare(Operand left, Operand right, Type t)
{
    if (t == Type::F80) {
        x87_load(right,t,16); x87_load(left,t);
        byte(0xdf); byte(0xe9); // fucomip st0, st1
        byte(0xdd); byte(0xd8); // pop remaining rhs, retaining flags
    } else {
        // Always reserve distinct scratch for two logical operands. In particular
        // literals and spilled homes must never overwrite an allocated XMM value.
        fmove(Operand::r(xmm(14)),left,t);
        if (right.kind == Operand::Floating) { fmove(Operand::r(xmm(15)),right,t); right = Operand::r(xmm(15)); }
        if (right.kind == Operand::Reg) right.reg -= 16;
        form(0x0f2e,32,14,right,0,0,t == Type::F64 ? 0x66 : 0);
    }
}
void Encoder::floating(const Instruction& i)
{
    auto dst = i.args[0], lhs = i.args[1], rhs = i.args[2];
    switch (i.op) {
    case Op::Fmov: fmove(dst,lhs,i.type); return;
    case Op::Fcompare: float_compare(dst,lhs,i.type); return;
    case Op::Fset: {
        float_compare(lhs,rhs,i.type);
        form(0x0f90+i.condition,8,0,Operand::r(XR_R10));
        form(0x0f90+(i.condition == XC_NE ? XC_P : XC_NP),8,0,Operand::r(XR_R11));
        form(i.condition == XC_NE ? 0x08 : 0x20,8,XR_R11,Operand::r(XR_R10));
        load(Operand::r(XR_R10),Operand::r(XR_R10),Type::U8,false);
        if (dst.kind == Operand::Reg) mov(dst,Operand::r(XR_R10)); else store(dst,Operand::r(XR_R10),Type::I64);
        return;
    }
    case Op::Fpush: x87_load(dst,i.type); return;
    case Op::Freturn:
        x87_load(dst,i.type);
        if (function->shared_epilogue) branch(epilogue); else epilogue_code(); return;
    case Op::Fpop: x87_store(dst,i.type); return;
    case Op::Sitofp: case Op::Uitofp: case Op::Fptosi: case Op::Fptoui:
    case Op::Fpext: case Op::Fptrunc: float_convert(i); return;
    default: break;
    }
    if (i.type == Type::F80 || i.source_type == Type::F80) {
        if (i.op == Op::Fneg) { x87_load(lhs,i.type); byte(0xd9); byte(0xe0); }
        else {
            x87_load(lhs,i.type); x87_load(rhs,i.type,16);
            byte(0xde); byte(i.op == Op::Fadd ? 0xc1 : i.op == Op::Fsub ? 0xe9 : i.op == Op::Fmul ? 0xc9 : 0xf9);
        }
        x87_store(dst,i.type); return;
    }
    // A dead lhs can share the result carrier. If the result instead aliases
    // rhs, retain the reserved temporary so the two inputs remain distinct.
    int result = dst.kind == Operand::Reg && !(rhs.kind == Operand::Reg && rhs.reg == dst.reg &&
        !(lhs.kind == Operand::Reg && lhs.reg == dst.reg)) ? dst.reg : xmm(14);
    fmove(Operand::r(result),lhs,i.type);
    if (i.op == Op::Fneg) {
        // XOR the sign bit, including for zero and NaNs. Arithmetic subtraction
        // from positive zero is not a representation-preserving negation.
        mov(Operand::r(XR_RAX),Operand::imm(std::uint64_t(1) << (i.type.width()-1)));
        form(0x0f6e,i.type == Type::F64 ? 64 : 32,15,Operand::r(XR_RAX),0,0,0x66);
        form(0x0f57,32,result-16,Operand::r(15));
    } else {
        if (rhs.kind == Operand::Floating) { fmove(Operand::r(xmm(15)),rhs,i.type); rhs = Operand::r(xmm(15)); }
        unsigned opcode = i.op == Op::Fadd ? 0x0f58 : i.op == Op::Fsub ? 0x0f5c : i.op == Op::Fmul ? 0x0f59 : 0x0f5e;
        sse(opcode,i.type,result,rhs);
    }
    fmove(dst,Operand::r(result),i.type);
}
} // namespace native
