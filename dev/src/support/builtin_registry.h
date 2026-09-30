#pragma once
#include "preprocess/source.h"
namespace cppgm {
inline bool integer_pack_builtin(TextView name) { return name.equals("__integer_pack"); }
// Bounded immutable vocabulary shared by builtin construction and probes.
enum class FunctionBuiltin : unsigned char {
    None, AtomicFetchAdd, AtomicAddFetch, Strcmp, Strncmp, VaStart, VaEnd, VaCopy,
    Alloca, Expect, Abort, Unreachable, Vsnprintf, Vsprintf,
    Fabs, Fabsf, Fabsl, Abs, Labs, Llabs
};
inline FunctionBuiltin function_builtin(TextView name)
{
    static const char* const names[] = {"", "__atomic_fetch_add", "__atomic_add_fetch",
        "__builtin_strcmp", "__builtin_strncmp", "__builtin_va_start", "__builtin_va_end",
        "__builtin_va_copy", "__builtin_alloca", "__builtin_expect", "__builtin_abort",
        "__builtin_unreachable", "__builtin_vsnprintf", "__builtin_vsprintf",
        "__builtin_fabs", "__builtin_fabsf", "__builtin_fabsl", "__builtin_abs",
        "__builtin_labs", "__builtin_llabs"};
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (name.equals(names[i])) return FunctionBuiltin(i);
    return FunctionBuiltin::None;
}
enum class FloatingBuiltin : unsigned char {
    None, Nan, Nanf, Nanl, Inf, Inff, Infl, Huge, Hugef, Hugel,
    Finite, IsNan, IsNanf, IsNanl, Infinite, Normal, Classify
};
inline FloatingBuiltin floating_builtin_kind(TextView name)
{
    static const char* const names[] = {"", "__builtin_nan", "__builtin_nanf", "__builtin_nanl",
        "__builtin_inf", "__builtin_inff", "__builtin_infl", "__builtin_huge_val",
        "__builtin_huge_valf", "__builtin_huge_vall", "__builtin_isfinite", "__builtin_isnan",
        "__builtin_isnanf", "__builtin_isnanl", "__builtin_isinf", "__builtin_isnormal", "__builtin_fpclassify"};
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (name.equals(names[i])) return FloatingBuiltin(i);
    return FloatingBuiltin::None;
}
}
