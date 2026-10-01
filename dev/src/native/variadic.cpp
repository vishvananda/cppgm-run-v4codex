#include "native/selection.h"
namespace native {
using namespace lowir_model;
void Selector::save_variadic_registers()
{
    // Capture at entry, before ordinary placement can reuse any incoming
    // carrier. va_start may occur arbitrarily late in the body.
    static const int gp[] = {XR_RDI,XR_RSI,XR_RDX,XR_RCX,XR_R8,XR_R9};
    vararg_save = home(0,Type::object(176,16),true);
    for (unsigned n = 0; n < 6; ++n) {
        auto at = vararg_save; at.displacement += n*8;
        move(at,Operand::r(gp[n]),Type::I64);
    }
    auto done = next_label++;
    emit(Op::Compare,Type::U8,{Operand::r(XR_RAX),Operand::imm(0)});
    emit(Op::Jcc,Type(),{Operand::label(done)}).condition = XC_E;
    for (unsigned n = 0; n < 8; ++n) {
        auto at = vararg_save; at.displacement += 48+n*16;
        move(at,Operand::r(xmm(n)),Type::F64);
    }
    begin_block(done,0);
}
void Selector::variadic(const lowir_model::Instruction& i)
{
    auto list = memory(arg(i,0));
    // Stable pointer while both scalar fields and an indirect pointer are read.
    list.address = true; move(Operand::r(XR_R11),list,Type::Ptr);
    if (i.opcode == Opcode::VaStart) {
        require(vararg_save.kind != Operand::None,"va_start outside variadic function");
        move(Operand::mem(XR_R11),Operand::imm(vararg_gp),Type::U32);
        move(Operand::mem(XR_R11,4),Operand::imm(vararg_fp),Type::U32);
        auto overflow = Operand::mem(XR_RBP,vararg_stack); overflow.address = true;
        move(Operand::r(XR_R10),overflow,Type::Ptr);
        move(Operand::mem(XR_R11,8),Operand::r(XR_R10),Type::Ptr);
        auto save = vararg_save; save.address = true;
        move(Operand::r(XR_R10),save,Type::Ptr);
        move(Operand::mem(XR_R11,16),Operand::r(XR_R10),Type::Ptr); return;
    }
    require(i.type.scalar() || i.type.complex(),"unsupported variadic value class");
    auto dest = allocate(i.destination.index,i.type);
    unsigned overflow = next_label++, done = next_label++;
    bool fp = i.type == Type::F32 || i.type == Type::F64 || (i.type.complex() && i.type.component() != Type::F80);
    unsigned fp_bytes = i.type.complex() && i.type.component() == Type::F64 ? 32 : 16;
    if (i.type != Type::F80 && !(i.type.complex() && i.type.component() == Type::F80)) {
        move(Operand::r(XR_R10),Operand::mem(XR_R11,fp ? 4 : 0),Type::U32);
        emit(Op::Compare,Type::U32,{Operand::r(XR_R10),Operand::imm(fp ? 192-fp_bytes : i.type == Type::I128 ? 40 : 48)});
        emit(Op::Jcc,Type(),{Operand::label(overflow)}).condition = XC_AE;
        move(Operand::r(XR_RAX),Operand::mem(XR_R11,16),Type::Ptr);
        emit(Op::Add,Type::Ptr,{Operand::r(XR_RAX),Operand::r(XR_R10)});
        emit(Op::Add,Type::U32,{Operand::r(XR_R10),Operand::imm(fp ? fp_bytes : i.type == Type::I128 ? 16 : 8)});
        move(Operand::mem(XR_R11,fp ? 4 : 0),Operand::r(XR_R10),Type::U32);
        if (i.type.complex()) {
            move(fragment(dest,0),Operand::mem(XR_RAX),Type::F64);
            if (i.type.component() == Type::F64) move(fragment(dest,8),Operand::mem(XR_RAX,16),Type::F64);
        } else move(dest,Operand::mem(XR_RAX),i.type);
        emit(Op::Jump,Type(),{Operand::label(done)});
    }
    begin_block(overflow,0);
    move(Operand::r(XR_RAX),Operand::mem(XR_R11,8),Type::Ptr);
    if (i.type.abi_alignment() > 8) {
        emit(Op::Add,Type::Ptr,{Operand::r(XR_RAX),Operand::imm(15)});
        emit(Op::And,Type::Ptr,{Operand::r(XR_RAX),Operand::imm(-16)});
    }
    move(Operand::r(XR_R10),Operand::r(XR_RAX),Type::Ptr);
    emit(Op::Add,Type::Ptr,{Operand::r(XR_R10),Operand::imm((i.type.bytes()+7)&~std::uint64_t(7))});
    move(Operand::mem(XR_R11,8),Operand::r(XR_R10),Type::Ptr);
    move(dest,Operand::mem(XR_RAX),i.type);
    begin_block(done,0);
}
} // namespace native
