#include "native/model.h"
#include <sstream>
namespace native {
Operand Operand::r(int n) { Operand o; o.kind = Reg; o.reg = n; return o; }
Operand Operand::imm(std::uint64_t n) { Operand o; o.kind = Immediate; o.bits = n; return o; }
Operand Operand::mem(int b, std::int64_t d) { Operand o; o.kind = Memory; o.reg = b; o.displacement = d; return o; }
Operand Operand::symbol(SymbolId id, bool address) {
    Operand o; o.kind = Symbol; o.id = id.index; o.address = address; return o;
}
Operand Operand::label(std::uint32_t id) { Operand o; o.kind = Label; o.id = id; return o; }
const char* register_name(int reg) {
    static const char* const names[] = {"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
    lowir_model::require(reg >= 0 && reg < 16, "invalid native register");
    return names[reg];
}
std::string type_name(Type t) {
    static const char* const names[] = {"void","i1","i8","u8","i16","u16","i32","u32","i64","i128","f32","f64","f80","ptr"};
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
