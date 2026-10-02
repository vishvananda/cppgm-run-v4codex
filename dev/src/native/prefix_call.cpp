#include "native/encoding.h"
namespace native {
void Encoder::prefix_call(const Instruction& i)
{
    lowir_model::require(i.strlen_prefix == 16 && i.args[0].kind == Operand::Symbol &&
        i.arg_registers == (1u<<XR_RDI) && !i.bytes,"invalid strlen prefix ABI");
    // Linux x86-64 mappings have at least 4K page granularity. The unaligned
    // probe stays inside the pointer's readable page; the last 15 offsets use
    // the original call, as does a prefix with no zero. RDI stays unchanged.
    mov(Operand::r(XR_RAX),Operand::r(XR_RDI));
    form(0x81,32,4,Operand::r(XR_RAX),4,4095);
    form(0x81,32,7,Operand::r(XR_RAX),4,4080);
    auto boundary = local_jump(XC_A);
    form(0x0f6f,32,0,Operand::mem(XR_RDI),0,0,0xf3); // movdqu xmm0, [rdi]
    form(0x0fef,32,1,Operand::r(1),0,0,0x66);       // pxor xmm1, xmm1
    form(0x0f74,32,0,Operand::r(1),0,0,0x66);       // pcmpeqb xmm0, xmm1
    form(0x0fd7,32,XR_RAX,Operand::r(0),0,0,0x66); // pmovmskb eax, xmm0
    form(0x85,32,XR_RAX,Operand::r(XR_RAX));
    auto absent = local_jump(XC_E);
    form(0x0fbc,32,XR_RAX,Operand::r(XR_RAX));      // bsf eax, eax (nonzero)
    auto done = local_jump(-1);
    local_target(boundary); local_target(absent);
    call(i.args[0]);
    local_target(done);
}
} // namespace native
