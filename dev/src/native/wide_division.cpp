#include "native/selection.h"
namespace native {
using namespace lowir_model;
void Selector::wide_division(const lowir_model::Instruction& i)
{
    // Restoring division has exactly 128 iterations and constant MIR/code size.
    // No runtime helper or host operation implements the target computation.
    auto lhs = value(arg(i,0),Type::I128), rhs = value(arg(i,1),Type::I128);
    auto dst = allocate(i.destination.index,Type::I128);
    auto divisor = home(0,Type::I128,true), count = home(0,Type::I64,true);
    bool sign = i.operation == Operation::Div || i.operation == Operation::Mod;
    bool remainder = i.operation == Operation::Mod || i.operation == Operation::Umod;
    Operand sign_home;
    if (sign) sign_home = home(0,Type::I64,true);
    auto qlo = Operand::r(XR_R10), qhi = Operand::r(XR_RAX);
    auto rlo = Operand::r(XR_RDX), rhi = Operand::r(XR_RCX);
    auto negate = [&](Operand low, Operand high) {
        emit(Op::Neg,Type::I64,{low}); emit(Op::Adc,Type::I64,{high,Operand::imm(0)}); emit(Op::Neg,Type::I64,{high});
    };
    move(qlo,fragment(rhs,0),Type::I64); move(qhi,fragment(rhs,8),Type::I64);
    if (sign) {
        move(sign_home,qhi,Type::I64);
        unsigned positive = next_label++;
        emit(Op::Compare,Type::I64,{qhi,Operand::imm(0)});
        emit(Op::Jcc,Type(),{Operand::label(positive)}).condition = XC_GE;
        negate(qlo,qhi); begin_block(positive,0);
    }
    move(fragment(divisor,0),qlo,Type::I64); move(fragment(divisor,8),qhi,Type::I64);
    move(qlo,fragment(lhs,0),Type::I64); move(qhi,fragment(lhs,8),Type::I64);
    if (sign) {
        move(Operand::r(XR_R11),qhi,Type::I64);
        if (!remainder) emit(Op::Xor,Type::I64,{Operand::r(XR_R11),sign_home});
        move(sign_home,Operand::r(XR_R11),Type::I64);
        unsigned positive = next_label++;
        emit(Op::Compare,Type::I64,{qhi,Operand::imm(0)});
        emit(Op::Jcc,Type(),{Operand::label(positive)}).condition = XC_GE;
        negate(qlo,qhi); begin_block(positive,0);
    }
    move(rlo,Operand::imm(0),Type::I64); move(rhi,Operand::imm(0),Type::I64);
    move(count,Operand::imm(128),Type::I64);
    unsigned loop = next_label++, subtract = next_label++, next = next_label++;
    begin_block(loop,0);
    emit(Op::Shl,Type::I64,{qlo,Operand::imm(1)});
    emit(Op::Adc,Type::I64,{qhi,qhi});
    emit(Op::Adc,Type::I64,{rlo,rlo}); emit(Op::Adc,Type::I64,{rhi,rhi});
    emit(Op::Jcc,Type(),{Operand::label(subtract)}).condition = XC_B;
    emit(Op::Compare,Type::I64,{rhi,fragment(divisor,8)});
    emit(Op::Jcc,Type(),{Operand::label(subtract)}).condition = XC_A;
    emit(Op::Jcc,Type(),{Operand::label(next)}).condition = XC_B;
    emit(Op::Compare,Type::I64,{rlo,fragment(divisor,0)});
    emit(Op::Jcc,Type(),{Operand::label(next)}).condition = XC_B;
    begin_block(subtract,0);
    emit(Op::Sub,Type::I64,{rlo,fragment(divisor,0)}); emit(Op::Sbb,Type::I64,{rhi,fragment(divisor,8)});
    emit(Op::Or,Type::I64,{qlo,Operand::imm(1)});
    begin_block(next,0);
    emit(Op::Sub,Type::I64,{count,Operand::imm(1)});
    emit(Op::Jcc,Type(),{Operand::label(loop)}).condition = XC_NE;
    if (remainder) { move(qlo,rlo,Type::I64); move(qhi,rhi,Type::I64); }
    if (sign) {
        unsigned positive = next_label++;
        move(Operand::r(XR_R11),sign_home,Type::I64);
        emit(Op::Compare,Type::I64,{Operand::r(XR_R11),Operand::imm(0)});
        emit(Op::Jcc,Type(),{Operand::label(positive)}).condition = XC_GE;
        negate(qlo,qhi); begin_block(positive,0);
    }
    move(fragment(dst,0),qlo,Type::I64); move(fragment(dst,8),qhi,Type::I64);
}
}
