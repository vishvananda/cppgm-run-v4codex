#include "semantic/analyzer.h"
#include <cmath>
#include <cstring>

namespace cppgm { namespace semantic {
bool Analyzer::floating_type(TypeId t) const
{
    return types[t].kind==TypeKind::Fundamental && floating_precision(types[t].fundamental);
}
ExtendedFloat Analyzer::floating_value(Constant v) const
{
    if (floating_type(v.type)) return floating_constants[v.bits].value;
    auto bits = integer_value(v);
    return is_unsigned(v.type) ? static_cast<ExtendedFloat>(bits) : static_cast<ExtendedFloat>(__int128(bits));
}
bool Analyzer::constant_truth(Constant v) const
{
    if (complex_type(v.type)) return floating_constants[std::uint32_t(v.bits)].value != 0 || floating_constants[v.bits >> 32].value != 0;
    return floating_type(v.type) ? floating_value(v) != 0 : v.bits != 0;
}
Constant Analyzer::floating_constant(TypeId t, ExtendedFloat v, bool special, bool signaling)
{
    // Each operation/conversion observes the destination precision. The host
    // and target are Linux x86-64 (IEEE float/double and 80-bit long double).
    if ((floating_kind(t)==FT_FLOAT)) { volatile float rounded = v; v = rounded; }
    else if ((floating_kind(t)==FT_DOUBLE)) { volatile double rounded = v; v = rounded; }
    if ((floating_kind(t)==FT_LONG_DOUBLE)) v = static_cast<long double>(v);
    if ((floating_kind(t)==FT_FLOAT16)) v = half_value(half_bits(v));
    if (!special && !finite_float(v)) return Constant();
    std::uint64_t significand = 0, exponent = 0;
    std::memcpy(&significand,&v,8);
    std::memcpy(&exponent,reinterpret_cast<const char*>(&v)+8,8);
    auto hash = significand ^ (std::uint64_t(exponent)*0x9e3779b97f4a7c15ULL);
    auto head = floating_constant_index.get(hash);
    for (auto i = head; i; i = floating_constants[i].next)
        if (floating_constants[i].significand == significand && floating_constants[i].exponent == exponent && floating_constants[i].signaling == signaling) return Constant(t,i);
    auto id = floating_constants.size();
    floating_constants.push_back({v,significand,exponent,head,signaling}); floating_constant_index.put(hash,id);
    return Constant(t,id);
}
Constant Analyzer::floating_conversion(Constant v, TypeId to)
{
    if ((!integral(v.type) && !floating_type(v.type)) || (!integral(to) && !floating_type(to))) return Constant();
    auto value = floating_value(v);
    if (integral(v.type) && floating_type(to)) {
        auto precision=floating_precision(types[to].fundamental);
        value=integer_float(integer_value(v),!is_unsigned(v.type)&&negative_constant(v),precision);
    }
    if (floating_type(to)) return floating_constant(to,value,!finite_float(value),types.unqualified(to) == types.unqualified(v.type) && floating_signaling(v));
    if (fundamental(to,FT_BOOL)) return Constant(to,value != 0);
    if (!finite_float(value)) return Constant();
    // Test the truncated value before any host cast; out-of-range conversion
    // is a core constant-expression failure, including the unsigned case.
    // Integer range checks precede the truncating cast, including fractions
    // immediately below zero for unsigned destinations.
    auto truncated = trunc_float(value);
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
    bool special = !finite_float(x) || !finite_float(y);
    // C++11 [expr]/12 permits excess precision for the operation. Use the
    // Linux x86-64 x87 model, then materialize the destination precision;
    // do not claim this is a single IEEE rounding for double arithmetic.
    bool result;
    switch (op) {
    case OP_PLUS: return floating_constant(common,(floating_kind(common)==FT_FLOAT128) ? x+y : ExtendedFloat(static_cast<long double>(x)+static_cast<long double>(y)),special);
    case OP_MINUS: return floating_constant(common,(floating_kind(common)==FT_FLOAT128) ? x-y : ExtendedFloat(static_cast<long double>(x)-static_cast<long double>(y)),special);
    case OP_STAR: return floating_constant(common,(floating_kind(common)==FT_FLOAT128) ? x*y : ExtendedFloat(static_cast<long double>(x)*static_cast<long double>(y)),special);
    case OP_DIV: return y == 0 ? Constant() : floating_constant(common,(floating_kind(common)==FT_FLOAT128) ? x/y : ExtendedFloat(static_cast<long double>(x)/static_cast<long double>(y)),special);
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
