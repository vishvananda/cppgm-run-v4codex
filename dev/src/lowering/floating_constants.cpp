#include "lowering/procedural.h"
#include <cmath>
#include <cstring>
namespace cppgm { namespace lowering {
using lowir_model::DataItem;
DataItem Procedural::floating_data(IRType t, ExtendedFloat value, bool signaling)
{
    DataItem data; data.kind = DataItem::Scalar; data.type = t;
    if (t == IRType::F128) {
        if (!nan_float(value)) { data.value = Operand::extended(value,signaling); return data; }
        std::uint64_t bits[2]; std::memcpy(bits,&value,16);
        if (signaling) { bits[1]&=~(std::uint64_t(1)<<47); if (!bits[0] && !(bits[1]&0xffffffffffffULL)) bits[0]=1; }
        data.type=IRType::I128; data.value=Operand::integer(bits[0]); data.value.integer_high(bits[1]); return data;
    }
    if (t == IRType::F16) { data.type=IRType::U16; data.value=Operand::integer(half_bits(value)); return data; }
    data.value = Operand::floating(static_cast<long double>(value),signaling);
    if (!nan_float(value)) return data;
    unsigned char bytes[16] = {};
    if (t == IRType::F32) { float v = value; std::memcpy(bytes,&v,4); }
    else if (t == IRType::F64) { double v = value; std::memcpy(bytes,&v,8); }
    else { long double v=value; std::memcpy(bytes,&v,10); }
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
Value Procedural::floating_literal(TypeId t, ExtendedFloat value, bool signaling)
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
