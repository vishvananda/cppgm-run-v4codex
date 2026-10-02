#include "support/builtin_registry.h"
#include "support/type_traits.h"
#include "support/atomic_builtins.h"
#include <cstring>
#include "support/packed_builtins.h"
#include "support/x86_builtins.h"
namespace cppgm {
bool hosted_builtin(TextView name)
{
    return name.equals("__builtin_shuffle") || x86_builtin(name) || packed_builtin(name) || value_builtin(name) != ValueBuiltin::None || atomic_builtin(name).op != AtomicOp::None || builtin_trait(name) != BuiltinTrait::None || function_builtin(name) != FunctionBuiltin::None ||
        integer_builtin(name).operation != IntegerBuiltin::None || fixed_vector_builtin(name).lane_bytes || libm_builtin(name).shape != LibmShape::None || floating_builtin_kind(name) != FloatingBuiltin::None || name.equals("__builtin_va_arg") || integer_pack_builtin(name) || invoke_builtin_name(name);
}
FixedVectorBuiltin fixed_vector_builtin(TextView name)
{
    struct Entry { const char* name; unsigned bytes; bool extract; };
    static const Entry entries[] = {
        {"__builtin_ia32_vec_init_v8qi",1,false},
        {"__builtin_ia32_vec_init_v4hi",2,false},
        {"__builtin_ia32_vec_init_v2si",4,false},
        {"__builtin_ia32_vec_ext_v4hi",2,true},
        {"__builtin_ia32_vec_ext_v2si",4,true},
    };
    FixedVectorBuiltin result;
    for (const auto& entry : entries) if (name.equals(entry.name)) {
        result.lane_bytes = entry.bytes; result.extract = entry.extract; break;
    }
    return result;
}

LibmBuiltin libm_builtin(TextView name)
{
    struct Entry { const char* name; LibmShape shape; };
    using S = LibmShape;
    static const Entry entries[] = {
        {"sqrt",S::Unary},
        {"floor",S::Unary},
        {"ceil",S::Unary},
        {"trunc",S::Unary},
        {"round",S::Unary},
        {"sin",S::Unary},
        {"cos",S::Unary},
        {"tan",S::Unary},
        {"exp",S::Unary},
        {"log",S::Unary},
        {"cbrt",S::Unary},
        {"log2",S::Unary},
        {"log10",S::Unary},
        {"asin",S::Unary},
        {"acos",S::Unary},
        {"atan",S::Unary},
        {"sinh",S::Unary},
        {"cosh",S::Unary},
        {"tanh",S::Unary},
        {"lgamma",S::Unary},
        {"tgamma",S::Unary},
        {"rint",S::Unary},
        {"nearbyint",S::Unary},
        {"logb",S::Unary},
        {"erf",S::Unary},
        {"erfc",S::Unary},
        {"expm1",S::Unary},
        {"log1p",S::Unary},
        {"exp2",S::Unary},
        {"atanh",S::Unary},
        {"asinh",S::Unary},
        {"acosh",S::Unary},
        {"pow",S::Binary},
        {"fmod",S::Binary},
        {"atan2",S::Binary},
        {"hypot",S::Binary},
        {"copysign",S::Binary},
        {"fmax",S::Binary},
        {"fmin",S::Binary},
        {"fdim",S::Binary},
        {"nextafter",S::Binary},
        {"remainder",S::Binary},
        {"fma",S::Ternary},
        {"frexp",S::IntOut},
        {"ldexp",S::IntIn},
        {"scalbn",S::IntIn},
        {"scalbln",S::LongIn},
        {"modf",S::FloatOut},
        {"remquo",S::Quotient},
        {"nexttoward",S::Toward},
        {"ilogb",S::IntResult},
        {"lrint",S::LongResult},
        {"lround",S::LongResult},
        {"llrint",S::LongLongResult},
        {"llround",S::LongLongResult},
        {"cabs",S::ComplexReal}, {"carg",S::ComplexReal},
        {"creal",S::ComplexReal}, {"cimag",S::ComplexReal},
        {"conj",S::ComplexUnary}, {"cproj",S::ComplexUnary},
        {"csqrt",S::ComplexUnary}, {"cexp",S::ComplexUnary}, {"clog",S::ComplexUnary},
        {"csin",S::ComplexUnary}, {"ccos",S::ComplexUnary}, {"ctan",S::ComplexUnary},
        {"casin",S::ComplexUnary}, {"cacos",S::ComplexUnary}, {"catan",S::ComplexUnary},
        {"csinh",S::ComplexUnary}, {"ccosh",S::ComplexUnary}, {"ctanh",S::ComplexUnary},
        {"casinh",S::ComplexUnary}, {"cacosh",S::ComplexUnary}, {"catanh",S::ComplexUnary},
        {"cpow",S::ComplexBinary},
    };
    LibmBuiltin result;
    if (name.size < 11 || std::memcmp(name.data,"__builtin_",10)) return result;
    for (const auto& entry : entries) {
        auto size = std::strlen(entry.name);
        if (name.size != size+10 && name.size != size+11) continue;
        if (std::memcmp(name.data+10,entry.name,size)) continue;
        if (name.size == size+11 && name.data[size+10] != 'f' && name.data[size+10] != 'l') continue;
        result.shape = entry.shape;
        result.suffix = name.size == size+10 ? 0 : name.data[size+10] == 'f' ? 1 : 2;
        return result;
    }
    return result;
}
}

namespace cppgm {
IntegerBuiltinSignature integer_builtin(TextView name)
{
    using I = IntegerBuiltin;
    struct Entry { const char* name; I operation; unsigned width; };
    static const Entry entries[] = {
        {"__builtin_clz",I::Clz,32},
        {"__builtin_clzl",I::Clz,1},
        {"__builtin_clzll",I::Clz,64},
        {"__builtin_ctz",I::Ctz,32},
        {"__builtin_ctzl",I::Ctz,1},
        {"__builtin_ctzll",I::Ctz,64},
        {"__builtin_popcount",I::Popcount,32},
        {"__builtin_popcountl",I::Popcount,1},
        {"__builtin_popcountll",I::Popcount,64},
        {"__builtin_parity",I::Parity,32},
        {"__builtin_parityl",I::Parity,1},
        {"__builtin_parityll",I::Parity,64},
        {"__builtin_ffs",I::Ffs,32},
        {"__builtin_ffsl",I::Ffs,1},
        {"__builtin_ffsll",I::Ffs,64},
        {"__builtin_clzg",I::Clz,0},
        {"__builtin_ctzg",I::Ctz,0},
        {"__builtin_popcountg",I::Popcount,0},
        {"__builtin_bswap16",I::Bswap,16},
        {"__builtin_bswap32",I::Bswap,32},
        {"__builtin_bswap64",I::Bswap,64},
    };
    IntegerBuiltinSignature result;
    for (const auto& entry : entries) if (name.equals(entry.name)) {
        result.operation = entry.operation; result.width = entry.width; break;
    }
    return result;
}
}
