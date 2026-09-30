#pragma once
#include "native/model.h"
#include <algorithm>
namespace native {
// PA24's object boundary has integer eightbytes only. Classification is shared
// by both sides of every call. Failed multiword allocation consumes no GPRs.
inline bool aggregate(Type t) { return t == Type::I128 || t.kind() == Type::Object; }
inline bool indirect_return(Type t) { return t.kind() == Type::Object && t.bytes() > 16; }
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
    AbiLocation take(Type t, int stack_base) {
        static const int regs[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
        AbiLocation r;
        unsigned chunks = aggregate(t) ? (t.bytes()+7)/8 : 1;
        if ((t == Type::F32 || t == Type::F64) && fp < 8) r.parts[0] = Operand::r(xmm(fp++));
        else if (!t.floating() && chunks <= 2 && gp+chunks <= 6) {
            r.count = chunks;
            for (unsigned k = 0; k < chunks; ++k) r.parts[k] = Operand::r(regs[gp++]);
        } else {
            unsigned alignment = t == Type::I128 ? 16 : std::max(8u,t.alignment());
            stack = (stack+alignment-1)&~std::uint64_t(alignment-1);
            r.parts[0] = Operand::mem(stack_base,stack); r.memory = true;
            stack += (t.bytes()+7)&~std::uint64_t(7);
        }
        return r;
    }
};
}
