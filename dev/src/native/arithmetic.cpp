#include "native/selection.h"
namespace native {
using namespace lowir_model;
static Op binary_op(Operation op)
{
    switch (op) {
    case Operation::Add: return Op::Add;
    case Operation::Sub: return Op::Sub;
    case Operation::Mul: return Op::Mul;
    case Operation::And: return Op::And;
    case Operation::Or: return Op::Or;
    case Operation::Xor: return Op::Xor;
    case Operation::Shl: return Op::Shl;
    case Operation::Shr: return Op::Sar;
    case Operation::Ushr: return Op::Shr;
    default: throw ParseError("invalid native binary operation");
    }
}
void Selector::arithmetic(const lowir_model::Instruction& i)
{
    if (i.type.floating()) { floating_arithmetic(i); return; }
    require(scalar_integer(i.type), "native arithmetic class not implemented");
    if (state(i.destination.index).compare_branch) {
        auto left = in_register(value(arg(i,0),i.type),value_type(arg(i,0),i.type),XR_R10);
        emit(Op::Compare,i.type,{left,Operand::imm(0)}); return;
    }
    Operand dest = allocate(i.destination.index,i.type);
    Operand result = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
    auto lhs = value(arg(i,0),i.type);
    if (i.operation == Operation::Div || i.operation == Operation::Mod ||
        i.operation == Operation::Udiv || i.operation == Operation::Umod) {
        bool sign = i.operation == Operation::Div || i.operation == Operation::Mod;
        move(Operand::r(XR_R11),value(arg(i,1),i.type),value_type(arg(i,1),i.type));
        move(Operand::r(XR_RAX),lhs,value_type(arg(i,0),i.type));
        // Division uses full logical register values. Explicit operand-width
        // normalization is necessary even for unsigned division of signed types.
        if (i.type.width() < 64) {
            emit(sign ? Op::ExtendSigned : Op::ExtendUnsigned,i.type,{Operand::r(XR_RAX),Operand::r(XR_RAX)});
            emit(sign ? Op::ExtendSigned : Op::ExtendUnsigned,i.type,{Operand::r(XR_R11),Operand::r(XR_R11)});
        }
        if (sign) emit(Op::SignDividend,Type::I64,{});
        else emit(Op::Mov,Type::I64,{Operand::r(XR_RDX),Operand::imm(0)});
        emit(sign ? Op::Div : Op::Udiv,Type::I64,{Operand::r(XR_R11)});
        bool remainder = i.operation == Operation::Mod || i.operation == Operation::Umod;
        move(result,Operand::r(remainder ? XR_RDX : XR_RAX),i.type);
    } else if (i.opcode == Opcode::Unary) {
        move(result,lhs,value_type(arg(i,0),i.type));
        if (i.operation == Operation::Not) {
            emit(Op::Compare,i.type,{result,Operand::imm(0)});
            emit(Op::Set,Type::I1,{result}).condition = XC_E;
            emit(Op::ExtendUnsigned,Type::U8,{result,result});
        } else {
            Op op = i.operation == Operation::Neg ? Op::Neg : i.operation == Operation::Bitnot ? Op::Not : Op::Bswap;
            emit(op,i.type,{result});
        }
    } else {
        Operand rhs = value(arg(i,1),i.type);
        Type rt = value_type(arg(i,1),i.type);
        // A memory home contains its own width, never the width of a consumer.
        if (rhs.address || (rhs.kind == Operand::Memory && rt != i.type)) rhs = in_register(rhs,rt,XR_R11);
        if (i.operation == Operation::Shl || i.operation == Operation::Shr || i.operation == Operation::Ushr) {
            if (rhs.kind != Operand::Immediate) { move(Operand::r(XR_RCX),rhs,rt); rhs = Operand::r(XR_RCX); }
            move(result,lhs,value_type(arg(i,0),i.type));
            if (i.operation == Operation::Ushr && i.type.width() < 64)
                emit(Op::ExtendUnsigned,i.type,{result,result});
        } else move(result,lhs,value_type(arg(i,0),i.type));
        emit(binary_op(i.operation),i.type,{result,rhs});
    }
    normalize_register(result,i.type);
    move(dest,result,i.type);
}
static X86Condition condition(Operation op)
{
    switch (op) {
    case Operation::Eq: return XC_E;
    case Operation::Ne: return XC_NE;
    case Operation::Lt: return XC_L;
    case Operation::Le: return XC_LE;
    case Operation::Gt: return XC_G;
    case Operation::Ge: return XC_GE;
    case Operation::Ult: return XC_B;
    case Operation::Ule: return XC_BE;
    case Operation::Ugt: return XC_A;
    case Operation::Uge: return XC_AE;
    default: throw ParseError("invalid native comparison");
    }
}
void Selector::compare(const lowir_model::Instruction& i, bool branch)
{
    if (i.type.floating()) { floating_compare(i,branch); return; }
    require(scalar_integer(i.type), "native comparison class not implemented");
    Operand left = value(arg(i,0),i.type), right = value(arg(i,1),i.type);
    Type lt = value_type(arg(i,0),i.type), rt = value_type(arg(i,1),i.type);
    left = in_register(left,lt,XR_R10);
    if (right.address || (right.kind == Operand::Memory && rt != i.type)) right = in_register(right,rt,XR_R11);
    emit(Op::Compare,i.type,{left,right});
    if (branch) return;
    Operand dest = allocate(i.destination.index,Type::I64);
    Operand result = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
    emit(Op::Set,Type::I1,{result}).condition = condition(i.operation);
    emit(Op::ExtendUnsigned,Type::U8,{result,result});
    move(dest,result,Type::I64);
}
void Selector::conversion(const lowir_model::Instruction& i)
{
    if (i.type.floating() || i.source_type.floating()) {
        convert_to(allocate(i.destination.index,i.type),value(arg(i,0),i.source_type),i.source_type,i.type,
            i.operation == Operation::Uitofp,i.operation == Operation::Fptoui); return;
    }
    require(scalar_integer(i.type) && scalar_integer(i.source_type), "native conversion class not implemented");
    Operand dest = allocate(i.destination.index,i.type);
    Operand result = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
    auto src = value(arg(i,0),i.source_type);
    if (src.address) src = in_register(src,Type::Ptr,XR_R11);
    else if (src.kind == Operand::Memory && value_type(arg(i,0),i.source_type) != i.source_type)
        src = in_register(src,value_type(arg(i,0),i.source_type),XR_R11);
    Type width = i.operation == Operation::Trunc ? i.type : i.source_type;
    bool sign = i.operation == Operation::Sext || (i.operation == Operation::Trunc && !unsigned_type(i.type));
    if (width.width() == 64) move(result,src,width);
    else emit(sign ? Op::ExtendSigned : Op::ExtendUnsigned,width,{result,src});
    move(dest,result,i.type);
}
void Selector::index(const lowir_model::Instruction& i)
{
    if (!state(i.destination.index).uses) return;
    Operand base = value(arg(i,0),Type::Ptr), offset = value(arg(i,1),Type::I64);
    if (arg(i,0).kind == lowir_model::Operand::Slot) base.address = true;
    std::uint64_t scale = i.type.bytes();
    if (offset.kind == Operand::Immediate && base.address) {
        base.displacement += offset.bits*scale;
        if (state(i.destination.index).address_only || base.kind == Operand::Symbol || base.reg == XR_RBP)
            state(i.destination.index).location = base;
        else move(allocate(i.destination.index,Type::Ptr),base,Type::Ptr);
        return;
    }
    base = in_register(base,Type::Ptr,XR_R11);
    Operand address = Operand::mem(base.reg);
    if (offset.kind == Operand::Immediate) address.displacement = offset.bits*scale;
    else {
        offset = in_register(offset,value_type(arg(i,1),Type::I64),XR_R10);
        if (scale != 1 && scale != 2 && scale != 4 && scale != 8) {
            move(Operand::r(XR_R10),offset,Type::I64);
            emit(Op::Mul,Type::I64,{Operand::r(XR_R10),Operand::imm(scale)});
            offset = Operand::r(XR_R10); scale = 1;
        }
        address.index = offset.reg; address.scale = scale;
    }
    bool survives_call = base.reg == XR_RBX || base.reg == XR_RBP || base.reg >= XR_R12;
    if (address.index >= 0) survives_call &= address.index == XR_RBX || address.index >= XR_R12;
    if (state(i.destination.index).folded_index && (!state(i.destination.index).crosses_call || survives_call) &&
        ((base.reg != XR_R10 && base.reg != XR_R11) || state(i.destination.index).last == position+1)) {
        address.address = true; state(i.destination.index).location = address; return;
    }
    auto dest = allocate(i.destination.index,Type::Ptr);
    auto result = dest.kind == Operand::Reg ? dest : Operand::r(XR_R10);
    emit(Op::Lea,Type::Ptr,{result,address}); move(dest,result,Type::Ptr);
}
// Shared predicate mapping for compare-fed branches.
X86Condition branch_condition(Operation op) { return condition(op); }
} // namespace native
