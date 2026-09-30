#include "native/selection.h"
namespace native {
using namespace lowir_model;
namespace {
bool exact_scale(Operand operand, Type type)
{
    if (operand.kind != Operand::Floating || (type != Type::F32 && type != Type::F64)) return false;
    unsigned fraction = type == Type::F32 ? 23 : 52;
    auto magnitude = operand.bits & (type == Type::F32 ? 0x7fffffffULL : 0x7fffffffffffffffULL);
    auto exponent = magnitude >> fraction;
    if (!exponent) return magnitude && !(magnitude & (magnitude-1));
    auto maximum = type == Type::F32 ? 255 : 2047;
    return exponent != unsigned(maximum) && !(magnitude & ((std::uint64_t(1)<<fraction)-1));
}
}

void Selector::floating_arithmetic(const lowir_model::Instruction& i)
{
    f.scratch_bytes = 48;
    auto lhs = value(arg(i,0),i.type);
    if (i.opcode == Opcode::Unary && i.operation == Operation::Not) {
        auto zero = Operand::floating(lowir_model::Operand::integer(0),i.type);
        if (state(i.destination.index).compare_branch) emit(Op::Fcompare,i.type,{lhs,zero});
        else {
            emit(Op::Fset,i.type,{Operand::r(XR_R10),lhs,zero}).condition = XC_E;
            convert_to(allocate(i.destination.index,i.type),Operand::r(XR_R10),Type::I64,i.type);
        }
        return;
    }
    auto dest = allocate(i.destination.index,i.type);
    if (i.opcode == Opcode::Unary) {
        require(i.operation == Operation::Neg,"unsupported floating unary operation");
        emit(Op::Fneg,i.type,{dest,lhs}); return;
    }
    auto op = i.operation == Operation::Add ? Op::Fadd : i.operation == Operation::Sub ? Op::Fsub :
        i.operation == Operation::Mul ? Op::Fmul : Op::Fdiv;
    require(i.operation == Operation::Add || i.operation == Operation::Sub ||
        i.operation == Operation::Mul || i.operation == Operation::Div,"unsupported floating binary operation");
    auto rhs = value(arg(i,1),i.type);
    auto precision = i.source_type;
    // Scaling a binary32/64 value by a finite nonzero power of two is exact
    // in binary80, including all possible product exponents. Only the final
    // binary32/64 rounding remains. SSE implements that same rounding, zero,
    // infinity and NaN behavior. Unknown scales retain extended evaluation.
    // Two constant-size bit tests, no search/allocation/growth; one selected
    // instruction carries the proof and uses the shorter ordinary SSE path.
    if (precision == Type::F80 && op == Op::Fmul && (exact_scale(lhs,i.type) || exact_scale(rhs,i.type)))
        precision = i.type;
    emit(op,i.type,{dest,lhs,rhs}).source_type = precision;
}
void Selector::floating_compare(const lowir_model::Instruction& i, bool branch)
{
    f.scratch_bytes = 48;
    auto lhs = value(arg(i,0),i.type), rhs = value(arg(i,1),i.type);
    if (branch) { emit(Op::Fcompare,i.type,{lhs,rhs}); return; }
    auto dst = allocate(i.destination.index,Type::I64);
    auto& selected = emit(Op::Fset,i.type,{dst,lhs,rhs});
    selected.condition = i.operation == Operation::Eq ? XC_E : i.operation == Operation::Ne ? XC_NE :
        (i.operation == Operation::Lt || i.operation == Operation::Ult) ? XC_B :
        (i.operation == Operation::Le || i.operation == Operation::Ule) ? XC_BE :
        (i.operation == Operation::Gt || i.operation == Operation::Ugt) ? XC_A : XC_AE;
}
void Selector::convert_to(Operand to, Operand from, Type source_type, Type target, bool ui, bool uo)
{
    if (source_type == target) { move(to,from,target); return; }
    require((source_type.floating() || source_type.integer() || source_type == Type::Ptr) &&
        (target.floating() || target.integer() || target == Type::Ptr),"unsupported native conversion");
    f.scratch_bytes = 48;
    if (source_type == Type::I128 && from.kind != Operand::Memory) {
        auto storage = home(0,source_type,true); move(storage,from,source_type); from = storage;
    }
    auto op = source_type.floating() ? target.floating() ?
        (target.width() > source_type.width() ? Op::Fpext : Op::Fptrunc) :
        (uo || unsigned_type(target) ? Op::Fptoui : Op::Fptosi) :
        (ui || unsigned_type(source_type) ? Op::Uitofp : Op::Sitofp);
    emit(op,target,{to,from}).source_type = source_type;
}
Operand Selector::convert_value(Operand from, Type source_type, Type target, bool ui, bool uo)
{
    if (source_type == target) return from;
    Operand result = target == Type::F80 ? home(0,target,true) : Operand::r(target.floating() ? xmm(14) : XR_R10);
    convert_to(result,from,source_type,target,ui,uo); return result;
}
} // namespace native
