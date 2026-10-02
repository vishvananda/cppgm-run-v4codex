#include "lowering/procedural.h"
#include <stdexcept>

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
    if (intrinsic == semantic::Intrinsic::VectorInit) {
        Value result(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
        // The fixed vocabulary has at most eight lanes. Each operand is
        // evaluated once and every byte of the result is initialized.
        for (unsigned i = 0; i < fact.argument_count; ++i)
            vector_write(result,Value(Operand::integer(i),IRType::I64,sem.types.fundamental(FT_UNSIGNED_LONG_INT)),argument(i));
        result.address = false; return result;
    }
    if (intrinsic == semantic::Intrinsic::VectorExtract) {
        auto vector = argument(0), lane = argument(1);
        return vector_read(vector,coerce(lane,IRType::I64));
    }
    if (intrinsic == semantic::Intrinsic::Complex) {
        auto real = argument(0); auto imag = argument(1);
        return complex_construct(fact.type,real,imag);
    }
    if (intrinsic == semantic::Intrinsic::FltRounds) {
        auto target = symbol(sem.facts[n].entity);
        auto mode = emit(Opcode::Call,IRType::I32,{Operand::symbol(target)});
        // Linux x86-64 fenv encodes nearest/down/up/zero as 0/1/2/3 in bits 10–11.
        auto index = emit(Opcode::Binary,IRType::I32,{mode.operand,Operand::integer(9)},Operation::Ushr);
        auto mapped = emit(Opcode::Binary,IRType::I32,{Operand::integer(45),index.operand},Operation::Ushr);
        auto result = emit(Opcode::Binary,IRType::I32,{mapped.operand,Operand::integer(3)},Operation::And);
        result.type = fact.type; return result;
    }
    if (intrinsic == semantic::Intrinsic::IsConstantEvaluated)
        return Value(Operand::integer(0),IRType::I8,fact.type);
    if (intrinsic >= semantic::Intrinsic::SourceFile && intrinsic <= semantic::Intrinsic::SourceColumn) {
        if (!source_invocation.site) throw std::logic_error("missing lowered source invocation fact");
        auto site = sem.source_sites[source_invocation.site];
        if (intrinsic == semantic::Intrinsic::SourceLine || intrinsic == semantic::Intrinsic::SourceColumn)
            return Value(Operand::integer(intrinsic == semantic::Intrinsic::SourceLine ? site.line : 0),IRType::I32,fact.type);
        auto symbol = source_string_symbol(intrinsic == semantic::Intrinsic::SourceFile ? site.file : site.function);
        return address(Value(Operand::symbol(symbol),IRType::Ptr,fact.type,true));
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
