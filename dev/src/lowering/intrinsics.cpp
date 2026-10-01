#include "lowering/procedural.h"

namespace cppgm { namespace lowering {
Value Procedural::intrinsic_call(NodeId n, semantic::Intrinsic intrinsic)
{
    if (intrinsic >= semantic::Intrinsic::Clz && intrinsic <= semantic::Intrinsic::Popcountg)
        return integer_builtin(n,intrinsic);
    if (intrinsic >= semantic::Intrinsic::AddOverflow && intrinsic <= semantic::Intrinsic::MulOverflow)
        return overflow_builtin(n,intrinsic);
    if (intrinsic == semantic::Intrinsic::Atomic) return atomic_call(n);
    auto fact = sem.expression_fact(n);
    auto argument = [&](unsigned i) {
        return converted(sem.call_argument(fact,i),sem.conversion_fact(fact.conversions+i));
    };
    if (intrinsic == semantic::Intrinsic::FltRounds) {
        auto target = symbol(sem.facts[n].entity);
        auto mode = emit(Opcode::Call,IRType::I32,{Operand::symbol(target)});
        // Linux x86-64 fenv encodes nearest/down/up/zero as 0/1/2/3 in bits 10–11.
        auto index = emit(Opcode::Binary,IRType::I32,{mode.operand,Operand::integer(9)},Operation::Ushr);
        auto mapped = emit(Opcode::Binary,IRType::I32,{Operand::integer(45),index.operand},Operation::Ushr);
        auto result = emit(Opcode::Binary,IRType::I32,{mapped.operand,Operand::integer(3)},Operation::And);
        result.type = fact.type; return result;
    }
    auto first = argument(0);
    if (intrinsic == semantic::Intrinsic::Prefetch || intrinsic == semantic::Intrinsic::AssumeAligned) {
        for (unsigned j = 1; j < fact.argument_count; ++j) argument(j);
        // Hints add no promise to the IR until an optimizer consumes a proven fact.
        first.type = fact.type;
        return intrinsic == semantic::Intrinsic::AssumeAligned ? first : Value(Operand(),IRType::Void,fact.type);
    }
    if (intrinsic == semantic::Intrinsic::VaStart) return emit(Opcode::VaStart,IRType(),{first.operand});
    if (intrinsic == semantic::Intrinsic::StackAlloc) return emit(Opcode::StackAlloc,IRType::Ptr,{first.operand});
    if (intrinsic == semantic::Intrinsic::VaCopy) {
        auto second = argument(1);
        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(sem.variadic_type());
        copy.alignment = sem.object_alignment(sem.variadic_type()); emit(copy,{second.operand,first.operand});
    }
    return Value(Operand(),IRType::Void,fact.type);
}
} }
