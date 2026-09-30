#include "native/encoding.h"
#include <climits>
namespace native {
void Encoder::multiply(const Instruction& i)
{
    auto dst = i.args[0], rhs = i.args[1];
    unsigned width = i.type.width();
    if (width < 16) width = 64;
    if (rhs.kind == Operand::Immediate && rhs.bits && rhs.bits <= INT64_MAX) {
        std::uint64_t odd = rhs.bits;
        unsigned shift = 0;
        while (!(odd&1)) { odd >>= 1; ++shift; }
        if (odd == 1 || odd == 3 || odd == 5 || odd == 9) {
            if (odd != 1) {
                Operand indexed = Operand::mem(dst.reg); indexed.index = dst.reg; indexed.scale = odd-1;
                form(0x8d,64,dst.reg,indexed);
            }
            if (shift) form(0xc1,width,4,dst,1,shift);
            return;
        }
    }
    if (rhs.kind == Operand::Immediate && std::int64_t(rhs.bits) >= INT32_MIN && std::int64_t(rhs.bits) <= INT32_MAX) {
        bool small = std::int64_t(rhs.bits) >= -128 && std::int64_t(rhs.bits) <= 127;
        form(small ? 0x6b : 0x69,width,dst.reg,dst,small ? 1 : width == 16 ? 2 : 4,rhs.bits);
    } else {
        if (rhs.kind == Operand::Immediate) { mov(Operand::r(XR_R11),rhs); rhs = Operand::r(XR_R11); }
        if (i.type.width() < 16 && rhs.kind != Operand::Reg) {
            load(Operand::r(XR_R11),rhs,i.type,!unsigned_type(i.type)); rhs = Operand::r(XR_R11);
        }
        form(0x0faf,width,dst.reg,rhs);
    }
}
void Encoder::arithmetic(const Instruction& i)
{
    auto dst = i.args[0], rhs = i.args[1];
    unsigned width = std::max(8u,i.type.width());
    if (i.op == Op::And && rhs.kind == Operand::Immediate && rhs.bits <= UINT32_MAX && width == 64) width = 32;
    if (i.op == Op::Mul) { multiply(i); return; }
    if (i.op == Op::Shl || i.op == Op::Shr || i.op == Op::Sar) {
        unsigned ext = i.op == Op::Shl ? 4 : i.op == Op::Shr ? 5 : 7;
        if (rhs.kind == Operand::Immediate) form(width == 8 ? 0xc0 : 0xc1,width,ext,dst,1,rhs.bits);
        else form(width == 8 ? 0xd2 : 0xd3,width,ext,dst);
        return;
    }
    unsigned ext = i.op == Op::Add ? 0 : i.op == Op::Or ? 1 : i.op == Op::And ? 4 :
        i.op == Op::Sub ? 5 : i.op == Op::Xor ? 6 : 7;
    if (i.op == Op::Compare && rhs.kind == Operand::Immediate && rhs.bits == 0) {
        form(width == 8 ? 0x84 : 0x85,width,dst.reg,dst); return;
    }
    if (rhs.kind == Operand::Immediate && width == 64 &&
        (std::int64_t(rhs.bits) < INT32_MIN || std::int64_t(rhs.bits) > INT32_MAX)) {
        mov(Operand::r(XR_R11),rhs); rhs = Operand::r(XR_R11);
    }
    if (rhs.kind == Operand::Immediate) {
        bool small = std::int64_t(rhs.bits) >= -128 && std::int64_t(rhs.bits) <= 127;
        form(width == 8 ? 0x80 : small ? 0x83 : 0x81,width,ext,dst,
            width == 8 || small ? 1 : width == 16 ? 2 : 4,rhs.bits);
    } else if (rhs.kind == Operand::Reg) form(ext*8 + (width == 8 ? 0 : 1),width,rhs.reg,dst);
    else form(ext*8 + (width == 8 ? 2 : 3),width,dst.reg,rhs);
}
void Encoder::instruction(const Instruction& i)
{
    auto a = i.args[0], b = i.args[1];
    unsigned width = std::max(8u,i.type.width());
    switch (i.op) {
    case Op::Mov: mov(a,b); break;
    case Op::Load: load(a,b,i.type,!unsigned_type(i.type)); break;
    case Op::Store: store(a,b,i.type); break;
    case Op::Lea: form(0x8d,64,a.reg,b); break;
    case Op::ExtendSigned: load(a,b,i.type,true); break;
    case Op::ExtendUnsigned: load(a,b,i.type,false); break;
    case Op::Add: case Op::Sub: case Op::Mul: case Op::And: case Op::Or: case Op::Xor:
    case Op::Compare: case Op::Shl: case Op::Shr: case Op::Sar: arithmetic(i); break;
    case Op::Neg: case Op::Not: form(width == 8 ? 0xf6 : 0xf7,width,i.op == Op::Neg ? 3 : 2,a); break;
    case Op::Bswap:
        if (width == 16) { form(0xc1,16,1,a,1,8); break; }
        byte((width == 64 ? 0x48 : 0x40) | (a.reg >= 8)); byte(0x0f); byte(0xc8+(a.reg&7)); break;
    case Op::Set: form(0x0f90+i.condition,8,0,a); break;
    case Op::SignDividend: byte(0x48); byte(0x99); break;
    case Op::Div: case Op::Udiv: form(0xf7,64,i.op == Op::Div ? 7 : 6,a); break;
    case Op::Jump: branch(a.id); break;
    case Op::Jcc: branch(a.id,i.condition); break;
    case Op::Call: call(a); break;
    case Op::Return:
        if (i.count) mov(Operand::r(XR_RAX),a);
        if (function->shared_epilogue) branch(epilogue); else epilogue_code();
        break;
    case Op::Exit:
        mov(Operand::r(XR_RAX),Operand::imm(60)); byte(0x0f); byte(0x05); break;
    case Op::Trap: byte(0x0f); byte(0x0b); break;
    case Op::Fence: byte(0x0f); byte(0xae); byte(0xf0); break;
    case Op::Xadd: case Op::Cmpxchg:
        form(i.op == Op::Xadd ? (width == 8 ? 0x0fc0 : 0x0fc1) : (width == 8 ? 0x0fb0 : 0x0fb1),width,b.reg,a,0,0,0xf0); break;
    case Op::Exchange: form(width == 8 ? 0x86 : 0x87,width,b.reg,a); break;
    case Op::CopyBytes: case Op::ZeroBytes: bulk(i); break;
    default: throw lowir_model::ParseError("unencoded native instruction");
    }
}
} // namespace native
