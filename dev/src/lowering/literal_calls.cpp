#include "lowering/procedural.h"
#include <cstring>
namespace cppgm { namespace lowering {
void Procedural::literal_arguments(NodeId n)
{
    auto kind = sem.literal_call_kind(n);
    auto lit = ast.literals[ast[n].literal];
    if (kind == semantic::LiteralCallKind::Pack) return;
    if (kind == semantic::LiteralCallKind::Scalar) {
        if (lit.kind == LiteralKind::floating) {
            long double value = 0; std::memcpy(&value,lit.scalar.data(),10);
            auto source = sem.types.fundamental(lit.type);
            call_work.push_back(convert(Value(Operand::floating(value),type(source),source),sem.conversion_fact(sem.expression_fact(n).conversions).target).operand);
        } else {
            std::uint64_t value = 0; std::memcpy(&value,lit.scalar.data(),fundamental_width(lit.type));
            auto source = sem.types.fundamental(lit.type);
            call_work.push_back(convert(Value(Operand::integer(value),type(source),source),sem.conversion_fact(sem.expression_fact(n).conversions).target).operand);
        }
        return;
    }
    Value string = emit(Opcode::Addr,IRType(),{Operand::symbol(strings[n])});
    call_work.push_back(string.operand);
    if (kind == semantic::LiteralCallKind::Raw) return;
    Instruction widen(Opcode::Convert,IRType::I64); widen.source_type = IRType::I32; widen.operation = Operation::Sext;
    Value length = emit(widen,{Operand::integer(lit.elements-1)});
    call_work.push_back(length.operand);
}
} }
