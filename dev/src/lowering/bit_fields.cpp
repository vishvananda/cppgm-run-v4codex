#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
namespace {
std::uint64_t mask(unsigned width) { return width == 64 ? ~std::uint64_t(0) : (std::uint64_t(1)<<width)-1; }
IRType access_type(IRType t) { return t == IRType::U32 ? IRType(IRType::I32) : t; }
}
Value Procedural::initialization_value(NodeId n, TypeId t)
{
    if (!n) return Value(type(t).floating() ? Operand::floating(0) : Operand::integer(0), type(t), t);
    auto input = sem.expression_fact(n).incoming;
    if (reference(t) || (input && sem.conversion_fact(input).derived))
        return input ? converted(n, sem.conversion_fact(input)) : convert(expression(n, reference(t)), t);
    Value value = load(expression(n));
    IRType target = type(t);
    bool boolean = sem.types[t].kind == TypeKind::Fundamental && sem.types[t].fundamental == FT_BOOL;
    if (!boolean && value.ir.integer() && target.integer() && target.width() < value.ir.width()) {
        Instruction conversion(Opcode::Convert, target); conversion.source_type = value.ir; conversion.operation = Operation::Trunc;
        value = emit(conversion, {value.operand}); value.type = t; return value;
    }
    return convert(value, t);
}
Value Procedural::load_bit_field(Value location)
{
    auto field = sem.field_fact(location.bit_field);
    IRType storage = access_type(type(field.storage_type));
    Value value;
    if (location.cached && !(sem.types[location.type].cv & 2)) value = Value(location.stored, storage);
    else {
        Instruction read(Opcode::Load, storage); read.is_volatile = sem.types[location.type].cv & 2;
        value = emit(read, {location.operand});
        if (field.shift) value = emit(Opcode::Binary, storage, {value.operand, Operand::integer(field.shift)}, storage == IRType::U8 || storage == IRType::U16 || storage == IRType::U32 ? Operation::Ushr : Operation::Shr);
        value = emit(Opcode::Binary, storage, {value.operand, Operand::integer(mask(field.width))}, Operation::And);
    }
    if (!sem.unsigned_type(field.storage_type) && field.width < storage.width()) {
        auto shift = Operand::integer(storage.width()-field.width);
        value = emit(Opcode::Binary, storage, {value.operand, shift}, Operation::Shl);
        value = emit(Opcode::Binary, storage, {value.operand, shift}, Operation::Shr);
    }
    value = coerce(value, type(location.type), sem.unsigned_type(field.storage_type));
    value.type = location.type; return value;
}
Value Procedural::store_bit_field(Value value, Value location)
{
    auto field = sem.field_fact(location.bit_field);
    IRType storage = location.initializing ? type(field.storage_type) : access_type(type(field.storage_type));
    bool boolean = sem.types[field.storage_type].kind == TypeKind::Fundamental && sem.types[field.storage_type].fundamental == FT_BOOL;
    if (!boolean && value.ir.integer() && value.ir.width() == storage.width())
        value = coerce(value, storage, sem.unsigned_type(value.type));
    else if (!boolean && value.ir.integer() && value.ir.width() > storage.width()) {
        Instruction convert(Opcode::Convert, storage); convert.source_type = value.ir; convert.operation = Operation::Trunc;
        value = emit(convert, {value.operand});
    } else {
        value = convert(value, field.storage_type);
        value = coerce(value, storage, sem.unsigned_type(field.storage_type));
    }
    auto bits = mask(field.width);
    Value assigned = location.initializing ?
        emit(Opcode::Binary, storage, {Operand::integer(bits), value.operand}, Operation::And) :
        emit(Opcode::Binary, storage, {value.operand, Operand::integer(bits)}, Operation::And);
    Value packed = assigned;
    if (field.shift) packed = emit(Opcode::Binary, storage, {packed.operand, Operand::integer(field.shift)}, Operation::Shl);
    bool first = location.initializing && field.may_clear_unit && !initialized_units.get(location.init_offset+1);
    if (location.initializing) initialized_units.put(location.init_offset+1, 1);
    if (!first) {
        Instruction read(Opcode::Load, storage); read.is_volatile = sem.types[location.type].cv & 2;
        Value old = emit(read, {location.operand});
        old = emit(Opcode::Binary, storage, {old.operand, Operand::integer(mask(storage.width()) & ~(bits << field.shift))}, Operation::And);
        packed = emit(Opcode::Binary, storage, {old.operand, packed.operand}, Operation::Or);
    }
    Instruction write(Opcode::Store, storage); write.is_volatile = sem.types[location.type].cv & 2;
    emit(write, {packed.operand, location.operand});
    assigned.type = location.type; return assigned;
}
} }
