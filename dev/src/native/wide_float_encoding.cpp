#include "native/encoding.h"
#include <cmath>
namespace native {
// Width-128 conversions are bounded target operations. Keep all integer bits
// until rounding: a 64-bit round-to-odd intermediate prevents double rounding
// when the final target has fewer than 64 significant bits.
void Encoder::wide_to_float(const Instruction& i)
{
    auto from = i.args[1], high = from; high.displacement += 8;
    auto lo = Operand::r(XR_RAX), hi = Operand::r(XR_R10), lost = Operand::r(XR_R11), count = Operand::r(XR_RCX);
    load(lo,from,Type::I64,false); load(hi,high,Type::I64,false);
    mov(Operand::r(XR_RDX),Operand::imm(0));
    if (i.op == Op::Sitofp) {
        mov(Operand::r(XR_RDX),hi); form(0xc1,64,7,Operand::r(XR_RDX),1,63);
        form(0x85,64,hi.reg,hi); auto positive = local_jump(XC_NS);
        form(0xf7,64,3,lo); form(0x83,64,2,hi,1,0); form(0xf7,64,3,hi);
        local_target(positive);
    }
    mov(count,Operand::imm(0));
    form(0x85,64,hi.reg,hi); auto fits = local_jump(XC_E);
    form(0x0fbd,64,count.reg,hi); form(0x83,64,0,count,1,1); // bsr + 1
    mov(lost,lo); form(0xf7,64,3,count); form(0xd3,64,4,lost); form(0xf7,64,3,count);
    form(0x83,64,7,count,1,64); auto full = local_jump(XC_E);
    form(0x0fad,64,hi.reg,lo); auto shifted = local_jump(-1);
    local_target(full); mov(lo,hi); local_target(shifted);
    if (i.type == Type::F80) {
        mov(hi,Operand::imm(std::uint64_t(1)<<63)); form(0x39,64,hi.reg,lost);
        auto below = local_jump(XC_B), above = local_jump(XC_A);
        form(0xf7,64,0,lo,4,1); auto even = local_jump(XC_E);
        local_target(above); form(0x83,64,0,lo,1,1); auto no_overflow = local_jump(XC_NE);
        mov(lo,hi); form(0x83,64,0,count,1,1);
        local_target(no_overflow); local_target(below); local_target(even);
    } else {
        form(0x85,64,lost.reg,lost); auto exact = local_jump(XC_E);
        form(0x83,64,1,lo,1,1); local_target(exact);
    }
    local_target(fits);
    store(scratch(32),count,Type::I64);
    store(scratch(),lo,Type::I64); form(0xdf,32,5,scratch());
    form(0x85,64,lo.reg,lo); auto signed_fit = local_jump(XC_NS);
    x87_load(Operand::floating(lowir_model::Operand::floating(18446744073709551616.0L),Type::F80),Type::F80,16);
    byte(0xde); byte(0xc1); local_target(signed_fit);
    form(0xdf,32,5,scratch(32)); byte(0xd9); byte(0xc9); // value, exponent
    byte(0xd9); byte(0xfd); byte(0xdd); byte(0xd9); // fscale; discard exponent
    form(0x85,64,XR_RDX,Operand::r(XR_RDX)); auto positive = local_jump(XC_E);
    byte(0xd9); byte(0xe0); local_target(positive);
    x87_store(i.args[0],i.type);
}
void Encoder::float_to_wide(const Instruction& i)
{
    auto to = i.args[0];
    lowir_model::require(to.kind == Operand::Memory,"wide floating conversion requires a result home");
    x87_load(i.args[1],i.source_type);
    // Keep sign independently, then split the nonnegative magnitude into four
    // exact base-2^32 digits. Each fild/fistp fits a signed 64-bit carrier.
    byte(0xd9); byte(0xe4); byte(0xdf); byte(0xe0); // ftst; fnstsw ax
    mov(Operand::r(XR_RDX),Operand::r(XR_RAX));
    form(0x81,32,4,Operand::r(XR_RDX),4,0x100);
    byte(0xd9); byte(0xe1); // fabs
    form(0xd9,32,7,scratch(40)); load(Operand::r(XR_R10),scratch(40),Type::U16,false);
    form(0x81,32,1,Operand::r(XR_R10),4,0xc00); store(scratch(42),Operand::r(XR_R10),Type::U16);
    form(0xd9,32,5,scratch(42));
    for (int part = 3; part >= 0; --part) {
        byte(0xd9); byte(0xc0); // duplicate remainder
        if (part) {
            x87_load(Operand::floating(lowir_model::Operand::floating(std::ldexp(1.0L,-32*part)),Type::F80),Type::F80,16);
            byte(0xde); byte(0xc9);
        }
        form(0xdf,32,7,scratch());
        load(Operand::r(XR_RAX),scratch(),Type::I64,false);
        auto dest = to; dest.displacement += part*4; store(dest,Operand::r(XR_RAX),Type::U32);
        if (part) {
            form(0xdf,32,5,scratch());
            x87_load(Operand::floating(lowir_model::Operand::floating(std::ldexp(1.0L,32*part)),Type::F80),Type::F80,16);
            byte(0xde); byte(0xc9); byte(0xde); byte(0xe9); // subtract digit's contribution
        }
    }
    byte(0xdd); byte(0xd8); form(0xd9,32,5,scratch(40));
    if (i.op == Op::Fptosi) {
        form(0x85,64,XR_RDX,Operand::r(XR_RDX)); auto positive = local_jump(XC_E);
        auto high = to; high.displacement += 8;
        load(Operand::r(XR_RAX),to,Type::I64,false); load(Operand::r(XR_R10),high,Type::I64,false);
        form(0xf7,64,3,Operand::r(XR_RAX)); form(0x83,64,2,Operand::r(XR_R10),1,0); form(0xf7,64,3,Operand::r(XR_R10));
        store(to,Operand::r(XR_RAX),Type::I64); store(high,Operand::r(XR_R10),Type::I64);
        local_target(positive);
    }
}
}
