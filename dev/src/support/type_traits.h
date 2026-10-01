#pragma once
#include "preprocess/source.h"
namespace cppgm {
enum class BuiltinTrait : unsigned char {
    None, Enum, Union, Class, Trivial, TriviallyCopyable, StandardLayout, Pod,
    Literal, Empty, Polymorphic, Final, Abstract, Constructible, NothrowConstructible,
    TriviallyConstructible, Assignable, NothrowAssignable, TriviallyAssignable,
    TrivialDestructor, VirtualDestructor, Same, BaseOf, Underlying, Decay,
    Const, Volatile, Void, Array, BoundedArray, UnboundedArray, LvalueReference, RvalueReference, Reference,
        Pointer, Function, Object, Integral, Floating, Arithmetic, Fundamental, Compound, Referenceable, Signed,
        Unsigned, Scalar, MemberPointer, MemberObjectPointer, MemberFunctionPointer,
    ArrayRank, Offsetof, Aggregate, Destructible, TriviallyDestructible, NothrowDestructible, Convertible, NothrowConvertible,
    RemoveCV, RemoveConst, RemoveVolatile, RemoveReference, RemoveCVRef, RemovePointer, RemoveExtent,
        RemoveAllExtents, AddPointer, AddLRef, AddRRef, MakeSigned, MakeUnsigned
};
inline bool type_transform(BuiltinTrait trait)
{
    return trait == BuiltinTrait::Underlying || trait == BuiltinTrait::Decay || trait >= BuiltinTrait::RemoveCV;
}
inline BuiltinTrait builtin_trait(TextView text)
{
    static const char* const names[] = {"", "__is_enum", "__is_union", "__is_class", "__is_trivial",
        "__is_trivially_copyable", "__is_standard_layout", "__is_pod", "__is_literal_type",
        "__is_empty", "__is_polymorphic", "__is_final", "__is_abstract", "__is_constructible",
        "__is_nothrow_constructible", "__is_trivially_constructible", "__is_assignable",
        "__is_nothrow_assignable", "__is_trivially_assignable", "__has_trivial_destructor",
        "__has_virtual_destructor", "__is_same", "__is_base_of", "__underlying_type", "__decay",
        "__is_const", "__is_volatile", "__is_void", "__is_array", "__is_bounded_array", "__is_unbounded_array",
        "__is_lvalue_reference", "__is_rvalue_reference", "__is_reference", "__is_pointer", "__is_function",
        "__is_object", "__is_integral", "__is_floating_point", "__is_arithmetic", "__is_fundamental",
        "__is_compound", "__is_referenceable", "__is_signed", "__is_unsigned", "__is_scalar",
        "__is_member_pointer", "__is_member_object_pointer", "__is_member_function_pointer",
        "__array_rank", "__builtin_offsetof", "__is_aggregate", "__is_destructible", "__is_trivially_destructible", "__is_nothrow_destructible",
        "__is_convertible", "__is_nothrow_convertible", "__remove_cv",
        "__remove_const", "__remove_volatile", "__remove_reference_t", "__remove_cvref", "__remove_pointer",
        "__remove_extent", "__remove_all_extents", "__add_pointer", "__add_lvalue_reference",
        "__add_rvalue_reference", "__make_signed", "__make_unsigned"};
    static_assert(sizeof(names)/sizeof(*names) == unsigned(BuiltinTrait::MakeUnsigned)+1, "trait registry size");
    if (text.equals("__remove_reference")) return BuiltinTrait::RemoveReference;
    if (text.equals("__decay_t")) return BuiltinTrait::Decay;
    if (text.equals("__is_convertible_to")) return BuiltinTrait::Convertible;
    if (text.equals("__is_literal")) return BuiltinTrait::Literal;
    if (text.size < 5 || text.data[0] != '_' || text.data[1] != '_') return BuiltinTrait::None;
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (text.equals(names[i])) return BuiltinTrait(i);
    return BuiltinTrait::None;
}
}
