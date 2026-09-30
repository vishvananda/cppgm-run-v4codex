#include "lowering/procedural.h"

namespace cppgm { namespace lowering {
Value Procedural::intrinsic_call(NodeId n, semantic::Intrinsic intrinsic)
{
    auto fact = sem.expression_fact(n);
    auto argument = [&](unsigned i) {
        return converted(sem.call_argument(fact,i),sem.conversion_fact(fact.conversions+i));
    };
    auto first = argument(0);
    if (intrinsic == semantic::Intrinsic::VaStart) return emit(Opcode::VaStart,IRType(),{first.operand});
    if (intrinsic == semantic::Intrinsic::StackAlloc) return emit(Opcode::StackAlloc,IRType::Ptr,{first.operand});
    if (intrinsic == semantic::Intrinsic::VaCopy) {
        auto second = argument(1);
        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(sem.variadic_type());
        copy.alignment = sem.object_alignment(sem.variadic_type()); emit(copy,{second.operand,first.operand});
    }
    if (intrinsic == semantic::Intrinsic::AtomicFetchAdd || intrinsic == semantic::Intrinsic::AtomicAddFetch) {
        auto increment = argument(1);
        argument(2); // Evaluate even a dynamic order exactly once.
        // GNU permits a conservative sequentially-consistent implementation
        // for every supplied order, including runtime values. LowIR atomics
        // operate modulo the declared width, also for signed integer types.
        auto value = emit(Opcode::AtomicAddFetch,increment.ir,{first.operand,increment.operand,Operand::integer(5)});
        if (intrinsic == semantic::Intrinsic::AtomicFetchAdd) {
            Instruction subtract(Opcode::Binary,increment.ir); subtract.operation = Operation::Sub;
            value = emit(subtract,{value.operand,increment.operand});
        }
        value.type = increment.type; return convert(value,fact.type);
    }
    return Value(Operand(),IRType::Void,fact.type);
}
} }
