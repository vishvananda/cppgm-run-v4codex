#include "native/encoding.h"
#include "native/bulk_policy.h"
namespace native {
void Encoder::bulk(const Instruction& i)
{
    Operand dst = i.args[0], src = i.args[1];
    // A register bulk operand denotes the address itself; a memory operand
    // denotes a structured base/index/displacement address, not a pointer load.
    if (dst.kind == Operand::Reg) dst = Operand::mem(dst.reg);
    if (src.kind == Operand::Reg) src = Operand::mem(src.reg);
    // Vector scratch xmm15 is permanently reserved, outside ordinary placement.
    // Direct forms never alter GPR address carriers or condition flags.
    bool direct = direct_copy_bytes(i.bytes,i.alignment);
    if (i.op == Op::ZeroBytes && i.bytes == 16) {
        form(0x0fef,32,15,Operand::r(15),0,0,0x66);
        form(0x0f7f,32,15,dst,0,0,0xf3); return;
    }
    if (i.op == Op::CopyBytes && direct) {
        unsigned offset = 0;
        // Unaligned small copies commonly combine recently written scalar
        // fields. Widening those reads to 16 bytes defeats scalar store
        // forwarding on x86. Keep bounded scalar chunks without alignment
        // evidence; aligned aggregates retain the vector form.
        while (i.alignment >= 8 && offset+16 <= i.bytes) {
            auto from = src, to = dst; from.displacement += offset; to.displacement += offset;
            form(0x0f6f,32,15,from,0,0,0xf3); form(0x0f7f,32,15,to,0,0,0xf3);
            offset += 16;
        }
        for (unsigned bytes : {8u,4u,2u,1u}) while (offset+bytes <= i.bytes) {
            auto from = src, to = dst; from.displacement += offset; to.displacement += offset;
            Type t = bytes == 8 ? Type::I64 : bytes == 4 ? Type::U32 : bytes == 2 ? Type::U16 : Type::U8;
            load(Operand::r(XR_RAX),from,t,false); store(to,Operand::r(XR_RAX),t);
            offset += bytes;
        }
        return;
    }
    // Exact bounded byte comparison for scalar zero stores versus REP setup.
    // Emission is rewound locally; it cannot change symbol identities/fixups.
    if (i.op == Op::ZeroBytes && i.bytes <= 32) {
        auto begin = code.size(), fixes = image.code_fixups.size();
        unsigned offset = 0;
        for (unsigned bytes : {8u,4u,2u,1u}) while (offset+bytes <= i.bytes) {
            auto to = dst; to.displacement += offset;
            Type t = bytes == 8 ? Type::I64 : bytes == 4 ? Type::U32 : bytes == 2 ? Type::U16 : Type::U8;
            store(to,Operand::imm(0),t); offset += bytes;
        }
        // REP requires LEA dst (3..8), MOV ecx (5), MOV eax (5), REP STOSB (2).
        // Encode its actual bytes below when scalar is not unconditionally smaller.
        auto scalar_end = code.size();
        std::vector<unsigned char> scalar(code.begin()+begin,code.end());
        std::vector<Fixup> scalar_fixes(image.code_fixups.begin()+fixes,image.code_fixups.end());
        code.resize(begin); image.code_fixups.resize(fixes);
        form(0x8d,64,XR_RDI,dst);
        mov(Operand::r(XR_RCX),Operand::imm(i.bytes)); mov(Operand::r(XR_RAX),Operand::imm(0)); byte(0xf3); byte(0xaa);
        if (scalar_end-begin <= code.size()-begin) {
            code.resize(begin); image.code_fixups.resize(fixes);
            code.insert(code.end(),scalar.begin(),scalar.end());
            image.code_fixups.insert(image.code_fixups.end(),scalar_fixes.begin(),scalar_fixes.end());
        }
        return;
    }
    if (i.op == Op::CopyBytes) {
        // Staging both addresses avoids copy-register overlap in either order.
        // rax is reserved; the source's r10/r11 carriers remain intact.
        form(0x8d,64,XR_RAX,dst); form(0x8d,64,XR_RSI,src); mov(Operand::r(XR_RDI),Operand::r(XR_RAX));
    } else form(0x8d,64,XR_RDI,dst);
    mov(Operand::r(XR_RCX),Operand::imm(i.bytes));
    if (i.op == Op::CopyBytes) { byte(0xf3); byte(0xa4); }
    else { mov(Operand::r(XR_RAX),Operand::imm(0)); byte(0xf3); byte(0xaa); }
}
} // namespace native
