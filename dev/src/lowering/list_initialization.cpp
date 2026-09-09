#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::list_conversion(const semantic::Conversion& c, Value destination)
{
    auto object = sem.list_objects[c.materialization];
    auto plan = sem.list_plans[object.plan];
    auto call = object.call;
    TypeId t = reference(c.target) ? sem.types[c.target].child : c.target;
    bool supplied = destination.ir != IRType::Void;
    if (plan.direct_binding) return converted(sem.call_arguments[call.arguments],sem.conversion_fact(call.conversions));
    Value value;
    if (object.temporary && !supplied) destination = class_address(object.temporary,t);
    if (plan.aggregate) {
        Value at = destination; at.address = true; at.type = t;
        if (!call_aggregate_helper(object.initializer,at)) initialize_plan(object.initializer,at);
    } else if (plan.constructor) {
        if (plan.zero) {
            Instruction zero(Opcode::ZeroInit); zero.bytes = sem.object_size(t); zero.alignment = sem.object_alignment(t);
            emit(zero,{destination.operand});
        }
        if (sem.direct_transfer(plan.constructor)) {
            Value source = converted(sem.call_arguments[call.arguments],sem.conversion_fact(call.conversions));
            if (!sem.empty_class(t)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(t); copy.alignment = sem.object_alignment(t);
                emit(copy,{source.operand,destination.operand});
            }
        } else if (sem.constructor_needed(plan.constructor)) {
            std::size_t begin = call_work.size();
            call_work.push_back(Operand::symbol(symbol(plan.constructor))); call_work.push_back(destination.operand);
            for (unsigned j = 0; j < call.argument_count; ++j)
                call_work.push_back(converted(sem.call_arguments[call.arguments+j],sem.conversion_fact(call.conversions+j)).operand);
            guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
            call_work.resize(begin);
        }
    } else {
        value = call.argument_count ? converted(sem.call_arguments[call.arguments],sem.conversion_fact(call.conversions)) :
            Value(type(t).floating() ? Operand::floating(0) : Operand::integer(0),type(t),t);
        if (supplied || object.temporary) { Value at = destination; at.address = true; at.type = t; store(value,at); }
    }
    if (supplied) return destination;
    if (c.reference) { activate_temporary(object.temporary); return destination; }
    if (object.temporary) {
        if (sem.indirect_parameter(t)) return destination;
        return Value(class_temporary(object.temporary,t).operand,type(t),t);
    }
    return value;
}
} }
