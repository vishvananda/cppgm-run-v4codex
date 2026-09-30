#include "lowering/procedural.h"
#include <cmath>
#include <cstring>
namespace cppgm { namespace lowering {
using lowir_model::DataItem;
DataItem Procedural::floating_data(IRType t, long double value, bool signaling)
{
    DataItem data; data.kind = DataItem::Scalar; data.type = t;
    data.value = Operand::floating(value,signaling);
    if (!std::isnan(value)) return data;
    unsigned char bytes[16] = {};
    if (t == IRType::F32) { float v = value; std::memcpy(bytes,&v,4); }
    else if (t == IRType::F64) { double v = value; std::memcpy(bytes,&v,8); }
    else std::memcpy(bytes,&value,10);
    unsigned last = t == IRType::F32 ? 2 : t == IRType::F64 ? 6 : 7;
    unsigned quiet = t == IRType::F64 ? 8 : 64;
    bool payload = (bytes[last] & (quiet-1)) != 0;
    for (unsigned j = 0; j < last; ++j) payload |= bytes[j] != 0;
    if (!signaling && !payload) return data;
    if (signaling) { bytes[last] &= ~quiet; if (!payload) bytes[0] |= 1; }
    std::uint64_t low = 0, high = 0;
    std::memcpy(&low,bytes,8); std::memcpy(&high,bytes+8,8);
    data.type = t == IRType::F32 ? IRType::U32 : t == IRType::F64 ? IRType::I64 : IRType::I128;
    data.value = Operand::integer(low);
    if (t == IRType::F80) data.value.integer_high(high);
    return data;
}
Value Procedural::floating_literal(TypeId t, long double value, bool signaling)
{
    auto ir = type(t); auto data = floating_data(ir,value,signaling);
    if (data.type == ir) return Value(data.value,ir,t);
    // LowIR's nan/snan words do not specify a payload. Reinterpret exact bits
    // through typed storage rather than attaching an unprintable backend fact.
    auto slot = builder->add_slot(0,ir);
    emit(Opcode::Store,data.type,{data.value,Operand::slot(slot)});
    auto result = emit(Opcode::Load,ir,{Operand::slot(slot)});
    result.type = t; return result;
}
} }
