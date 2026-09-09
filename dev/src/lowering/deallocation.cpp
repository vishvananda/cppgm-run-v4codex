#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
void Procedural::emit_allocation_adapters()
{
    // LowIR has singleton allocation/free roles. Distinct unreplaced C++
    // functions keep distinct addresses and delegate to that shared runtime.
    for (auto adapter : allocation_adapters) {
        function = FunctionId(p.symbols[adapter.symbol.index-1].entity);
        auto& f = p.functions[function.index-1]; f.declaration = false;
        auto sig = p.signatures[f.signature.index-1];
        builder.reset(new lowir_model::FunctionBuilder(p,function)); reset_lifetime(0); start(block());
        Value value = emit(Opcode::Call,sig.result,{Operand::symbol(adapter.runtime),Operand::value(p.parameters[sig.parameters.begin].value)});
        if (sig.result == IRType::Void) emit(Opcode::Return,IRType(),{});
        else emit(Opcode::Return,sig.result,{value.operand});
        builder.reset();
    }
}
Value Procedural::delete_expression(NodeId n)
{
    auto use = sem.delete_fact(n);
    Value pointer = converted(use.operand,use.conversion);
    BlockId end;
    if (sem.class_value(use.leaf)) {
        Value nonnull = emit(Opcode::Compare,IRType::Ptr,{pointer.operand,Operand::integer(0)},Operation::Ne);
        BlockId run = block(); end = block();
        emit(Opcode::Branch,IRType(),{nonnull.operand,Operand::label(run),Operand::label(end)}); start(run);
    }
    Value allocation = pointer;
    Operand bytes = Operand::integer(sem.object_size(use.type));
    if (use.cookie) {
        Operand back = Operand::integer(-use.cookie); back.negative_integer = true;
        allocation = emit(Opcode::Index,IRType::I8,{pointer.operand,back});
        Value count = emit(Opcode::Load,IRType::I64,{allocation.operand});
        Operand elements = count.operand;
        heap_array_destroy(use.destructor,use.leaf,pointer,elements);
        if (use.sized) {
            bytes = emit(Opcode::Binary,IRType::I64,{count.operand,Operand::integer(sem.object_size(use.leaf))},Operation::Mul).operand;
            bytes = emit(Opcode::Binary,IRType::I64,{bytes,Operand::integer(use.cookie)},Operation::Add).operand;
        }
    } else if (!use.array && use.destructor && (!sem.synthetic_member(use.destructor) || sem.destructor_needed(use.destructor))) {
        Operand args[] = {Operand::symbol(symbol(use.destructor)),pointer.operand};
        guarded_call(Instruction(Opcode::Call,IRType::Void),args,2);
    }
    Operand args[] = {Operand::symbol(symbol(use.deallocation)),allocation.operand,bytes};
    guarded_call(Instruction(Opcode::Call,IRType::Void),args,use.sized ? 3 : 2);
    if (end) { jump(end); start(end); }
    return Value(Operand(),IRType::Void,sem.expression_fact(n).type);
}
} }
