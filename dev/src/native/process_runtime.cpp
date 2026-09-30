#include "native/model.h"
namespace native {
// Standalone process primitives. Host-object runtime bindings belong to PA26.
// Syscall exposes the real inputs; Linux clobbers rax, rcx and r11. These leaf
// bodies use only caller-saved registers and have no hidden stack adjustment.
Function process_runtime(SymbolId symbol, ir_model::SymbolRole role)
{
    Function f; f.symbol = symbol; f.frame_pointer = false; f.shared_epilogue = false;
    f.result = role == ir_model::SR_ALLOCATE_MEMORY ? Type::Ptr : Type();
    if (role == ir_model::SR_ALLOCATE_MEMORY || role == ir_model::SR_FREE_MEMORY)
        f.params.push_back({0,role == ir_model::SR_ALLOCATE_MEMORY ? Type::I64 : Type::Ptr,Operand::r(XR_RDI)});
    auto block = [&](unsigned id) {
        Block b; b.id = id; b.name = 0; b.instructions.begin = f.instructions.size(); f.blocks.push_back(b);
    };
    auto emit = [&](Op op, Type type, std::initializer_list<Operand> args) -> Instruction& {
        Instruction i(op,type); i.count = args.size(); std::copy(args.begin(),args.end(),i.args.begin());
        f.instructions.push_back(i); ++f.blocks.back().instructions.count; return f.instructions.back();
    };
    auto reg = [](int r) { return Operand::r(r); };
    auto mov = [&](int r, Operand value) { emit(Op::Mov,Type::I64,{reg(r),value}); };
    auto branch = [&](unsigned id, X86Condition cc) { emit(Op::Jcc,Type(),{Operand::label(id)}).condition = cc; };
    auto syscall = [&](unsigned mask) { emit(Op::Syscall,Type(),{}).arg_registers = mask | (1u<<XR_RAX); };
    block(1);
    if (role == ir_model::SR_ALLOCATE_MEMORY) {
        // A private 16-byte prefix keeps the mapping extent and gives every
        // returned address fundamental alignment, including new(0).
        mov(XR_RSI,reg(XR_RDI)); emit(Op::Add,Type::I64,{reg(XR_RSI),Operand::imm(16)});
        branch(3,XC_B);
        mov(XR_RDI,Operand::imm(0)); mov(XR_RDX,Operand::imm(3)); // read/write
        mov(XR_R10,Operand::imm(0x22)); // MAP_PRIVATE | MAP_ANONYMOUS
        mov(XR_R8,Operand::imm(-1)); mov(XR_R9,Operand::imm(0)); mov(XR_RAX,Operand::imm(9));
        syscall((1u<<XR_RDI)|(1u<<XR_RSI)|(1u<<XR_RDX)|(1u<<XR_R10)|(1u<<XR_R8)|(1u<<XR_R9));
        emit(Op::Compare,Type::I64,{reg(XR_RAX),Operand::imm(-4095)}); branch(3,XC_AE);
        block(2);
        emit(Op::Store,Type::I64,{Operand::mem(XR_RAX),reg(XR_RSI)});
        emit(Op::Add,Type::I64,{reg(XR_RAX),Operand::imm(16)});
        emit(Op::Return,Type::Ptr,{reg(XR_RAX)});
    } else if (role == ir_model::SR_FREE_MEMORY) {
        emit(Op::Compare,Type::Ptr,{reg(XR_RDI),Operand::imm(0)}); branch(2,XC_E);
        emit(Op::Sub,Type::I64,{reg(XR_RDI),Operand::imm(16)});
        emit(Op::Load,Type::I64,{reg(XR_RSI),Operand::mem(XR_RDI)});
        mov(XR_RAX,Operand::imm(11)); syscall((1u<<XR_RDI)|(1u<<XR_RSI)); // munmap
        block(2); emit(Op::Return,Type(),{}); return f;
    } else lowir_model::require(role == ir_model::SR_TERMINATE || role == ir_model::SR_PURE_VIRTUAL,
        "unsupported process runtime role");
    block(3); mov(XR_RDI,Operand::imm(1)); emit(Op::Exit,Type(),{}); emit(Op::Trap,Type(),{});
    return f;
}
} // namespace native
