#include "lowering/procedural.h"
#include <stdexcept>

namespace cppgm { namespace lowering {
void Procedural::global_construction(NodeId n, TypeId t)
{
    auto plan = sem.constant_construction(n,t);
    if (!plan.valid) throw std::logic_error("missing constant construction facts");
    std::uint64_t end = 0;
    for (unsigned j = 0; j < plan.count; ++j) {
        auto field = sem.constant_fields[plan.first+j];
        auto offset = sem.entities[field.field].member_offset;
        if (offset > end) { lowir_model::DataItem zero; zero.zero_bytes = offset-end; p.data.push_back(zero); }
        lowir_model::DataItem item; item.type = type(field.type);
        auto value = field.value;
        if (value.kind == semantic::StaticValue::Address) { item.kind = lowir_model::DataItem::Address; item.symbol = symbol(value.entity); item.addend = value.addend; }
        else if (value.kind == semantic::StaticValue::String) { item.kind = lowir_model::DataItem::Address; item.symbol = strings[value.string]; }
        else { item.kind = lowir_model::DataItem::Scalar; item.value = value.kind == semantic::StaticValue::Floating ? Operand::floating(value.floating) : Operand::integer(value.bits); }
        p.data.push_back(item); end = offset + type(field.type).bytes();
    }
    if (sem.object_size(t) > end) { lowir_model::DataItem zero; zero.zero_bytes = sem.object_size(t)-end; p.data.push_back(zero); }
}
} }
