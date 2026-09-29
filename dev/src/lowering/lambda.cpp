#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::captured_address(unsigned id)
{
    auto capture = sem.closure_captures[id];
    auto base = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
    auto value = field(base,capture.field);
    return capture.object ? address(value) : load(value);
}
void Procedural::initialize_closure(NodeId n, Value destination)
{
    auto closure = sem.closure(sem.types[sem.expression_fact(n).type].entity);
    for (auto id = closure.first_capture; id; id = sem.closure_captures[id].next) {
        auto capture = sem.closure_captures[id];
        auto offset = sem.entities[capture.field].member_offset;
        auto target = destination;
        if (offset) {
            Instruction projection(Opcode::Index,IRType::I8); projection.projection = ir_model::IPK_FIELD;
            target = emit(projection,{destination.operand,Operand::integer(offset)});
        }
        auto source = capture.source ? captured_address(capture.source) : capture.object ?
            address(binding(capture.object)) : emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
        emit(Opcode::Store,IRType::Ptr,{source.operand,target.operand});
    }
}
void Procedural::closure_adapter(EntityId e)
{
    auto closure = sem.closure_adapter(e);
    function = FunctionId(p.symbols[symbol(e).index-1].entity);
    builder.reset(new lowir_model::FunctionBuilder(p,function));
    reset_lifetime(e); start(block());
    emit(Opcode::Return,IRType::Ptr,{Operand::symbol(symbol(closure.thunk))});
    builder.reset();
}
} }
