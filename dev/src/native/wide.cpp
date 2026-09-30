#include "native/selection.h"
namespace native {
using namespace lowir_model;
X86Condition branch_condition(Operation);
static Operation low_predicate(Operation op)
{
    return op == Operation::Lt ? Operation::Ult : op == Operation::Le ? Operation::Ule :
        op == Operation::Gt ? Operation::Ugt : op == Operation::Ge ? Operation::Uge : op;
}
void Selector::wide_compare(const lowir_model::Instruction& i, bool branch)
{
    auto lhs = value(arg(i,0),i.type), rhs = value(arg(i,1),i.type);
    if (!branch) {
        Operand dest = allocate(i.destination.index,Type::I64);
        auto low = Operand::r(XR_RAX), high = Operand::r(XR_R10), equal = Operand::r(XR_R11);
        move(high,fragment(lhs,0),Type::I64);
        emit(Op::Compare,Type::I64,{high,fragment(rhs,0)});
        emit(Op::Set,Type::I1,{low}).condition = branch_condition(low_predicate(i.operation));
        emit(Op::ExtendUnsigned,Type::U8,{low,low});
        move(high,fragment(lhs,8),Type::I64);
        emit(Op::Compare,Type::I64,{high,fragment(rhs,8)});
        emit(Op::Set,Type::I1,{high}).condition = branch_condition(i.operation);
        emit(Op::ExtendUnsigned,Type::U8,{high,high});
        if (i.operation == Operation::Eq || i.operation == Operation::Ne) {
            emit(i.operation == Operation::Eq ? Op::And : Op::Or,Type::I64,{low,high});
        } else {
            // For inclusive predicates, unequal high words use strict ordering.
            auto strict = i.operation == Operation::Le ? Operation::Lt : i.operation == Operation::Ge ? Operation::Gt :
                i.operation == Operation::Ule ? Operation::Ult : i.operation == Operation::Uge ? Operation::Ugt : i.operation;
            emit(Op::Set,Type::I1,{high}).condition = branch_condition(strict);
            emit(Op::Set,Type::I1,{equal}).condition = XC_E;
            emit(Op::ExtendUnsigned,Type::U8,{high,high}); emit(Op::ExtendUnsigned,Type::U8,{equal,equal});
            emit(Op::And,Type::I64,{low,equal}); emit(Op::Or,Type::I64,{low,high});
        }
        move(dest,low,Type::I64); return;
    }
    const auto& next = p.instructions[position];
    unsigned yes = edge_target(arg(next,1).ref), no = edge_target(arg(next,2).ref);
    move(Operand::r(XR_R10),fragment(lhs,8),Type::I64);
    emit(Op::Compare,Type::I64,{Operand::r(XR_R10),fragment(rhs,8)});
    auto op = i.operation;
    if (op == Operation::Eq || op == Operation::Ne)
        emit(Op::Jcc,Type(),{Operand::label(op == Operation::Eq ? no : yes)}).condition = XC_NE;
    else {
        bool less = op == Operation::Lt || op == Operation::Le || op == Operation::Ult || op == Operation::Ule;
        bool sign = op == Operation::Lt || op == Operation::Le || op == Operation::Gt || op == Operation::Ge;
        emit(Op::Jcc,Type(),{Operand::label(yes)}).condition = less ? (sign ? XC_L : XC_B) : (sign ? XC_G : XC_A);
        emit(Op::Jcc,Type(),{Operand::label(no)}).condition = XC_NE;
    }
    move(Operand::r(XR_R10),fragment(lhs,0),Type::I64);
    emit(Op::Compare,Type::I64,{Operand::r(XR_R10),fragment(rhs,0)});

}
void Selector::wide_arithmetic(const lowir_model::Instruction& i)
{
    if (i.operation == Operation::Div || i.operation == Operation::Udiv || i.operation == Operation::Mod || i.operation == Operation::Umod) { wide_division(i); return; }
    if (i.operation == Operation::Shl || i.operation == Operation::Shr || i.operation == Operation::Ushr) { wide_shift(i); return; }
    auto lhs = value(arg(i,0),i.type);
    if (i.operation == Operation::Not) {
        auto reg = Operand::r(XR_R10);
        move(reg,fragment(lhs,0),Type::I64); emit(Op::Or,Type::I64,{reg,fragment(lhs,8)});
        emit(Op::Compare,Type::I64,{reg,Operand::imm(0)});
        if (state(i.destination.index).compare_branch) return;
        auto dst = allocate(i.destination.index,Type::I128);
        emit(Op::Set,Type::I1,{reg}).condition = XC_E; emit(Op::ExtendUnsigned,Type::U8,{reg,reg});
        move(fragment(dst,0),reg,Type::I64); move(fragment(dst,8),Operand::imm(0),Type::I64); return;
    }
    // Implicit widening may emit scratch-using moves. Finish preparing both
    // logical inputs before loading either fixed arithmetic carrier.
    Operand rhs;
    if (i.opcode == Opcode::Binary) rhs = value(arg(i,1),i.type);
    auto dst = allocate(i.destination.index,i.type);
    Operand lo = Operand::r(XR_R10), hi = Operand::r(XR_RAX);
    move(lo,fragment(lhs,0),Type::I64); move(hi,fragment(lhs,8),Type::I64);
    if (i.opcode == Opcode::Unary) {
        if (i.operation == Operation::Neg) {
            emit(Op::Neg,Type::I64,{lo}); emit(Op::Adc,Type::I64,{hi,Operand::imm(0)}); emit(Op::Neg,Type::I64,{hi});
        } else if (i.operation == Operation::Bitnot) {
            emit(Op::Not,Type::I64,{lo}); emit(Op::Not,Type::I64,{hi});
        } else throw ParseError("native wide unary operation not implemented");
    } else {
        Op op = i.operation == Operation::Add ? Op::Add : i.operation == Operation::Sub ? Op::Sub :
            i.operation == Operation::And ? Op::And : i.operation == Operation::Or ? Op::Or : Op::Xor;
        if (i.operation == Operation::Mul) {
            // Low 128 bits: low*low plus the two cross products. Only reserved
            // scratch and the declared fixed rdx multiply carrier are clobbered.
            move(lo,fragment(lhs,8),Type::I64); emit(Op::Mul,Type::I64,{lo,fragment(rhs,0)});
            move(hi,fragment(lhs,0),Type::I64); move(Operand::r(XR_R11),fragment(rhs,0),Type::I64);
            emit(Op::MulWide,Type::I64,{Operand::r(XR_R11)});
            emit(Op::Add,Type::I64,{Operand::r(XR_RDX),lo});
            move(lo,fragment(lhs,0),Type::I64); emit(Op::Mul,Type::I64,{lo,fragment(rhs,8)});
            emit(Op::Add,Type::I64,{Operand::r(XR_RDX),lo});
            move(fragment(dst,0),hi,Type::I64); move(fragment(dst,8),Operand::r(XR_RDX),Type::I64); return;
        }
        require(i.operation == Operation::Add || i.operation == Operation::Sub || i.operation == Operation::And ||
            i.operation == Operation::Or || i.operation == Operation::Xor,"native wide binary operation not implemented");
        emit(op,Type::I64,{lo,fragment(rhs,0)});
        if (op == Op::Add) op = Op::Adc;
        if (op == Op::Sub) op = Op::Sbb;
        emit(op,Type::I64,{hi,fragment(rhs,8)});
    }
    move(fragment(dst,0),lo,Type::I64); move(fragment(dst,8),hi,Type::I64);
}
void Selector::wide_conversion(const lowir_model::Instruction& i)
{
    auto src = value(arg(i,0),i.source_type), dst = allocate(i.destination.index,i.type);
    if (i.type == Type::I128) {
        Operand low = Operand::r(XR_R10), high = Operand::r(XR_RAX);
        if (i.source_type == Type::I128) { move(dst,src,i.type); return; }
        emit(i.operation == Operation::Sext ? Op::ExtendSigned : Op::ExtendUnsigned,i.source_type,{low,src});
        move(high,low,Type::I64);
        if (i.operation == Operation::Sext) emit(Op::Sar,Type::I64,{high,Operand::imm(63)});
        else move(high,Operand::imm(0),Type::I64);
        move(fragment(dst,0),low,Type::I64); move(fragment(dst,8),high,Type::I64);
    } else {
        Operand reg = dst.kind == Operand::Reg ? dst : Operand::r(XR_R10);
        emit(unsigned_type(i.type) ? Op::ExtendUnsigned : Op::ExtendSigned,i.type,{reg,fragment(src,0)});
        move(dst,reg,i.type);
    }
}
}
