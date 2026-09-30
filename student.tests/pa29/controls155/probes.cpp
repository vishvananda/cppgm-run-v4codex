#ifndef __has_builtin
#error absent builtin probe
#endif
#if !__has_builtin(__is_same) || !__has_builtin(__remove_reference_t) || !__has_builtin(__builtin_fabs)
#error implemented builtin not advertised
#endif
#if __has_builtin(__not_a_builtin) || __has_feature(modules) || __has_extension(blocks)
#error unsupported capability advertised
#endif
#if __has_cpp_attribute(vendor::attribute) || __has_warning("-Whypothetical") || __has_declspec_attribute(dllexport)
#error unsupported guarantee
#endif
#if !__has_feature(__cxx_binary_literals__) || !__has_extension(cxx_variadic_templates)
#error missing existing language feature
#endif
_Pragma("GCC diagnostic ignored \"-Wunknown-pragmas\"")
#warning hosted warning
int main() { return 0; }
