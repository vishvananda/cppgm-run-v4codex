#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::arrow_object(NodeId n, std::uint32_t id)
{
    if (!id) return load(expression(n));
    auto object = address(expression(n,true));
    auto chain = sem.arrow_chains[id];
    for (unsigned j = 0; j < chain.count; ++j) {
        auto step = sem.arrow_steps[chain.first+j];
        object = base_projection(object,step.adjustment);
        bool aggregate = sem.class_value(step.result), indirect = sem.indirect_value(step.result);
        Value destination;
        if (aggregate) destination = class_address(step.temporary,step.result);
        Instruction call(Opcode::Call,indirect ? IRType(IRType::Void) : type(step.result));
        auto fn = Operand::symbol(symbol(step.function));
        if (step.virtual_slot) { fn = virtual_function(object,step.virtual_slot).operand; call.signature = virtual_signature(step.function); }
        std::vector<Operand> arguments{fn};
        if (indirect) arguments.push_back(destination.operand);
        arguments.push_back(object.operand);
        object = guarded_call(call,arguments.data(),arguments.size()); object.type = step.result;
        if (aggregate) {
            if (!indirect && !sem.empty_class(step.result)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(step.result); copy.alignment = sem.object_alignment(step.result);
                emit(copy,{object.operand,destination.operand});
            }
            activate_temporary(step.temporary); object = destination;
        }
    }
    return object;
}
} }
