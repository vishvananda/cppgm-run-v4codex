#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::list_conversion(const semantic::Conversion& c, Value destination)
{
    auto object = sem.list_objects[c.materialization];
    auto plan = sem.list_plans[object.plan];
    auto call = object.call;
    TypeId t = reference(c.target) ? sem.types[c.target].child : c.target;
    bool supplied = destination.ir != IRType::Void;
    if (plan.direct_binding) return converted(sem.call_argument(call),sem.conversion_fact(call.conversions));
    Value value;
    if (object.temporary && !supplied) destination = class_address(object.temporary,t);
    if (plan.backing_element) {
        Value backing(Operand::integer(0),IRType::Ptr);
        if (object.backing) {
            auto array = sem.entities[object.backing].type;
            backing = class_address(object.backing,array);
            auto initial = live;
            semantic::Index retired;
            bool cleanup = sem.temporary_cleanup(object.backing);
            for (unsigned j = 0; j < call.argument_count; ++j) {
                auto conversion = sem.conversion_fact(call.conversions+j);
                Value scalar;
                if (!sem.class_value(plan.backing_element)) scalar = converted(sem.call_argument(call,j),conversion);
                Instruction projection(Opcode::Index,IRType::I8);
                projection.projection = sem.class_value(plan.backing_element) ? ir_model::IPK_ARRAY_ELEMENT : ir_model::IPK_NONE;
                auto offset = j*sem.object_size(plan.backing_element);
                Value at = offset ? emit(projection,{backing.operand,Operand::integer(offset)}) : backing;
                at.type = plan.backing_element; at.address = true;
                if (sem.class_value(plan.backing_element))
                    construct_value(sem.call_argument(call,j),conversion,address(at));
                else store(scalar,at);
                if (cleanup) {
                    close_expression_region();
                    TemporaryState state; state.object = sem.types[plan.backing_element].entity;
                    state.destructor = sem.object_destructor(object.backing);
                    state.location = lowir_model::ValueId(at.operand.ref);
                    state.tail = live; state.depth = lifetime_state(live).depth+1;
                    temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
                    retired.put(live,1);
                }
            }
            if (cleanup) {
                close_expression_region();
                semantic::Index cache; live = retire_construction(live,initial,retired,cache);
            }
            activate_temporary(object.backing);
        }
        auto begin_offset = sem.entities[plan.backing_begin].member_offset;
        auto begin = begin_offset ? emit(Opcode::Index,IRType::I8,{destination.operand,Operand::integer(begin_offset)}) : destination;
        emit(Opcode::Store,IRType::Ptr,{backing.operand,begin.operand});
        auto size = emit(Opcode::Index,IRType::I8,{destination.operand,Operand::integer(sem.entities[plan.backing_size].member_offset)});
        emit(Opcode::Store,IRType::I64,{Operand::integer(call.argument_count),size.operand});
    } else if (plan.aggregate) {
        Value at = destination; at.address = true; at.type = t;
        if (plan.zero && sem.class_value(t) && !sem.empty_class(t)) zero_object(t,destination);
        else if (!call_aggregate_helper(object.initializer,at)) initialize_plan(object.initializer,at);
    } else if (plan.constructor) {
        if (plan.zero) {
            zero_object(t,destination);
        }
        if (sem.direct_transfer(plan.constructor)) {
            Value source = converted(sem.call_argument(call),sem.conversion_fact(call.conversions));
            if (!sem.empty_class(t)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(t); copy.alignment = sem.object_alignment(t);
                emit(copy,{source.operand,destination.operand});
            }
        } else if (sem.constructor_needed(plan.constructor)) {
            std::size_t begin = call_work.size();
            call_work.push_back(Operand::symbol(symbol(plan.constructor))); call_work.push_back(destination.operand);
            for (unsigned j = 0; j < call.argument_count; ++j)
                call_work.push_back(converted(sem.call_argument(call,j),sem.conversion_fact(call.conversions+j)).operand);
            guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
            call_work.resize(begin);
        }
    } else {
        value = call.argument_count ? converted(sem.call_argument(call),sem.conversion_fact(call.conversions)) :
            initialization_value(0,t);
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
