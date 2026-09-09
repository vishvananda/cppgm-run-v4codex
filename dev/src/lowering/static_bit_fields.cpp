#include "lowering/procedural.h"
#include <stdexcept>

namespace cppgm { namespace lowering {
void Procedural::global_bit_field(std::uint32_t plan, std::uint64_t& bytes)
{
    using lowir_model::DataItem;
    auto action = sem.initializers[plan];
    auto field = sem.field_fact(action.field);
    auto offset = sem.entities[action.field].member_offset;
    auto value = sem.static_value(action.source, action.type);
    if (value.kind != semantic::StaticValue::Integer)
        throw std::logic_error("nonconstant static bit-field");
    std::uint64_t mask = field.width == 64 ? ~std::uint64_t(0) : (std::uint64_t(1) << field.width)-1;
    std::uint64_t packed = (value.bits & mask) << field.shift;
    mask <<= field.shift;
    auto end = offset + (field.shift + field.width + 7)/8;
    if (bytes < end) {
        DataItem zero; zero.zero_bytes = end-bytes; p.data.push_back(zero); bytes = end;
    }
    // Layout may place the storage unit over earlier ordinary scalar fields.
    // Patch only the represented bits. At most eight trailing bytes are visited;
    // a large zero span is split without materializing its bytes.
    for (unsigned byte = 0; mask; ++byte, mask >>= 8, packed >>= 8) {
        unsigned bits = mask & 255;
        if (!bits) continue;
        std::uint64_t position = offset+byte, at = bytes;
        std::size_t index = p.data.size();
        while (index) {
            auto item = p.data[--index];
            auto width = item.kind == DataItem::Zero ? item.zero_bytes : item.type.bytes();
            at -= width;
            if (at > position) continue;
            if (item.kind == DataItem::Zero) {
                DataItem replacement[3]; unsigned count = 0;
                if (position > at) { DataItem zero; zero.zero_bytes = position-at; replacement[count++] = zero; }
                DataItem scalar; scalar.kind = DataItem::Scalar; scalar.type = IRType::U8;
                scalar.value = Operand::integer(packed & bits); replacement[count++] = scalar;
                if (position+1 < at+width) { DataItem zero; zero.zero_bytes = at+width-position-1; replacement[count++] = zero; }
                p.data[index] = replacement[0];
                for (unsigned j = 1; j < count; ++j)
                    p.data.insert(p.data.begin()+index+j, {replacement[j]});
            } else {
                if (item.kind != DataItem::Scalar || !item.type.integer())
                    throw std::logic_error("bit-field overlaps nonscalar static data");
                unsigned shift = (position-at)*8;
                auto selected = std::uint64_t(bits) << shift;
                p.data[index].value.data.integer = (item.value.data.integer & ~selected) | ((packed & bits) << shift);
            }
            break;
        }
    }
}
} }
