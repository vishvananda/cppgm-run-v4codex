#include "native/selection.h"
namespace native {
using namespace lowir_model;
const char* runtime_name(unsigned entity)
{
    static const char* const names[] = {"@native.exception_top","@native.exception_value"};
    require(entity < unsigned(RuntimeEntity::Count),"invalid native runtime entity");
    return names[entity];
}
void Selector::runtime(const lowir_model::Instruction& i)
{
    switch (i.opcode) {
    case Opcode::EhTry: case Opcode::EhCleanup:
        require(i.operands.count == 1,"host EH clauses require the object runtime");
        require(f.exception_base.kind != Operand::None,"missing handler frame fact");
        emit(Op::EhPush,Type(),{Operand::label(arg(i,0).ref),Operand::imm(i.opcode == Opcode::EhCleanup)});
        break;
    case Opcode::EhEnd: emit(Op::EhPop,Type(),{}); break;
    case Opcode::Throw: {
        auto payload = runtime_operand(p,RuntimeEntity::ExceptionValue);
        auto from = value(arg(i,0),i.type);
        Type actual = consumed_type(arg(i,0),i.type);
        if (actual.floating() || i.type.floating()) convert_to(payload,from,actual,i.type);
        else {
            if (scalar_integer(i.type) && from.kind == Operand::Memory && actual != i.type)
                from = in_register(from,actual,XR_R10);
            move(payload,from,i.type);
        }
        emit(Op::Throw,i.type,{}); break;
    }
    case Opcode::Resume: emit(Op::Resume,Type(),{}); break;
    case Opcode::Exception:
        move(allocate(i.destination.index,i.type),runtime_operand(p,RuntimeEntity::ExceptionValue),i.type);
        break;
    case Opcode::StackAlloc: {
        auto size = value(arg(i,0),Type::I64);
        move(Operand::r(XR_R10),size,consumed_type(arg(i,0),Type::I64));
        auto dest = allocate(i.destination.index,Type::Ptr);
        auto reg = dest.kind == Operand::Reg ? dest : Operand::r(XR_RAX);
        emit(Op::StackAlloc,Type::Ptr,{reg,Operand::r(XR_R10)});
        move(dest,reg,Type::Ptr); break;
    }
    default: throw ParseError("invalid generic runtime operation");
    }
}
} // namespace native
