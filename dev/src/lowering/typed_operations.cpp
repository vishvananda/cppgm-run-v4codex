#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::typed_conversion(Value value, const semantic::Conversion& c, Value destination)
{
    bool supplied = destination.ir != IRType::Void;
    if (c.kind == semantic::Conversion::Kind::User) {
        auto record = sem.user_conversions[c.materialization];
        semantic::RangeOperation call;
        call.function = c.function; call.receiver = true; call.supplied = 1;
        call.returned = sem.types[sem.entities[c.function].type].child;
        call.result.type = reference(call.returned) ? sem.types[call.returned].child : call.returned;
        call.result.category = reference(call.returned) ? semantic::ValueCategory::Lvalue : semantic::ValueCategory::Prvalue;
        call.adjustment = record.adjustment; call.virtual_slot = record.virtual_slot;
        call.temporary = record.source_temporary;
        auto result = range_operation(call,{value});
        return typed_conversion(result,record.result,destination);
    }
    if (c.kind == semantic::Conversion::Kind::Construction) {
        auto recipe = sem.conversion_objects[c.materialization];
        auto target = reference(c.target) ? sem.types[c.target].child : c.target;
        Value storage;
        if (!supplied) { storage = class_temporary(recipe.temporary,target); destination = address(storage); }
        auto source = typed_conversion(value,sem.conversion_fact(recipe.call.conversions));
        if (sem.trivial_transfer(recipe.constructor) || sem.direct_transfer(recipe.constructor)) {
            if (!sem.empty_class(target)) {
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(target); copy.alignment = sem.object_alignment(target);
                emit(copy,{source.operand,destination.operand});
            }
        } else {
            std::vector<Operand> args{Operand::symbol(symbol(recipe.constructor)),destination.operand,source.operand};
            for (unsigned i = 1; i < recipe.call.argument_count; ++i)
                args.push_back(converted(sem.call_argument(recipe.call,i),sem.conversion_fact(recipe.call.conversions+i)).operand);
            guarded_call(Instruction(Opcode::Call,IRType::Void),args.data(),args.size());
        }
        if (!supplied && c.reference) activate_temporary(recipe.temporary);
        if (supplied || c.reference || sem.indirect_parameter(target)) return destination;
        storage.address = false; return storage;
    }
    if (supplied && sem.class_value(c.target)) {
        if (!sem.empty_class(c.target)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(c.target); copy.alignment = sem.object_alignment(c.target);
            emit(copy,{address(value).operand,destination.operand});
        }
        return destination;
    }
    auto converted = converted_value(value,c);
    if (supplied) { destination.type = c.target; destination.address = true; store(converted,destination); }
    return converted;
}
Value Procedural::range_operation(const semantic::RangeOperation& op, const std::vector<Value>& args, Value destination)
{
    if (!op.function) {
        if (op.op == OP_STAR) {
            auto result = typed_conversion(args[0],sem.conversion_fact(op.result.conversions));
            result.type = op.result.type; result.address = true; return result;
        }
        if (op.op == OP_NE) {
            auto a = typed_conversion(args[0],sem.conversion_fact(op.result.conversions));
            auto b = typed_conversion(args[1],sem.conversion_fact(op.result.conversions+1));
            auto result = emit(Opcode::Compare,a.ir,{a.operand,b.operand},Operation::Ne);
            result.type = op.result.type; return result;
        }
        if (op.op == OP_INC) {
            auto c = sem.conversion_fact(op.result.conversions);
            auto location = args[0];
            if (c.kind != semantic::Conversion::Kind::Standard || c.adjustment) {
                auto pointer = typed_conversion(args[0],c);
                location = Value(pointer.operand,IRType::Ptr,sem.types[c.target].child,true);
            }
            auto prior = load(location);
            auto computation = sem.conversion_fact(op.result.conversions+op.result.count-1).target;
            Value next;
            if (sem.types[location.type].kind == TypeKind::Fundamental && sem.types[location.type].fundamental == FT_BOOL)
                next = Value(Operand::integer(1),IRType::U8,location.type);
            else next = operation(OP_PLUS,convert(prior,computation),
                Value(Operand::integer(1),IRType::I32,sem.types.fundamental(FT_INT)),computation);
            store(convert(next,location.type),location); return location;
        }
        throw std::logic_error("missing typed range operation");
    }
    bool class_result = sem.class_value(op.returned), indirect = sem.indirect_value(op.returned);
    bool own = class_result && destination.ir == IRType::Void;
    if (own) destination = class_address(op.temporary,op.result.type);
    std::vector<Operand> operands{Operand::symbol(symbol(op.function))};
    if (indirect) operands.push_back(destination.operand);
    if (op.receiver) {
        auto object = base_projection(address(args[0]),op.adjustment);
        operands.push_back(object.operand);
        if (op.virtual_slot) operands[0] = virtual_function(object,op.virtual_slot).operand;
    }
    for (unsigned i = op.receiver; i < args.size(); ++i)
        operands.push_back(typed_conversion(args[i],sem.conversion_fact(op.result.conversions+i)).operand);
    for (unsigned i = args.size(); i < op.result.count; ++i)
        operands.push_back(converted(sem.default_argument_value(op.function,i-op.receiver),sem.conversion_fact(op.result.conversions+i)).operand);
    Instruction call(Opcode::Call,indirect ? IRType(IRType::Void) : type(op.returned));
    if (op.virtual_slot) call.signature = virtual_signature(op.function);
    auto value = guarded_call(call,operands.data(),operands.size());
    if (class_result) {
        if (!indirect && !sem.empty_class(op.returned)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(op.returned); copy.alignment = sem.object_alignment(op.returned);
            emit(copy,{value.operand,destination.operand});
        }
        if (own) activate_temporary(op.temporary);
        value = destination; value.address = true;
    } else value.address = reference(op.returned);
    value.type = op.result.type; return value;
}
void Procedural::range_initialize(EntityId object, const semantic::RangeOperation& op, const std::vector<EntityId>& entities, std::uint32_t conversion)
{
    auto t = sem.entities[object].type;
    if (!objects[object]) objects[object] = source_slot(object);
    Value location(Operand::slot(objects[object]),type(t),t,true);
    auto c = sem.conversion_fact(conversion);
    bool direct = sem.class_value(t) && op.result.category == semantic::ValueCategory::Prvalue &&
        sem.types.unqualified(op.result.type) == sem.types.unqualified(t);
    auto initial = live;
    full_expression.enabled = full_expression.lexical = live != 0;
    auto destination = sem.class_value(t) ? address(location) : Value();
    std::vector<Value> args;
    for (auto entity : entities) args.push_back(binding(entity));
    if (direct) range_operation(op,args,destination);
    else {
        auto value = range_operation(op,args);
        if (sem.class_value(t)) typed_conversion(value,c,destination);
        else store(typed_conversion(value,c),location);
    }
    finish_full_expression(initial);
    if (auto lifetime = sem.object_lifetime(object)) live = lifetime;
}
} }
