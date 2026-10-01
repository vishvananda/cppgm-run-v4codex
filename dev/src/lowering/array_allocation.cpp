#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
void Procedural::heap_array_destroy(EntityId destructor, TypeId leaf, Value data, Operand count)
{
    if (!destructor || (sem.synthetic_member(destructor) && !sem.destructor_needed(destructor))) return;
    SlotId index = builder->add_slot(0,IRType::I64);
    emit(Opcode::Store,IRType::I64,{count,Operand::slot(index)});
    auto initial = live;
    semantic::Index retired;
    if (!emitting_cleanup && !sem.function_nonthrowing(destructor))
        activate_subobject(leaf,data,index,retired);
    BlockId cond = block(), body = block(), end = block(); jump(cond); start(cond);
    Value current = emit(Opcode::Load,IRType::I64,{Operand::slot(index)});
    Value test = emit(Opcode::Compare,IRType::I64,{current.operand,Operand::integer(0)},Operation::Ne);
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(body),Operand::label(end)}); start(body);
    Value next = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(1)},Operation::Sub);
    emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(index)});
    Operand offset = emit(Opcode::Binary,IRType::I64,
        {next.operand,Operand::integer(sem.object_size(leaf))},Operation::Mul).operand;
    Value at = emit(Opcode::Index,IRType::I8,{data.operand,offset});
    Operand args[] = {Operand::symbol(symbol(destructor)),at.operand};
    guarded_call(Instruction(Opcode::Call,IRType::Void),args,2);
    if (!retired.empty()) close_expression_region();
    jump(cond); start(end); live = initial;
}
Value Procedural::array_new(NodeId n, const semantic::PlacementNew& use)
{
    // Keep the extent across the allocator call; it is evaluated only once.
    bool dynamic = use.dynamic_extent;
    Operand bytes;
    SlotId extent;
    if (dynamic) {
        Value count = incoming(use.bound);
        if (!use.narrow_extent) count = coerce(count,IRType::I64,sem.unsigned_type(sem.expression_fact(use.bound).type),true);
        if (use.stride != 1) count = emit(Opcode::Binary,count.ir,{count.operand,Operand::integer(use.stride)},Operation::Mul);
        if (use.cookie) count = emit(Opcode::Binary,count.ir,{count.operand,Operand::integer(use.cookie)},Operation::Add);
        count = coerce(count,IRType::I64,sem.unsigned_type(sem.expression_fact(use.bound).type),true);
        bytes = count.operand;
        extent = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{bytes,Operand::slot(extent)});
    } else {
        auto size = use.fixed_count*use.stride+use.cookie;
        bytes = Operand::integer(size);
        if (size <= 2147483647) {
            Instruction widen(Opcode::Convert,IRType::I64); widen.source_type = IRType::I32; widen.operation = Operation::Sext;
            bytes = emit(widen,{bytes}).operand;
        }
    }
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(use.allocation))); call_work.push_back(bytes);
    for (unsigned j = 0; j < use.call.argument_count; ++j)
        call_work.push_back(converted(sem.call_argument(use.call,j),sem.conversion_fact(use.call.conversions+j)).operand);
    Value allocation = guarded_call(Instruction(Opcode::Call,IRType::Ptr),call_work.data()+begin,call_work.size()-begin);
    std::vector<Operand> placement_release;
    if (use.deallocation && use.call.argument_count) {
        placement_release.push_back(Operand::symbol(symbol(use.deallocation)));
        placement_release.push_back(allocation.operand);
        auto signature = sem.types[sem.entities[use.deallocation].type];
        for (unsigned j = 0; j < use.call.argument_count; ++j) {
            auto t = type(sem.types.parameters[signature.offset+j+1]);
            auto slot = builder->add_slot(0,t);
            emit(Opcode::Store,t,{call_work[begin+j+2],Operand::slot(slot)});
            placement_release.push_back(Operand::slot(slot));
        }
    }
    call_work.resize(begin);
    BlockId nullable_end; SlotId nullable_result;
    if (sem.function_nonthrowing(use.allocation) && (use.cookie || use.zero || use.construct)) {
        nullable_result = builder->add_slot(0,IRType::Ptr);
        emit(Opcode::Store,IRType::Ptr,{Operand::integer(0),Operand::slot(nullable_result)});
        BlockId run = block(); nullable_end = block();
        Value valid = emit(Opcode::Compare,IRType::Ptr,{allocation.operand,Operand::integer(0)},Operation::Ne);
        emit(Opcode::Branch,IRType(),{valid.operand,Operand::label(run),Operand::label(nullable_end)}); start(run);
    }
    if (extent) bytes = emit(Opcode::Load,IRType::I64,{Operand::slot(extent)}).operand;
    Value data = allocation;
    auto count = [&]() {
        auto leaf_stride = sem.object_size(use.leaf);
        if (!dynamic) return emit(Opcode::Const,IRType::I64,{Operand::integer(use.fixed_count*(use.stride/leaf_stride))}).operand;
        Operand elements = bytes;
        if (use.cookie) elements = emit(Opcode::Binary,IRType::I64,{elements,Operand::integer(use.cookie)},Operation::Sub).operand;
        if (leaf_stride != 1) elements = emit(Opcode::Binary,IRType::I64,{elements,Operand::integer(leaf_stride)},Operation::Udiv).operand;
        return elements;
    };
    if (use.cookie) {
        data = emit(Opcode::Index,IRType::I8,{allocation.operand,Operand::integer(use.cookie)});
        emit(Opcode::Store,IRType::I64,{count(),allocation.operand});
    }
    if (use.zero) {
        if (!use.zero_plan) throw std::logic_error("missing heap zero-initialization plan");
        auto plan = sem.zero_initializations[use.zero_plan];
        auto stride = plan.bulk ? 1 : plan.bytes;
        Operand length = bytes;
        if (use.cookie) length = emit(Opcode::Binary,IRType::I64,{length,Operand::integer(use.cookie)},Operation::Sub).operand;
        SlotId offset = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(offset)});
        BlockId cond = block(), body = block(), end = block(); jump(cond); start(cond);
        Value current = emit(Opcode::Load,IRType::I64,{Operand::slot(offset)});
        Value test = emit(Opcode::Compare,IRType::I64,{current.operand,length},Operation::Ult);
        emit(Opcode::Branch,IRType(),{test.operand,Operand::label(body),Operand::label(end)}); start(body);
        Value at = emit(Opcode::Index,IRType::I8,{data.operand,current.operand});
        if (plan.bulk) emit(Opcode::Store,IRType::I8,{Operand::integer(0),at.operand});
        else zero_plan(use.zero_plan,at,!sem.class_value(use.leaf));
        Value next = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(stride)},Operation::Add);
        emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(offset)}); jump(cond); start(end);
    }
    if (use.construct) {
        Operand elements = count();
        auto stride = sem.object_size(use.leaf);
        SlotId index = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(index)});
        auto initial = live;
        semantic::Index retired;
        // A raw loop owns the whole unwind state only with an empty incoming
        // prefix and no default-argument recipe. Otherwise retain typed owners
        // so nested expression cleanups can reach the actual source handler.
        bool contextual = exception_context && (live || sem.types[sem.entities[use.constructor].type].count);
        if (contextual) {
            if (use.deallocation) {
                std::vector<Operand> args{allocation.operand};
                auto signature = sem.types[sem.entities[use.deallocation].type];
                if (!placement_release.empty()) {
                    for (unsigned j = 0; j < use.call.argument_count; ++j)
                        args.push_back(emit(Opcode::Load,type(sem.types.parameters[signature.offset+j+1]),{placement_release[j+2]}).operand);
                } else if (signature.count == 2) args.push_back(bytes);
                retired.put(protect_deallocation(use.deallocation,args.data(),args.size()),1);
            }
            activate_subobject(use.leaf,data,index,retired);
        }
        auto prefix = live;
        BlockId cond = block(), body = block(), end = block();
        BlockId cleanup = contextual ? BlockId() : block(), continuation = contextual ? BlockId() : block();
        jump(cond); start(cond);
        Value current = emit(Opcode::Load,IRType::I64,{Operand::slot(index)});
        Value test = emit(Opcode::Compare,IRType::I64,{current.operand,elements},Operation::Ult);
        emit(Opcode::Branch,IRType(),{test.operand,Operand::label(body),Operand::label(end)}); start(body);
        Operand offset = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(stride)},Operation::Mul).operand;
        Value at = emit(Opcode::Index,IRType::I8,{data.operand,offset});
        if (!contextual) emit(Opcode::EhTry,IRType(),{Operand::label(cleanup)});
        auto saved_live = live;
        if (!contextual) live = 0;
        construct(use.constructor,0,at);
        bool temporaries = live != (contextual ? prefix : 0);
        auto advance = [&]() {
            Value next = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(1)},Operation::Add);
            emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(index)});
        };
        if (temporaries) advance();
        clean_inline(live,contextual ? prefix : 0); live = saved_live;
        if (contextual) close_expression_region();
        else emit(Opcode::EhEnd,IRType(),{});
        if (!temporaries) advance();
        jump(cond);
        start(end);
        if (contextual) {
            close_expression_region(); semantic::Index cache;
            live = retire_construction(live,initial,retired,cache);
        } else {
            jump(continuation); start(cleanup);
            exception_clauses(exception_context,true);
            current = emit(Opcode::Load,IRType::I64,{Operand::slot(index)});
            bool saved_cleanup = emitting_cleanup; emitting_cleanup = true;
            heap_array_destroy(use.destructor,use.leaf,data,current.operand);
            if (!placement_release.empty())
                guarded_call(Instruction(Opcode::Call,IRType::Void),placement_release.data(),placement_release.size());
            else if (use.deallocation) {
                Operand args[] = {Operand::symbol(symbol(use.deallocation)),allocation.operand,bytes};
                guarded_call(Instruction(Opcode::Call,IRType::Void),args,sem.types[sem.entities[use.deallocation].type].count+1);
            }
            emitting_cleanup = saved_cleanup;
            if (exception_context) {
                emit(Opcode::EhEnd,IRType(),{});
                resume_exception(live,exception_context,false);
            } else emit(Opcode::Resume,IRType(),{});
            start(continuation);
        }
    }
    if (nullable_end) {
        emit(Opcode::Store,IRType::Ptr,{data.operand,Operand::slot(nullable_result)}); jump(nullable_end); start(nullable_end);
        data = emit(Opcode::Load,IRType::Ptr,{Operand::slot(nullable_result)});
    }
    data.type = sem.expression_fact(n).type; return data;
}
} }
