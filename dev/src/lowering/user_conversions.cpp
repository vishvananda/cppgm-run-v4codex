#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::user_conversion(NodeId n, const semantic::Conversion& c, Value destination)
{
    auto record = sem.user_conversions[c.materialization];
    if (!record.prepared) throw std::logic_error("missing selected user conversion");
    TypeId returned = sem.types[sem.entities[c.function].type].child;
    bool class_result = sem.class_value(returned);
    SlotId result_slot = !class_result && type(returned) != IRType::Void && full_expression.enabled && !sem.function_nonthrowing(c.function) ? builder->add_slot(0,type(returned)) : SlotId();
    bool supplied = destination.ir != IRType::Void;
    bool transfer = record.result.kind == semantic::Conversion::Kind::Construction;
    TypeId target = reference(c.target) ? sem.types[c.target].child : c.target;
    if ((class_result || transfer) && !supplied) destination = class_address(record.temporary,sem.entities[record.temporary].type);
    Value call_destination = record.source_temporary ? class_address(record.source_temporary,returned) : destination;
    Value object = record.object_entity ? converted_value(binding(record.object_entity),record.object) : converted(n,record.object);
    object = base_projection(object,record.adjustment);
    Operand arguments[3]; std::size_t count = 0;
    arguments[count++] = record.virtual_slot ? virtual_function(object,record.virtual_slot).operand : Operand::symbol(symbol(c.function));
    if (sem.indirect_value(returned)) arguments[count++] = call_destination.operand;
    arguments[count++] = object.operand;
    Instruction call(Opcode::Call,sem.indirect_value(returned) ? IRType(IRType::Void) : type(returned));
    if (record.virtual_slot) call.signature = virtual_signature(c.function);
    Value result = guarded_call(call,arguments,count);
    if (result_slot) {
        emit(Opcode::Store,result.ir,{result.operand,Operand::slot(result_slot)});
        result = emit(Opcode::Load,result.ir,{Operand::slot(result_slot)});
    }
    if (class_result) {
        if (!sem.indirect_value(returned) && !sem.empty_class(returned)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(returned); copy.alignment = sem.object_alignment(returned);
            emit(copy,{result.operand,call_destination.operand});
        }
        if (record.source_temporary) activate_temporary(record.source_temporary);
        result = call_destination;
    }
    result.type = reference(returned) ? sem.types[returned].child : returned;
    result.address = reference(returned) || class_result;
    if (supplied && !class_result && !transfer) {
        auto second = record.result; second.reference = second.temporary = false; second.target = target;
        destination.type = target; destination.address = true;
        store(converted_value(result,second),destination); destination.address = false; return destination;
    }
    if (transfer) {
        auto materialized = sem.conversion_objects[record.result.materialization];
        auto call = materialized.call;
        Value source = converted_value(result,sem.conversion_fact(call.conversions));
        // A nonempty trivial transfer has a direct storage operation. Empty
        // retained transfers keep the selected address-based call boundary.
        if (sem.direct_transfer(materialized.constructor) && (!materialized.retained || !sem.empty_class(target))) {
            if (!sem.empty_class(target)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(target); copy.alignment = sem.object_alignment(target);
                emit(copy,{source.operand,destination.operand});
            }
        } else {
            std::size_t begin = call_work.size();
            call_work.push_back(Operand::symbol(symbol(materialized.constructor)));
            call_work.push_back(destination.operand); call_work.push_back(source.operand);
            for (unsigned j = 1; j < call.argument_count; ++j)
                call_work.push_back(converted(sem.call_arguments[call.arguments+j],sem.conversion_fact(call.conversions+j)).operand);
            guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
            call_work.resize(begin);
        }
    }
    if (class_result || transfer) {
        if (supplied) return destination;
        if (c.reference) { activate_temporary(record.temporary); return converted_value(result,record.result); }
        if (sem.indirect_parameter(target)) return destination;
        return Value(class_temporary(record.temporary,target).operand,type(target),target);
    }
    return converted_value(result,record.result);
}
} }
