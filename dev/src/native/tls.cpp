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
void Encoder::tls_wrappers()
{
    // Declarations denote runtime-provided accessors; definitions remain
    // ordinary functions, including their user-supplied initialization work.
    if (image.tls_wrappers.empty()) return;
    std::vector<bool> demanded(image.symbols.size());
    for (const auto& fix : image.code_fixups)
        if (fix.kind != Fixup::ThreadOffset) demanded[fix.symbol] = true;
    for (const auto& fix : image.data_fixups) demanded[fix.symbol] = true;
    for (unsigned id : image.tls_wrappers) if (demanded[id]) {
        image.symbols[id] = code.size(); image.defined[id] = true;
        tls_address(Operand::r(XR_RAX),Operand::symbol(lowir_model::SymbolId(id)));
        byte(0xc3);
    }
}
} // namespace native
