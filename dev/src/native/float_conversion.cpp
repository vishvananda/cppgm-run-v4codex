#include "native/encoding.h"
namespace native {
std::size_t Encoder::local_jump(int condition)
{
    if (condition < 0) byte(0xe9); else { byte(0x0f); byte(0x80+condition); }
    auto offset = code.size(); number(0,4); return offset;
}
void Encoder::local_target(std::size_t offset)
{
    auto distance = code.size()-offset-4;
    for (unsigned k = 0; k < 4; ++k) code[offset+k] = distance >> (8*k);
}
void Encoder::float_convert(const Instruction& i)
{
    if (i.source_type == Type::I128) { wide_to_float(i); return; }
    if (i.type == Type::I128) { float_to_wide(i); return; }
    auto to = i.args[0], from = i.args[1];
    Type src = i.source_type, dst = i.type;
    if (src.floating() && dst.floating()) {
        if (src == Type::F80 || dst == Type::F80) { x87_load(from,src); x87_store(to,dst); }
        else {
            if (from.kind == Operand::Floating) { fmove(Operand::r(xmm(15)),from,src); from = Operand::r(xmm(15)); }
            sse(0x0f5a,src,xmm(14),from); fmove(to,Operand::r(xmm(14)),dst);
        }
        return;
    }
    if (!src.floating()) {
        bool uns = i.op == Op::Uitofp;
        load(Operand::r(XR_RAX),from,src,!uns);
        if (dst != Type::F80 && (!uns || src.width() < 64)) {
            form(0x0f2a,64,14,Operand::r(XR_RAX),0,0,dst == Type::F32 ? 0xf3 : 0xf2);
            fmove(to,Operand::r(xmm(14)),dst); return;
        }
        store(scratch(),Operand::r(XR_RAX),Type::I64);
        form(0xdf,32,5,scratch()); // fild signed qword
        if (uns && src.width() == 64) {
            form(0x85,64,XR_RAX,Operand::r(XR_RAX)); auto positive = local_jump(XC_NS);
            auto correction = Operand::floating(lowir_model::Operand::floating(18446744073709551616.0L),Type::F80);
            x87_load(correction,Type::F80,16); byte(0xde); byte(0xc1);
            local_target(positive);
        }
        x87_store(to,dst); return;
    }
    bool uns = i.op == Op::Fptoui;
    if (src != Type::F80 && (!uns || dst.width() < 64)) {
        if (from.kind == Operand::Floating) { fmove(Operand::r(xmm(15)),from,src); from = Operand::r(xmm(15)); }
        if (from.kind == Operand::Reg) from.reg -= 16;
        form(0x0f2c,64,XR_R10,from,0,0,src == Type::F32 ? 0xf3 : 0xf2);
    } else {
        x87_load(from,src);
        // Save/restore the caller's x87 control word; integer conversion truncates
        // independently of its rounding mode. Only reserved GPRs are consumed.
        form(0xd9,32,7,scratch(40));
        load(Operand::r(XR_R10),scratch(40),Type::U16,false);
        form(0x81,32,1,Operand::r(XR_R10),4,0xc00);
        store(scratch(42),Operand::r(XR_R10),Type::U16);
        form(0xd9,32,5,scratch(42));
        if (uns && dst.width() == 64) {
            auto threshold = Operand::floating(lowir_model::Operand::floating(9223372036854775808.0L),Type::F80);
            x87_load(threshold,Type::F80,16);
            byte(0xd9); byte(0xc9); // x, 2^63
            byte(0xdb); byte(0xf1); // fcomi x, 2^63 (no pop)
            auto small = local_jump(XC_B);
            byte(0xde); byte(0xe1); // fsubrp: x - 2^63
            form(0xdf,32,7,scratch(32));
            load(Operand::r(XR_R10),scratch(32),Type::I64,false);
            mov(Operand::r(XR_RAX),Operand::imm(std::uint64_t(1)<<63));
            form(0x31,64,XR_RAX,Operand::r(XR_R10));
            auto done = local_jump(-1); local_target(small);
            byte(0xdd); byte(0xd9); // fstp st1 drops threshold, retains x
            form(0xdf,32,7,scratch(32));
            load(Operand::r(XR_R10),scratch(32),Type::I64,false);
            local_target(done);
        } else {
            form(0xdf,32,7,scratch(32));
            load(Operand::r(XR_R10),scratch(32),Type::I64,false);
        }
        form(0xd9,32,5,scratch(40));
    }
    if (dst.width() < 64) load(Operand::r(XR_R10),Operand::r(XR_R10),dst,!uns);
    if (to.kind == Operand::Reg) mov(to,Operand::r(XR_R10)); else store(to,Operand::r(XR_R10),dst);
}
} // namespace native
