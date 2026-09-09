#include "lowering/procedural.h"

namespace cppgm { namespace lowering {
Value Procedural::placement_new(NodeId n)
{
    auto use = sem.placement_fact(n);
    if (use.array) return array_new(n,use);
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(use.allocation)));
    auto size = sem.object_size(use.type);
    Operand bytes = Operand::integer(size);
    if (size <= 2147483647) {
        Instruction widen(Opcode::Convert, IRType::I64); widen.source_type = IRType::I32; widen.operation = Operation::Sext;
        bytes = emit(widen, {bytes}).operand;
    }
    call_work.push_back(bytes);
    for (unsigned j = 0; j < use.call.argument_count; ++j)
        call_work.push_back(converted(sem.call_arguments[use.call.arguments+j], sem.conversion_fact(use.call.conversions+j)).operand);
    Value result = guarded_call(Instruction(Opcode::Call, IRType::Ptr), call_work.data()+begin, call_work.size()-begin);
    call_work.resize(begin);
    BlockId initialize_block, end;
    if (sem.function_nonthrowing(use.allocation) && (use.initializer || sem.constructor_needed(use.constructor))) {
        initialize_block = block(); end = block();
        Value valid = emit(Opcode::Compare,IRType::Ptr,{result.operand,Operand::integer(0)},Operation::Ne);
        emit(Opcode::Branch,IRType(),{valid.operand,Operand::label(initialize_block),Operand::label(end)}); start(initialize_block);
    }
    Value location(result.operand, type(use.type), use.type, true);
    if (use.initializer) {
        auto plan = sem.initializer_plan(use.initializer, use.type);
        if (!plan || !call_aggregate_helper(plan, location)) initialize(use.initializer, use.type, location);
    } else if (use.constructor) construct(use.constructor, 0, result);
    if (end) { jump(end); start(end); }
    result.type = sem.expression_fact(n).type; return result;
}
} }
