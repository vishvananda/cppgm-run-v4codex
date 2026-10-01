#pragma once
#include "preprocess/source.h"
namespace cppgm {
inline bool integer_pack_builtin(TextView name) { return name.equals("__integer_pack"); }
// Bounded immutable vocabulary shared by builtin construction and probes.
enum class FunctionBuiltin : unsigned char {
    None, Strcmp, Strncmp, VaStart, VaEnd, VaCopy,
    Alloca, Expect, Abort, Unreachable, Vsnprintf, Vsprintf,
    Fabs, Fabsf, Fabsl, Abs, Labs, Llabs, Memcpy, Memmove, Memset, Memcmp, Memchr, Strlen, Strchr, Strrchr, Bzero, Strstr, Strpbrk, Prefetch, AssumeAligned, FltRounds, AddOverflow, SubOverflow, MulOverflow, OperatorNew, OperatorDelete
};
inline bool invoke_builtin_name(TextView name) { return name.equals("__builtin_invoke"); }
inline FunctionBuiltin function_builtin(TextView name)
{
    static const char* const names[] = {"",
        "__builtin_strcmp", "__builtin_strncmp", "__builtin_va_start", "__builtin_va_end",
        "__builtin_va_copy", "__builtin_alloca", "__builtin_expect", "__builtin_abort",
        "__builtin_unreachable", "__builtin_vsnprintf", "__builtin_vsprintf",
        "__builtin_fabs", "__builtin_fabsf", "__builtin_fabsl", "__builtin_abs",
        "__builtin_labs", "__builtin_llabs", "__builtin_memcpy", "__builtin_memmove",
        "__builtin_memset", "__builtin_memcmp", "__builtin_memchr", "__builtin_strlen",
        "__builtin_strchr", "__builtin_strrchr", "__builtin_bzero", "__builtin_strstr", "__builtin_strpbrk", "__builtin_prefetch", "__builtin_assume_aligned", "__builtin_flt_rounds", "__builtin_add_overflow", "__builtin_sub_overflow", "__builtin_mul_overflow", "__builtin_operator_new", "__builtin_operator_delete"};
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (name.equals(names[i])) return FunctionBuiltin(i);
    return FunctionBuiltin::None;
}
enum class LibmShape : unsigned char { None, Unary, Binary, Ternary, IntOut, IntIn, LongIn, FloatOut, Quotient, Toward, IntResult, LongResult, LongLongResult };
struct LibmBuiltin { LibmShape shape = LibmShape::None; unsigned suffix = 0; };
LibmBuiltin libm_builtin(TextView name);
enum class IntegerBuiltin : unsigned char { None, Clz, Ctz, Popcount, Parity, Ffs, Bswap };
struct IntegerBuiltinSignature {
    IntegerBuiltin operation = IntegerBuiltin::None;
    unsigned width = 0; // 0 means generic unpromoted unsigned input; 1 means unsigned long.
};
IntegerBuiltinSignature integer_builtin(TextView name);
enum class FloatingBuiltin : unsigned char {
    None, Nan, Nanf, Nanl, Inf, Inff, Infl, Huge, Hugef, Hugel,
    Finite, IsNan, IsNanf, IsNanl, Infinite, Normal, Signbit, Signbitf, Signbitl, Classify, Nans, Nansf, Nansl, Greater, GreaterEqual, Less, LessEqual, LessGreater, Unordered
};
inline FloatingBuiltin floating_builtin_kind(TextView name)
{
    static const char* const names[] = {"", "__builtin_nan", "__builtin_nanf", "__builtin_nanl",
        "__builtin_inf", "__builtin_inff", "__builtin_infl", "__builtin_huge_val",
        "__builtin_huge_valf", "__builtin_huge_vall", "__builtin_isfinite", "__builtin_isnan",
        "__builtin_isnanf", "__builtin_isnanl", "__builtin_isinf", "__builtin_isnormal", "__builtin_signbit", "__builtin_signbitf", "__builtin_signbitl", "__builtin_fpclassify", "__builtin_nans", "__builtin_nansf", "__builtin_nansl", "__builtin_isgreater", "__builtin_isgreaterequal", "__builtin_isless", "__builtin_islessequal", "__builtin_islessgreater", "__builtin_isunordered"};
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (name.equals(names[i])) return FloatingBuiltin(i);
    return FloatingBuiltin::None;
}
}
