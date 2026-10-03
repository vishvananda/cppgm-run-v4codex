#include "native/selection.h"
namespace native {
using namespace lowir_model;
bool Selector::constant_quotient(const lowir_model::Instruction& i, Operand input, Operand result)
{
    if (i.type.width() != 64 || (i.operation != Operation::Div && i.operation != Operation::Udiv)) return false;
    auto literal = arg(i,1);
    if (literal.kind != lowir_model::Operand::Integer) return false;
    std::uint64_t divisor = literal.data.integer;
    bool sign = i.operation == Operation::Div;
    // Keep the existing traps for zero and the signed MIN/-1 overflow case.
    if (!divisor || (sign && divisor == ~std::uint64_t(0))) return false;
    bool negative = sign && std::int64_t(divisor) < 0;
    if (negative) divisor = 0-divisor;
    auto ax = Operand::r(XR_RAX), dx = Operand::r(XR_RDX);
    auto original = Operand::r(XR_R10), mask = Operand::r(XR_R11);
    move(ax,input,consumed_type(arg(i,0),i.type));
    if (sign) {
        move(mask,ax,Type::I64);
        emit(Op::Sar,Type::I64,{mask,Operand::imm(63)});
        emit(Op::Xor,Type::I64,{ax,mask});
        emit(Op::Sub,Type::I64,{ax,mask});
    }
    unsigned shift = 0;
    for (auto n = divisor; n > 1; n >>= 1) ++shift;
    if (!(divisor & (divisor-1))) {
        if (shift) emit(Op::Shr,Type::I64,{ax,Operand::imm(shift)});
        move(dx,ax,Type::I64);
    } else if (!sign && divisor > (std::uint64_t(1)<<63)) {
        // The only possible unsigned quotients in this range are zero and one.
        move(mask,Operand::imm(divisor),Type::I64);
        emit(Op::Compare,Type::I64,{ax,mask});
        emit(Op::Set,Type::I1,{dx}).condition = XC_AE;
        emit(Op::ExtendUnsigned,Type::U8,{dx,dx});
    } else {
        // Choose ceil(2^(64+shift)/d). Its rounding error must be <=2^shift
        // so that no 64-bit dividend can cross the next quotient boundary.
        // Otherwise double the precision; the resulting 65-bit multiplier
        // is represented by its low word and one implicit high bit.
        unsigned __int128 power = static_cast<unsigned __int128>(1) << (64+shift);
        unsigned __int128 multiplier = power/divisor;
        std::uint64_t remainder = power%divisor;
        bool add = divisor-remainder > (std::uint64_t(1)<<shift);
        if (add) multiplier = (power*2)/divisor;
        ++multiplier;
        move(original,ax,Type::I64);
        move(ax,Operand::imm(std::uint64_t(multiplier)),Type::I64);
        emit(Op::MulWide,Type::I64,{original});
        if (add) {
            // (n + high(n*m))/2 without overflowing the 64-bit sum.
            emit(Op::Sub,Type::I64,{original,dx});
            emit(Op::Shr,Type::I64,{original,Operand::imm(1)});
            emit(Op::Add,Type::I64,{dx,original});
        }
        if (shift) emit(Op::Shr,Type::I64,{dx,Operand::imm(shift)});
    }
    if (sign) {
        emit(Op::Xor,Type::I64,{dx,mask});
        emit(Op::Sub,Type::I64,{dx,mask});
        if (negative) emit(Op::Neg,Type::I64,{dx});
    }
    move(result,dx,i.type);
    return true;
}
} // namespace native
