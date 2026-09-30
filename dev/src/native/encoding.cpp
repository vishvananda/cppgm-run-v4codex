#include "native/encoding.h"
#include <climits>
namespace native {
using lowir_model::require;
void Encoder::byte(unsigned n) { code.push_back(n); }
void Encoder::number(std::uint64_t n, unsigned width) { for (unsigned k = 0; k < width; ++k) { byte(n); n >>= 8; } }
void Encoder::modrm(unsigned reg, const Operand& o)
{
    if (o.kind == Operand::Reg) { byte(0xc0 | (reg&7)*8 | (o.reg&7)); return; }
    if (o.kind == Operand::Symbol) {
        byte((reg&7)*8 | 5);
        Fixup fix; fix.offset = code.size(); fix.symbol = o.id; fix.addend = o.displacement;
        image.code_fixups.push_back(fix); number(0,4); return;
    }
    require(o.kind == Operand::Memory && o.reg >= 0, "invalid encoded memory operand");
    require(o.displacement >= INT32_MIN && o.displacement <= INT32_MAX, "memory displacement too large");
    bool sib = (o.reg&7) == 4 || o.index >= 0;
    unsigned mode = o.displacement == 0 && (o.reg&7) != 5 ? 0 :
        o.displacement >= -128 && o.displacement <= 127 ? 1 : 2;
    byte(mode*64 | (reg&7)*8 | (sib ? 4 : o.reg&7));
    if (sib) {
        unsigned scale = o.scale == 8 ? 3 : o.scale == 4 ? 2 : o.scale == 2 ? 1 : 0;
        byte(scale*64 | (o.index < 0 ? 4 : o.index&7)*8 | (o.reg&7));
    }
    if (mode) number(o.displacement,mode == 1 ? 1 : 4);
}
void Encoder::form(unsigned opcode, unsigned width, unsigned reg, Operand rm, unsigned immediate_bytes, std::uint64_t immediate, unsigned prefix)
{
    if (prefix) byte(prefix);
    if (width == 16) byte(0x66);
    unsigned rex = 0x40 | (width == 64 ? 8 : 0) | (reg >= 8 ? 4 : 0);
    if ((rm.kind == Operand::Reg || rm.kind == Operand::Memory) && rm.reg >= 8) rex |= 1;
    if (rm.kind == Operand::Memory && rm.index >= 8) rex |= 2;
    if (rex != 0x40 || (width == 8 && (reg >= 4 || (rm.kind == Operand::Reg && rm.reg >= 4)))) byte(rex);
    if (opcode > 255) byte(opcode >> 8);
    byte(opcode);
    std::size_t fixes = image.code_fixups.size();
    modrm(reg,rm);
    number(immediate,immediate_bytes);
    if (image.code_fixups.size() != fixes) image.code_fixups.back().end = code.size();
}
void Encoder::mov(Operand to, Operand from)
{
    require(to.kind == Operand::Reg, "mov destination must be a register");
    if (from.kind == Operand::Symbol) {
        if (image.host && image.indirect_functions[from.id]) {
            auto offset = from.displacement; from.displacement = 0;
            form(0x8b,64,to.reg,from);
            image.code_fixups.back().kind = Fixup::GotSymbol;
            if (offset) form(0x8d,64,to.reg,Operand::mem(to.reg,offset));
        } else form(0x8d,64,to.reg,from);
        return;
    }
    if (from.kind == Operand::Reg) { if (to.reg != from.reg) form(0x89,64,from.reg,to); return; }
    require(from.kind == Operand::Immediate, "mov requires register or immediate source");
    if (from.bits <= UINT32_MAX) {
        if (to.reg >= 8) byte(0x41);
        byte(0xb8+(to.reg&7)); number(from.bits,4);
    } else if (std::int64_t(from.bits) >= INT32_MIN && std::int64_t(from.bits) <= INT32_MAX)
        form(0xc7,64,0,to,4,from.bits);
    else {
        byte(0x48 | (to.reg >= 8)); byte(0xb8+(to.reg&7)); number(from.bits,8);
    }
}
void Encoder::load(Operand to, Operand from, Type t, bool sign)
{
    unsigned width = t.width();
    if (from.kind == Operand::Immediate) {
        std::uint64_t n = from.bits;
        if (width < 64) {
            auto mask = (std::uint64_t(1)<<width)-1; n &= mask;
            if (sign && (n & (std::uint64_t(1)<<(width-1)))) n |= ~mask;
        }
        mov(to,Operand::imm(n)); return;
    }
    if (width <= 16) form(sign && width != 1 ? (width == 16 ? 0x0fbf : 0x0fbe) : (width == 16 ? 0x0fb7 : 0x0fb6),64,to.reg,from);
    else if (width == 32 && sign) form(0x63,64,to.reg,from);
    else form(0x8b,width,to.reg,from);
    if (width == 1) form(0x83,32,4,to,1,1);
}
void Encoder::store(Operand to, Operand from, Type t)
{
    unsigned width = t.width() == 1 ? 8 : t.width();
    if (from.kind == Operand::Immediate) {
        if (width == 64 && (std::int64_t(from.bits) < INT32_MIN || std::int64_t(from.bits) > INT32_MAX)) {
            mov(Operand::r(XR_RAX),from); from = Operand::r(XR_RAX);
        } else {
            form(width == 8 ? 0xc6 : 0xc7,width,0,to,width == 64 ? 4 : width/8,from.bits); return;
        }
    }
    require(from.kind == Operand::Reg, "store source must be register or immediate");
    form(width == 8 ? 0x88 : 0x89,width,from.reg,to);
}
void Encoder::branch(unsigned label, int condition)
{
    if (condition < 0) byte(0xe9);
    else { byte(0x0f); byte(0x80+condition); }
    branches.push_back({code.size(),label}); number(0,4);
}
void Encoder::call(Operand target)
{
    if (target.kind == Operand::Symbol) {
        byte(0xe8);
        Fixup fix; if (image.host) fix.kind = Fixup::CallSymbol; fix.symbol = target.id; fix.offset = code.size(); fix.end = code.size()+4;
        image.code_fixups.push_back(fix); number(0,4);
    } else form(0xff,64,2,target);
}
} // namespace native
