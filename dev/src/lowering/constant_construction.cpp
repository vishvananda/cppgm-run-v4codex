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
        if (offset > end) { lowir_model::DataItem zero; zero.zero_bytes = offset-end; p.data.push_back(zero); }
        lowir_model::DataItem item; item.type = type(field.type);
        auto value = field.value;
        if (value.kind == semantic::StaticValue::Address) { item.kind = lowir_model::DataItem::Address; item.symbol = symbol(value.entity); item.addend = value.addend; }
        else if (value.kind == semantic::StaticValue::String) { string_literal(value.string); item.kind = lowir_model::DataItem::Address; item.symbol = strings[value.string]; item.addend = value.addend; }
        else { item.kind = lowir_model::DataItem::Scalar; item.value = value.kind == semantic::StaticValue::Floating ? Operand::floating(value.floating) : Operand::integer(value.bits); }
        p.data.push_back(item); end = offset + type(field.type).bytes();
    }
    if (sem.object_size(t) > end) { lowir_model::DataItem zero; zero.zero_bytes = sem.object_size(t)-end; p.data.push_back(zero); }
}
} }

namespace cppgm { namespace lowering {
Value Procedural::constant_operand(semantic::Constant c, TypeId t)
{
    auto v = sem.constant_static_value(c);
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
    return Value(v.kind == semantic::StaticValue::Floating ? Operand::floating(v.floating) : Operand::integer(v.bits),type(t),t);
}
} }
