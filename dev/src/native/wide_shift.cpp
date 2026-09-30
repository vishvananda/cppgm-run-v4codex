#include "native/selection.h"
namespace native {
using namespace lowir_model;
void Selector::wide_shift(const lowir_model::Instruction& i)
{
    auto lhs = value(arg(i,0),Type::I128), rhs = value(arg(i,1),Type::I128);
    auto dst = allocate(i.destination.index,Type::I128);
    Operand low = Operand::r(XR_R10), high = Operand::r(XR_RAX);
    Operand count = rhs.kind == Operand::Reg ? rhs : fragment(rhs,0);
    if (count.kind != Operand::Immediate) { move(Operand::r(XR_RCX),count,Type::I64); count = Operand::r(XR_RCX); }
    move(low,fragment(lhs,0),Type::I64); move(high,fragment(lhs,8),Type::I64);
    bool left = i.operation == Operation::Shl, sign = i.operation == Operation::Shr;
    auto small = [&]() {
        emit(left ? Op::Shld : Op::Shrd,Type::I64,{left ? high : low,left ? low : high,count});
        emit(left ? Op::Shl : sign ? Op::Sar : Op::Shr,Type::I64,{left ? low : high,count});
    };
    auto large = [&]() {
        if (left) { move(high,low,Type::I64); emit(Op::Shl,Type::I64,{high,count}); move(low,Operand::imm(0),Type::I64); }
        else {
            move(low,high,Type::I64); emit(sign ? Op::Sar : Op::Shr,Type::I64,{low,count});
            if (sign) emit(Op::Sar,Type::I64,{high,Operand::imm(63)}); else move(high,Operand::imm(0),Type::I64);
        }
    };
    if (count.kind == Operand::Immediate) { if (count.bits < 64) small(); else large(); }
    else {
        unsigned big = next_label++, done = next_label++;
        emit(Op::Compare,Type::I64,{count,Operand::imm(64)});
        emit(Op::Jcc,Type(),{Operand::label(big)}).condition = XC_AE;
        small(); emit(Op::Jump,Type(),{Operand::label(done)});
        begin_block(big,0); large(); begin_block(done,0);
    }
    move(fragment(dst,0),low,Type::I64); move(fragment(dst,8),high,Type::I64);
}
void Selector::wide_atomic(const lowir_model::Instruction& i)
{
    // cmpxchg16b is the shared atomic primitive. Every width-128 object is
    // aligned by native layout. RBX is saved locally; RCX/RDX are declared
    // fixed effects to parameter placement and incoming-carrier flow.
    if (atomic_scratch.kind == Operand::None) atomic_scratch = home(0,Type::object(24,8),true);
    auto saved = fragment(atomic_scratch,0);
    move(saved,Operand::r(XR_RBX),Type::I64);
    Operand source, expected_home;
    if (i.opcode != Opcode::AtomicLoad && i.opcode != Opcode::AtomicCompareExchange)
        source = value(arg(i,i.opcode == Opcode::AtomicStore ? 0 : 1),Type::I128);
    else if (i.opcode == Opcode::AtomicCompareExchange) source = value(arg(i,2),Type::I128);
    auto address = memory(arg(i,i.opcode == Opcode::AtomicStore ? 1 : 0));
    address.address = true;
    // Preserve the address while loading either expected or desired operands.
    auto address_home = fragment(atomic_scratch,8); move(address_home,address,Type::Ptr);
    if (i.opcode == Opcode::AtomicCompareExchange) {
        auto expected = memory(arg(i,1)); expected.address = true;
        expected_home = fragment(atomic_scratch,16); move(expected_home,expected,Type::Ptr);
    }
    auto lo = Operand::r(XR_RAX), hi = Operand::r(XR_RDX);
    if (expected_home.kind != Operand::None) {
        move(Operand::r(XR_R11),expected_home,Type::Ptr);
        move(lo,Operand::mem(XR_R11),Type::I64); move(hi,Operand::mem(XR_R11,8),Type::I64);
    } else { move(lo,Operand::imm(0),Type::I64); move(hi,Operand::imm(0),Type::I64); }
    unsigned retry = next_label++;
    if (i.opcode != Opcode::AtomicLoad && i.opcode != Opcode::AtomicCompareExchange) begin_block(retry,0);
    if (i.opcode == Opcode::AtomicLoad) {
        move(Operand::r(XR_RBX),Operand::imm(0),Type::I64); move(Operand::r(XR_RCX),Operand::imm(0),Type::I64);
    } else if (i.opcode == Opcode::AtomicAddFetch) {
        move(Operand::r(XR_RBX),lo,Type::I64); move(Operand::r(XR_RCX),hi,Type::I64);
        emit(Op::Add,Type::I64,{Operand::r(XR_RBX),fragment(source,0)});
        emit(Op::Adc,Type::I64,{Operand::r(XR_RCX),fragment(source,8)});
    } else {
        move(Operand::r(XR_RBX),fragment(source,0),Type::I64); move(Operand::r(XR_RCX),fragment(source,8),Type::I64);
    }
    move(Operand::r(XR_R10),address_home,Type::Ptr);
    emit(Op::CmpxchgWide,Type::I128,{Operand::mem(XR_R10)});
    if (i.opcode == Opcode::AtomicCompareExchange) {
        unsigned success = next_label++;
        emit(Op::Jcc,Type(),{Operand::label(success)}).condition = XC_E;
        move(Operand::r(XR_R11),expected_home,Type::Ptr);
        move(Operand::mem(XR_R11),lo,Type::I64); move(Operand::mem(XR_R11,8),hi,Type::I64);
        begin_block(success,0);
        emit(Op::Set,Type::I1,{Operand::r(XR_R10)}).condition = XC_E;
        emit(Op::ExtendUnsigned,Type::U8,{Operand::r(XR_R10),Operand::r(XR_R10)});
    } else if (i.opcode != Opcode::AtomicLoad)
        emit(Op::Jcc,Type(),{Operand::label(retry)}).condition = XC_NE;
    if (i.destination) {
        auto dst = allocate(i.destination.index,i.opcode == Opcode::AtomicCompareExchange ? Type::I64 : Type::I128);
        if (i.opcode == Opcode::AtomicCompareExchange) {
            move(Operand::r(XR_RBX),saved,Type::I64); move(dst,Operand::r(XR_R10),Type::I64); return;
        }
        if (i.opcode == Opcode::AtomicAddFetch) { lo = Operand::r(XR_RBX); hi = Operand::r(XR_RCX); }
        move(fragment(dst,0),lo,Type::I64); move(fragment(dst,8),hi,Type::I64);
    }
    move(Operand::r(XR_RBX),saved,Type::I64);
}
}
