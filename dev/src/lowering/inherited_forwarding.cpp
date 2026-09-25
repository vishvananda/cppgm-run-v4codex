#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::inherited_call(EntityId e, Value object)
{
    auto member = sem.member_fact(e);
    auto target = member.inherited_constructor;
    auto signature = sem.types[sem.entities[target].type];
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(target,true)));
    call_work.push_back(object.operand);
    for (unsigned j = 0; j < signature.count; ++j) {
        auto argument = sem.inherited_arguments[member.inherited_arguments+j];
        auto c = sem.conversion_fact(argument.conversion);
        if (argument.value) {
            call_work.push_back(converted(argument.value,c).operand); continue;
        }
        auto parameter = sem.entities[argument.parameter].type;
        auto source = binding(argument.parameter);
        if (sem.class_value(c.target)) {
            auto slot = builder->add_slot(0,type(c.target));
            Value destination(Operand::slot(slot),type(c.target),c.target,true);
            auto pointer = address(destination);
            if (argument.transfer && !sem.trivial_transfer(argument.transfer) && !sem.direct_transfer(argument.transfer)) {
                auto start = call_work.size();
                call_work.push_back(Operand::symbol(symbol(argument.transfer)));
                call_work.push_back(pointer.operand); call_work.push_back(address(source).operand);
                auto transfer = sem.types[sem.entities[argument.transfer].type];
                for (unsigned k = 1; k < transfer.count; ++k)
                    call_work.push_back(converted(sem.default_argument_value(argument.transfer,k),
                        sem.conversion_fact(argument.transfer_defaults+k-1)).operand);
                guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+start,call_work.size()-start);
                call_work.resize(start);
            } else if (!sem.empty_class(c.target)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(c.target); copy.alignment = sem.object_alignment(c.target);
                emit(copy,{address(source).operand,pointer.operand});
            }
            call_work.push_back(sem.indirect_parameter(c.target) ? pointer.operand : destination.operand);
        } else if (reference(parameter)) call_work.push_back(address(source).operand);
        else call_work.push_back(converted_value(source,c).operand);
    }
    guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
    call_work.resize(begin);
}
} }
