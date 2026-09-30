#include "semantic/analyzer.h"
#include <cmath>
#include <cstring>

namespace cppgm { namespace semantic {
bool Analyzer::floating_type(TypeId t) const
{
    return types[t].kind == TypeKind::Fundamental && types[t].fundamental >= FT_FLOAT &&
        types[t].fundamental <= FT_LONG_DOUBLE;
}
long double Analyzer::floating_value(Constant v) const
{
    if (floating_type(v.type)) return floating_constants[v.bits].value;
    auto bits = integer_value(v);
    return is_unsigned(v.type) ? static_cast<long double>(bits) : static_cast<long double>(__int128(bits));
}
bool Analyzer::constant_truth(Constant v) const
{
    return floating_type(v.type) ? floating_value(v) != 0 : v.bits != 0;
}
Constant Analyzer::floating_constant(TypeId t, long double v, bool special)
{
    // Each operation/conversion observes the destination precision. The host
    // and target are Linux x86-64 (IEEE float/double and 80-bit long double).
    if (fundamental(t,FT_FLOAT)) { volatile float rounded = v; v = rounded; }
    else if (fundamental(t,FT_DOUBLE)) { volatile double rounded = v; v = rounded; }
    if (!special && !std::isfinite(v)) return Constant();
    std::uint64_t significand = 0; std::uint16_t exponent = 0;
    std::memcpy(&significand,&v,8);
    std::memcpy(&exponent,reinterpret_cast<const char*>(&v)+8,2);
    auto hash = significand ^ (std::uint64_t(exponent)*0x9e3779b97f4a7c15ULL);
    auto head = floating_constant_index.get(hash);
    for (auto i = head; i; i = floating_constants[i].next)
        if (floating_constants[i].significand == significand && floating_constants[i].exponent == exponent) return Constant(t,i);
    auto id = floating_constants.size();
    floating_constants.push_back({v,significand,exponent,head}); floating_constant_index.put(hash,id);
    return Constant(t,id);
}
Constant Analyzer::floating_conversion(Constant v, TypeId to)
{
    if ((!integral(v.type) && !floating_type(v.type)) || (!integral(to) && !floating_type(to))) return Constant();
    auto value = floating_value(v);
    if (floating_type(to)) return floating_constant(to,value,!std::isfinite(value));
    if (fundamental(to,FT_BOOL)) return Constant(to,value != 0);
    if (!std::isfinite(value)) return Constant();
    // Test the truncated value before any host cast; out-of-range conversion
    // is a core constant-expression failure, including the unsigned case.
    auto truncated = std::trunc(value);
    auto limit = std::ldexp(1.0L,width(to)-(is_unsigned(to) ? 0 : 1));
    if (truncated >= limit || truncated < (is_unsigned(to) ? 0 : -limit)) return Constant();
    return integer_constant(to,is_unsigned(to) ? WideInteger(truncated) : WideInteger(__int128(truncated)));
}
Constant Analyzer::floating_binary(ETokenType op, Constant a, Constant b, bool converted)
{
    auto common = converted ? a.type : arithmetic_type(a.type,b.type);
    a = convert(a,common,true); b = convert(b,common,true);
    if (!a.valid || !b.valid) return Constant();
    auto x = floating_value(a), y = floating_value(b);
    bool special = !std::isfinite(x) || !std::isfinite(y);
    // C++11 [expr]/12 permits excess precision for the operation. Use the
    // Linux x86-64 x87 model, then materialize the destination precision;
    // do not claim this is a single IEEE rounding for double arithmetic.
    bool result;
    switch (op) {
    case OP_PLUS: return floating_constant(common,x+y,special);
    case OP_MINUS: return floating_constant(common,x-y,special);
    case OP_STAR: return floating_constant(common,x*y,special);
    case OP_DIV: return y == 0 ? Constant() : floating_constant(common,x/y,special);
    case OP_EQ: result = x == y; break;
    case OP_NE: result = x != y; break;
    case OP_LT: result = x < y; break;
    case OP_GT: result = x > y; break;
    case OP_LE: result = x <= y; break;
    case OP_GE: result = x >= y; break;
    case OP_LAND: result = x && y; break;
    case OP_LOR: result = x || y; break;
    default: return Constant();
    }
    return Constant(types.fundamental(FT_BOOL),result);
}
} }
