#pragma once
#include "native/model.h"
#include <algorithm>
namespace native {
// Ordinary PA24 objects use integer eightbytes; complex primitives retain
// their floating classification. Failed multiword allocation consumes no registers.
inline bool aggregate(Type t) { return t == Type::I128 || t.kind() == Type::Object; }
inline bool indirect_return(Type t) { return t.kind() == Type::Object && !t.complex() && t.bytes() > 16; }
inline Type abi_chunk_type(Type t, unsigned part) {
    if (t.vector() && t.bytes() >= 8 && t.bytes() <= 16) return t;
    if (t.complex()) return Type::F64; // packed pair of floats, or one double
    unsigned bytes = std::min(8u,t.bytes()-part*8);
    return bytes > 4 ? Type::I64 : bytes > 2 ? Type::U32 : bytes > 1 ? Type::U16 : Type::U8;
}
inline Type chunk_type(unsigned bytes) {
    return bytes > 4 ? Type::I64 : bytes > 2 ? Type::U32 : bytes > 1 ? Type::U16 : Type::U8;
}
struct AbiLocation {
    std::array<Operand,2> parts;
    unsigned count = 1;
    bool memory = false;
};
struct AbiCursor {
    unsigned gp = 0, fp = 0;
    std::uint64_t stack = 0;
    unsigned stack_origin = 0, stack_alignment = 16;
    AbiLocation take(Type t, int stack_base) {
        static const int regs[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
        AbiLocation r;
        unsigned chunks = aggregate(t) ? (t.bytes()+7)/8 : 1;
        if (t.vector() && t.bytes() >= 8 && t.bytes() <= 16 && fp < 8) r.parts[0] = Operand::r(xmm(fp++));
        else if (t.complex() && t.component() != Type::F80 && fp+chunks <= 8) {
            r.count = chunks;
            for (unsigned k = 0; k < chunks; ++k) r.parts[k] = Operand::r(xmm(fp++));
        }
        else if ((t == Type::F32 || t == Type::F64) && fp < 8) r.parts[0] = Operand::r(xmm(fp++));
        else if (!(t.vector() && t.bytes() >= 8) && !t.complex() && !t.floating() && chunks <= 2 && gp+chunks <= 6) {
            r.count = chunks;
            for (unsigned k = 0; k < chunks; ++k) r.parts[k] = Operand::r(regs[gp++]);
        } else {
            unsigned alignment = std::max(8u,t.abi_alignment());
            stack_alignment = std::max(stack_alignment,alignment);
            stack = ((stack-stack_origin+alignment-1)&~std::uint64_t(alignment-1))+stack_origin;
            r.parts[0] = Operand::mem(stack_base,stack); r.memory = true;
            stack += (t.bytes()+7)&~std::uint64_t(7);
        }
        return r;
    }
};
}
