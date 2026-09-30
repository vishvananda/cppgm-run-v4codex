#pragma once
#include "support/type_traits.h"
#include "support/builtin_registry.h"
namespace cppgm {
inline bool hosted_builtin(TextView name)
{
    return builtin_trait(name) != BuiltinTrait::None || function_builtin(name) != FunctionBuiltin::None ||
        integer_builtin(name).operation != IntegerBuiltin::None || libm_builtin(name).shape != LibmShape::None || floating_builtin_kind(name) != FloatingBuiltin::None || name.equals("__builtin_va_arg") || integer_pack_builtin(name);
}
inline bool hosted_feature(TextView name, bool exceptions)
{
    if (name.size > 4 && name.data[0] == '_' && name.data[1] == '_' &&
        name.data[name.size-2] == '_' && name.data[name.size-1] == '_')
        name = TextView(name.data+2,name.size-4);
    if (name.equals("cxx_exceptions")) return exceptions;
    if (name.equals("cxx_rtti")) return true;
    static const char* const features[] = {"cxx_alignas", "cxx_alignof", "cxx_auto_type",
        "cxx_binary_literals", "cxx_constexpr", "cxx_alias_templates", "cxx_default_function_template_args",
        "cxx_explicit_conversions", "cxx_generalized_initializers", "cxx_lambdas", "cxx_local_type_template_args",
        "cxx_noexcept", "cxx_nullptr", "cxx_range_for", "cxx_raw_string_literals", "cxx_strong_enums",
        "cxx_unicode_literals", "cxx_decltype", "cxx_decltype_incomplete_return_types", "cxx_defaulted_functions",
        "cxx_deleted_functions", "cxx_inline_namespaces", "cxx_override_control", "cxx_reference_qualified_functions",
        "cxx_rvalue_references", "cxx_static_assert", "cxx_trailing_return", "cxx_unrestricted_unions",
        "cxx_user_literals", "cxx_variable_templates", "cxx_variadic_templates"};
    for (auto feature : features) if (name.equals(feature)) return true;
    std::string trait = "__" + std::string(name.data,name.size);
    return builtin_trait(TextView(trait.data(),trait.size())) != BuiltinTrait::None;
}
}
