#include "semantic/analyzer.h"
#include <cmath>
#include <complex>
namespace cppgm { namespace semantic {
bool Analyzer::complex_type(TypeId t) const
{
    return types[t].kind == TypeKind::Fundamental && types[t].fundamental >= FT_COMPLEX_FLOAT &&
        types[t].fundamental <= FT_COMPLEX_LONG_DOUBLE;
}
TypeId Analyzer::complex_component(TypeId t)
{
    return types.fundamental(EFundamentalType(FT_FLOAT + types[t].fundamental - FT_COMPLEX_FLOAT));
}
Expression Analyzer::complex_projection(Expression source, ETokenType op)
{
    Expression result;
    bool complex = complex_type(source.type);
    if (!complex && !arithmetic(source.type)) return result;
    result.type = complex ? types.qualify(complex_component(source.type),types[source.type].cv) : source.type;
    result.category = complex || op == KW_REAL ? source.category : ValueCategory::Prvalue;
    return result;
}
EntityId Analyzer::complex_signature(EntityId family, TypeId first, TypeId second)
{
    first = types.unqualified(first); second = types.unqualified(second);
    if (!floating_type(first) || first != second) return 0;
    if (auto old = complex_signatures.get(first)) return old;
    auto result = types.fundamental(EFundamentalType(FT_COMPLEX_FLOAT + types[first].fundamental - FT_FLOAT));
    auto e = make_entity(EntityKind::Function,global,entities[family].name,0);
    entities[e].type = types.function(result,{first,first},false); entities[e].exception_spec = 129;
    intrinsic_functions.put(e,unsigned(Intrinsic::Complex)); complex_signatures.put(first,e); return e;
}
Constant Analyzer::complex_constant(TypeId t, Constant real, Constant imag)
{
    if (!real.valid || !imag.valid) return {};
    auto component = complex_component(t);
    real = convert(real,component,true); imag = convert(imag,component,true);
    if (!real.valid || !imag.valid) return {};
    // Canonical component constants already have TU-local interned identity.
    // A pair fits the ordinary constant payload, with no new per-value node.
    return Constant(t,real.bits | (imag.bits << 32));
}
Constant Analyzer::complex_part(Constant v, unsigned part)
{
    return v.valid ? Constant(complex_component(v.type),part ? v.bits >> 32 : std::uint32_t(v.bits)) : Constant();
}
Constant Analyzer::complex_conversion(Constant v, TypeId target)
{
    if (complex_type(v.type)) {
        if (fundamental(target,FT_BOOL)) return Constant(target,constant_truth(v));
        if (!complex_type(target)) return convert(complex_part(v,0),target,true);
        return complex_constant(target,complex_part(v,0),complex_part(v,1));
    }
    if (!arithmetic(v.type)) return {};
    return complex_constant(target,v,floating_constant(complex_component(target),0));
}
Constant Analyzer::complex_binary(ETokenType op, Constant a, Constant b)
{
    auto common = arithmetic_type(a.type,b.type);
    a = convert(a,common,true); b = convert(b,common,true);
    if (!a.valid || !b.valid) return {};
    auto ar = complex_part(a,0), ai = complex_part(a,1), br = complex_part(b,0), bi = complex_part(b,1);
    std::complex<long double> x(floating_value(ar),floating_value(ai)), y(floating_value(br),floating_value(bi)), z;
    if (op == OP_EQ || op == OP_NE) return Constant(types.fundamental(FT_BOOL),op == OP_EQ ? x == y : x != y);
    if (op == OP_PLUS) z = x+y;
    else if (op == OP_MINUS) z = x-y;
    else if (op == OP_STAR) z = x*y;
    else if (op == OP_DIV && y != std::complex<long double>()) z = x/y;
    else return {};
    // As for scalar floating arithmetic, an already admitted infinity/NaN
    // can propagate through constant evaluation. Finite overflow remains a
    // failed constant expression; it must not invent an exceptional input.
    bool special = !std::isfinite(x.real()) || !std::isfinite(x.imag()) ||
        !std::isfinite(y.real()) || !std::isfinite(y.imag());
    return complex_constant(common,floating_constant(ar.type,z.real(),special),floating_constant(ar.type,z.imag(),special));
}
} }
