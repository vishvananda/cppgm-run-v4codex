#include "lowering/procedural.h"
#include <stdexcept>

namespace cppgm { namespace lowering {
void Procedural::global_construction(NodeId n, TypeId t)
{
    global_constant_fields(sem.constant_construction(n,t),t);
}
void Procedural::global_constant_fields(const semantic::ConstantObject& plan, TypeId t)
{
    if (!plan.valid) throw std::logic_error("missing constant construction facts");
    std::uint64_t end = 0;
    for (unsigned j = 0; j < plan.count; ++j) {
        auto field = sem.constant_fields[plan.first+j];
        auto offset = field.offset;
        if (field.field && sem.field_fact(field.field).bit_field) {
            if (field.value.kind != semantic::StaticValue::Integer)
                throw std::logic_error("nonintegral constant bit-field fact");
            global_bit_field_value(field.field,offset,field.value.bits,end); continue;
        }
        if (offset > end) { lowir_model::DataItem zero; zero.zero_bytes = offset-end; p.data.push_back(zero); }
        auto value = field.value;
        if (value.kind == semantic::StaticValue::Vtable) {
            const auto& model = sem.virtual_class(value.entity);
            lowir_model::DataItem item; item.kind = lowir_model::DataItem::Address; item.type = IRType::Ptr;
            item.symbol = value.bits ? view_symbol(value.entity,value.bits) : vtable_symbol(value.entity);
            item.addend = value.bits ? model.views[value.bits-1].address_point : model.address_point;
            p.data.push_back(item); end = offset+8; continue;
        }
        if (value.kind == semantic::StaticValue::MemberFunction) {
            member_pointer_data(value); end = offset+16; continue;
        }
        if (value.kind == semantic::StaticValue::Complex) {
            complex_data(field.type,semantic::Constant(field.type,value.bits)); end = offset+type(field.type).bytes(); continue;
        }
        lowir_model::DataItem item; item.type = type(field.type);
        if (value.kind == semantic::StaticValue::Address) { item.kind = lowir_model::DataItem::Address; item.symbol = symbol(value.entity); item.addend = value.addend; }
        else if (value.kind == semantic::StaticValue::String) { string_literal(value.string); item.kind = lowir_model::DataItem::Address; item.symbol = strings[value.string]; item.addend = value.addend; }
        else if (value.kind == semantic::StaticValue::Floating) item = floating_data(item.type,value.floating,value.signaling);
        else { item.kind = lowir_model::DataItem::Scalar; item.value = integer_operand(semantic::Constant(field.type,value.bits)); }
        p.data.push_back(item); end = offset + type(field.type).bytes();
    }
    if (sem.object_size(t) > end) { lowir_model::DataItem zero; zero.zero_bytes = sem.object_size(t)-end; p.data.push_back(zero); }
}
} }

namespace cppgm { namespace lowering {
Operand Procedural::integer_operand(semantic::Constant c)
{
    if (type(c.type) != IRType::I128 || sem.types[c.type].kind == TypeKind::MemberPointer)
        return Operand::integer(c.bits);
    auto bits = sem.integer_value(c);
    auto result = Operand::integer(std::uint64_t(bits));
    result.integer_high(std::uint64_t(bits >> 64)); return result;
}
Value Procedural::constant_operand(semantic::Constant c, TypeId t)
{
    auto v = sem.constant_static_value(c);
    if (v.kind == semantic::StaticValue::MemberFunction) return member_pointer_value(v.entity,t,v.addend);
    if (v.kind == semantic::StaticValue::Address || v.kind == semantic::StaticValue::String) {
        if (v.kind == semantic::StaticValue::String) string_literal(v.string);
        auto target = v.kind == semantic::StaticValue::Address ? symbol(v.entity) : strings[v.string];
        Value result(Operand::symbol(target),IRType::Ptr,t,true);
        if (!reference(c.type)) result = address(result);
        if (v.addend) {
            if (result.address) result = address(result);
            result = emit(Opcode::Index,IRType::I8,{result.operand,Operand::integer(v.addend)});
        }
        result.type = t; result.address = reference(c.type); return result;
    }
    if (v.kind == semantic::StaticValue::Invalid) throw std::logic_error("missing lowered scalar constant fact");
    if (v.kind == semantic::StaticValue::Complex) {
        auto real = sem.complex_part(c,0), imag = sem.complex_part(c,1);
        return complex_construct(t,floating_literal(real.type,sem.floating_value(real),sem.floating_signaling(real)),floating_literal(imag.type,sem.floating_value(imag),sem.floating_signaling(imag)));
    }
    if (v.kind == semantic::StaticValue::Floating) return floating_literal(t,v.floating,v.signaling);
    return Value(integer_operand(semantic::Constant(c.type,v.bits)),type(t),t);
}
} }
