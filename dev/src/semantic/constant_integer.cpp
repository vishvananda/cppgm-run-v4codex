#include "semantic/analyzer.h"
#include <algorithm>

namespace cppgm { namespace semantic {
Analyzer::WideInteger Analyzer::integer_value(Constant v) const
{
    if (width(v.type) == 128) return wide_constants.at(v.bits).value;
    return is_unsigned(v.type) ? WideInteger(v.bits) : WideInteger(__int128(std::int64_t(v.bits)));
}
Constant Analyzer::integer_constant(TypeId t, WideInteger value)
{
    auto bits = width(t);
    if (bits < 128) {
        auto mask = (WideInteger(1) << bits)-1;
        value &= mask;
        if (!is_unsigned(t) && (value & (WideInteger(1) << (bits-1)))) value |= ~mask;
        return Constant(t,std::uint64_t(value));
    }
    if (!value) return Constant(t,0);
    auto hash = std::uint64_t(value) ^ (std::uint64_t(value >> 64)*0x9e3779b97f4a7c15ULL);
    auto head = wide_constant_index.get(hash);
    for (auto i = head; i; i = wide_constants[i].next)
        if (wide_constants[i].value == value) return Constant(t,i);
    auto id = wide_constants.size();
    wide_constants.push_back({value,head}); wide_constant_index.put(hash,id);
    return Constant(t,id);
}
bool Analyzer::negative_constant(Constant v) const
{
    return !is_unsigned(v.type) && (integer_value(v) >> 127);
}
bool Analyzer::same_integer_value(Constant a, Constant b) const
{
    return a.valid && b.valid && integer_value(a) == integer_value(b) &&
        negative_constant(a) == negative_constant(b);
}
std::string Analyzer::integer_text(Constant v) const
{
    auto value = integer_value(v);
    bool negative = negative_constant(v);
    if (negative) value = 0-value;
    std::string text;
    do { text += char('0'+value%10); value /= 10; } while (value);
    if (negative) text += '-';
    std::reverse(text.begin(),text.end()); return text;
}
} }
