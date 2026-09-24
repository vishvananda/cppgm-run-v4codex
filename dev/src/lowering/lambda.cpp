#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::closure_adapter(EntityId e)
{
    auto closure = sem.closure_adapter(e);
    function = FunctionId(p.symbols[symbol(e).index-1].entity);
    builder.reset(new lowir_model::FunctionBuilder(p,function));
    reset_lifetime(e); start(block());
    if (e == closure.conversion) {
        emit(Opcode::Return,IRType::Ptr,{Operand::symbol(symbol(closure.thunk))});
    } else {
        auto sig = p.signatures[p.functions[function.index-1].signature.index-1];
        auto result = sem.types[sem.entities[e].type].child;
        std::vector<Operand> arguments(1,Operand::symbol(symbol(closure.function)));
        unsigned first = 0;
        if (sem.indirect_value(result)) arguments.push_back(Operand::value(p.parameters[sig.parameters.begin+first++].value));
        // The captureless call operator cannot observe the closure object's
        // state. Supply real addressable empty storage for its implicit object.
        auto slot = builder->add_slot(0,type(sem.entities[closure.entity].type));
        auto object = emit(Opcode::Addr,IRType(),{Operand::slot(slot)});
        arguments.push_back(object.operand);
        for (unsigned i = first; i < sig.parameters.count; ++i)
            arguments.push_back(Operand::value(p.parameters[sig.parameters.begin+i].value));
        auto value = emit(Instruction(Opcode::Call,sig.result),arguments.data(),arguments.size());
        if (sig.result == IRType::Void) emit(Opcode::Return,IRType(),{});
        else emit(Opcode::Return,sig.result,{value.operand});
    }
    builder.reset();
}
} }
