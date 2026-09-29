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
        call_work.push_back(converted(sem.call_argument(use.call,j), sem.conversion_fact(use.call.conversions+j)).operand);
    Value result = guarded_call(Instruction(Opcode::Call, IRType::Ptr), call_work.data()+begin, call_work.size()-begin);
    std::vector<Operand> release_arguments;
    if (use.deallocation) {
        release_arguments.push_back(result.operand);
        if (use.call.argument_count)
            release_arguments.insert(release_arguments.end(),call_work.begin()+begin+2,call_work.end());
        else if (sem.types[sem.entities[use.deallocation].type].count == 2) release_arguments.push_back(bytes);
    }
    call_work.resize(begin);
    BlockId initialize_block, end;
    if (sem.function_nonthrowing(use.allocation) && (use.initializer || sem.constructor_needed(use.constructor))) {
        initialize_block = block(); end = block();
        Value valid = emit(Opcode::Compare,IRType::Ptr,{result.operand,Operand::integer(0)},Operation::Ne);
        emit(Opcode::Branch,IRType(),{valid.operand,Operand::label(initialize_block),Operand::label(end)}); start(initialize_block);
    }
    Value location(result.operand, type(use.type), use.type, true);
    auto initial = live;
    auto release = use.deallocation ? protect_deallocation(use.deallocation,release_arguments.data(),release_arguments.size()) : 0;
    if (use.construct) {
        auto init = sem.class_initialization(use.initializer,use.type);
        if (init.source) construct_value(init.source,sem.conversion_fact(init.conversion),result,false,false,true);
        else construct(use.constructor,use.initializer,result,false,true);
    }
    else if (use.initializer) {
        auto plan = sem.initializer_plan(use.initializer, use.type);
        if (!plan || !call_aggregate_helper(plan, location)) initialize(use.initializer, use.type, location);
    } else if (use.constructor) construct(use.constructor, 0, result);
    if (release) retire_deallocation(release,initial);
    if (end) { jump(end); start(end); }
    result.type = sem.expression_fact(n).type; return result;
}
} }
