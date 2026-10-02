#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::x86_call(NodeId n)
{
    auto fact = sem.expression_fact(n);
    auto id = sem.x86_intrinsic(sem.facts[n].entity);
    auto d = x86_builtin(id);
    if (d.form >= X86Form::Shuffle) return x86_lane_call(n,d);
    auto storage = Operand::slot(builder->add_slot(0,IRType::object(64,16)));
    auto pointer = address(Value(storage,IRType::object(64,16),0,true));
    std::uint64_t predicate = ~std::uint64_t(0);
    for (unsigned j=0; j<fact.argument_count; ++j) {
        auto value = converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j));
        if (d.form == X86Form::DynamicCompare && j == 2 && value.operand.kind == Operand::Integer)
            predicate = value.operand.data.integer & 31;
        auto at = emit(Opcode::Index,IRType::I8,{pointer.operand,Operand::integer(16*(j+1))});
        at.type = value.type; at.ir = value.ir; at.address = true; store(value,at);
    }
    emit(Opcode::X86,IRType(),{Operand::integer(id),pointer.operand,Operand::integer(predicate)});
    if (type(fact.type) == IRType::Void) return Value(Operand(),IRType::Void,fact.type);
    // Every result byte, including preserved/zeroed SIMD upper lanes, is
    // established by the target operation. No semantic result is invented.
    return load(Value(pointer.operand,type(fact.type),fact.type,true));
}
} }
