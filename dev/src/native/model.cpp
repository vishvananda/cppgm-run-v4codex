#include "native/model.h"
#include <sstream>
#include <cstring>
#include <cmath>
namespace native {
Operand Operand::r(int n) { Operand o; o.kind = Reg; o.reg = n; return o; }
Operand Operand::imm(std::uint64_t n) { Operand o; o.kind = Immediate; o.bits = n; return o; }
Operand Operand::mem(int b, std::int64_t d) { Operand o; o.kind = Memory; o.reg = b; o.displacement = d; return o; }
Operand Operand::symbol(SymbolId id, bool address) {
    Operand o; o.kind = Symbol; o.id = id.index; o.address = address; return o;
}
Operand Operand::label(std::uint32_t id) { Operand o; o.kind = Label; o.id = id; return o; }
static long double integer_floating(const lowir_model::Operand& value, unsigned precision)
{
    std::uint64_t lo = value.data.integer, hi = value.integer_high();
    if (value.negative_integer) { hi = ~hi+(lo == 0); lo = 0-lo; }
    unsigned bits = 0; for (auto n = hi; n; n >>= 1) ++bits;
    if (!bits) return value.negative_integer ? -static_cast<long double>(lo) : static_cast<long double>(lo);
    unsigned shift = bits+64-precision;
    std::uint64_t top;
    bool guard, sticky;
    if (shift < 64) {
        top = (hi<<(64-shift)) | (lo>>shift);
        guard = (lo>>(shift-1))&1; sticky = (lo&((std::uint64_t(1)<<(shift-1))-1)) != 0;
    } else if (shift == 64) { top = hi; guard = lo>>63; sticky = (lo<<1) != 0; }
    else {
        unsigned tail = shift-64; top = hi>>tail;
        guard = (hi>>(tail-1))&1; sticky = lo || (hi&((std::uint64_t(1)<<(tail-1))-1));
    }
    if (guard && (sticky || (top&1))) {
        ++top;
        if (!top) { top = std::uint64_t(1)<<63; ++shift; }
    }
    long double result = std::ldexp(static_cast<long double>(top),shift);
    return value.negative_integer ? -result : result;
}
Operand Operand::floating(lowir_model::Operand value, Type type, const lowir_model::Program* p)
{
    Operand o; o.kind = Floating; o.id = type.kind();
    long double n = value.kind == lowir_model::Operand::Floating ? value.data.floating :
        integer_floating(value,type == Type::F32 ? 24 : type == Type::F64 ? 53 : 64);
    unsigned char bytes[16] = {};
    if (type == Type::F32) { float f = n; std::memcpy(bytes,&f,4); }
    else if (type == Type::F64) { double f = n; std::memcpy(bytes,&f,8); }
    else std::memcpy(bytes,&n,10);
    if (value.signaling_nan && std::isnan(n)) {
        bytes[type == Type::F32 ? 2 : type == Type::F64 ? 6 : 7] &= ~(type == Type::F64 ? 8 : 64);
        bool payload = false;
        unsigned last = type == Type::F32 ? 2 : type == Type::F64 ? 6 : 7;
        for (unsigned j = 0; j < last; ++j) payload |= bytes[j] != 0;
        payload |= (bytes[last] & (type == Type::F32 ? 0x3f : type == Type::F64 ? 7 : 0x3f)) != 0;
        if (!payload) bytes[0] |= 1;
    }
    std::memcpy(&o.bits,bytes,8); std::memcpy(&o.displacement,bytes+8,2);
    if (p && value.kind == lowir_model::Operand::Floating && value.ref) {
        const auto& rounded = p->floating_literals[value.ref-1];
        if (type == Type::F32) o.bits = rounded.f32;
        if (type == Type::F64) o.bits = rounded.f64;
    }
    return o;
}
long double Operand::floating_value() const
{
    if (id == Type::F32) { float f; std::memcpy(&f,&bits,4); return f; }
    if (id == Type::F64) { double f; std::memcpy(&f,&bits,8); return f; }
    unsigned char bytes[16] = {}; std::memcpy(bytes,&bits,8); std::memcpy(bytes+8,&displacement,2);
    long double f; std::memcpy(&f,bytes,16); return f;
}
const char* register_name(int reg) {
    static const char* const names[] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    static const char* const vector_names[] = {"xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"};
    if (reg >= 16 && reg < 32) return vector_names[reg-16];
    lowir_model::require(reg >= 0 && reg < 16, "invalid native register");
    return names[reg];
}
std::string type_name(Type t) {
    if (t == Type::I128 && t.abi_alignment() == 8) return "i128a8";
    static const char* const names[] = {"void","i1","i8","u8","i16","u16","i32","u32","i64","i128","f32","f64","f80","ptr"};
    if (t.complex()) return t.component() == Type::F32 ? "c32" : t.component() == Type::F64 ? "c64" : "c80";
    if (t.kind() != Type::Object) return names[t.kind()];
    std::ostringstream s; s << "obj<" << t.bytes() << 'x' << t.alignment() << '>'; return s.str();
}
bool unsigned_type(Type t) { return t == Type::I1 || t == Type::U8 || t == Type::U16 || t == Type::U32 || t == Type::Ptr; }
bool scalar_integer(Type t) { return t == Type::Ptr || (t.integer() && t != Type::I128); }
std::uint64_t normalize(std::uint64_t n, Type t) {
    unsigned w = t.width();
    if (w >= 64) return n;
    std::uint64_t mask = (std::uint64_t(1) << w) - 1;
    n &= mask;
    if (!unsigned_type(t) && (n & (std::uint64_t(1) << (w-1)))) n |= ~mask;
    return n;
}
} // namespace native
