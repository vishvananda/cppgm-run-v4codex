#pragma once
#include "preprocess/source.h"
namespace cppgm {
enum class BuiltinTrait : unsigned char {
    None, Enum, Union, Class, Trivial, TriviallyCopyable, StandardLayout, Pod,
    Literal, Empty, Polymorphic, Final, Abstract, Constructible, NothrowConstructible,
    TriviallyConstructible, Assignable, NothrowAssignable, TriviallyAssignable,
    TrivialDestructor, VirtualDestructor, Same, BaseOf, Underlying
};
inline BuiltinTrait builtin_trait(TextView text)
{
    static const char* const names[] = {"", "__is_enum", "__is_union", "__is_class", "__is_trivial",
        "__is_trivially_copyable", "__is_standard_layout", "__is_pod", "__is_literal_type",
        "__is_empty", "__is_polymorphic", "__is_final", "__is_abstract", "__is_constructible",
        "__is_nothrow_constructible", "__is_trivially_constructible", "__is_assignable",
        "__is_nothrow_assignable", "__is_trivially_assignable", "__has_trivial_destructor",
        "__has_virtual_destructor", "__is_same", "__is_base_of", "__underlying_type"};
    if (text.size < 5 || text.data[0] != '_' || text.data[1] != '_') return BuiltinTrait::None;
    for (unsigned i = 1; i < sizeof(names)/sizeof(*names); ++i)
        if (text.equals(names[i])) return BuiltinTrait(i);
    return BuiltinTrait::None;
}
}
