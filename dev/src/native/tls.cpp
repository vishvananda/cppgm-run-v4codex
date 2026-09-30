#include "native/selection.h"
#include "native/encoding.h"
namespace native {
bool Selector::tls_symbol(unsigned id) const
{
    return p.symbols[id-1].metadata.storage == ir_model::GSM_THREAD_LOCAL;
}
void Selector::tls_address(Operand to, Operand symbol)
{
    unsigned wrapper = workspace.tls_wrappers[symbol.id];
    if (wrapper) symbol.id = wrapper;
    emit(Op::TlsAddr,Type::Ptr,{to,symbol});
}
void Encoder::tls_address(Operand to, Operand symbol)
{
    unsigned target = image.tls_targets.at(symbol.id);
    lowir_model::require(target && to.kind == Operand::Reg,"invalid TLS address fact");
    // SysV local-exec address: FS:0 is the current thread pointer. The
    // selected destination is the only register changed by this operation.
    byte(0x64); byte(0x48 | (to.reg >= 8 ? 4 : 0)); byte(0x8b);
    byte((to.reg&7)*8+4); byte(0x25); number(0,4);
    // Force a disp32 form so the final TLS offset is a typed image fixup.
    byte(0x48 | (to.reg >= 8 ? 5 : 0)); byte(0x8d);
    byte(0x80 | (to.reg&7)*8 | ((to.reg&7) == 4 ? 4 : to.reg&7));
    if ((to.reg&7) == 4) byte(0x24);
    Fixup fix; fix.kind = Fixup::ThreadOffset; fix.symbol = target;
    fix.offset = code.size(); fix.addend = symbol.displacement;
    image.code_fixups.push_back(fix); number(0,4);
}
void Encoder::tls_startup()
{
    // This standalone image owns the initial thread's storage. Later hosted
    // object output will bind these same storage identities to its TLS ABI.
    mov(Operand::r(XR_RAX),Operand::imm(158)); // arch_prctl
    mov(Operand::r(XR_RDI),Operand::imm(0x1002)); // ARCH_SET_FS
    mov(Operand::r(XR_RSI),image.runtime(RuntimeEntity::ThreadPointer));
    byte(0x0f); byte(0x05);
    form(0x85,64,XR_RAX,Operand::r(XR_RAX));
    auto okay = local_jump(XC_E);
    mov(Operand::r(XR_RDI),Operand::imm(127));
    mov(Operand::r(XR_RAX),Operand::imm(60)); byte(0x0f); byte(0x05);
    local_target(okay);
}
Function builtin_tls(const lowir_model::Program& p, const lowir_model::Function& source)
{
    const auto& signature = p.signatures[source.signature.index-1];
    lowir_model::require(!signature.parameters.count && signature.result == Type::Ptr,
        "invalid TLS wrapper signature");
    Function f; f.symbol = source.symbol; f.result = Type::Ptr;
    f.frame_pointer = false; f.shared_epilogue = false;
    Block block; block.id = p.blocks.size()+1; block.name = 0;
    block.instructions.begin = 0; block.instructions.count = 2;
    f.blocks.push_back(block);
    Instruction address(Op::TlsAddr,Type::Ptr);
    address.count = 2; address.args[0] = Operand::r(XR_RAX);
    address.args[1] = Operand::symbol(p.symbols[source.symbol.index-1].metadata.tls_for);
    f.instructions.push_back(address);
    Instruction result(Op::Return,Type::Ptr);
    result.count = 1; result.args[0] = Operand::r(XR_RAX);
    f.instructions.push_back(result);
    return f;
}
} // namespace native
