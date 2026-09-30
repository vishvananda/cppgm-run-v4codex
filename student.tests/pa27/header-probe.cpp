// The harness supplies two search directories with the same physical names.
#define FILE_NAME "first.h"
#define LOOKUP(x) __has_include(x)
#define first a_macro_that_must_not_expand_inside_a_header_name
#if !defined(__has_include) || !defined(__has_include_next)
#error missing header probe
#endif
#if !__has_include(<first.h>) || !__has_include(FILE_NAME) || !LOOKUP(FILE_NAME)
#error cannot find present header
#endif
#if __has_include(<missing-149.h>) || __has_include("missing-149.h")
#error found an absent header
#endif
#include "first.h"
#ifndef FIRST_INCLUDED
#error first header absent
#endif
#ifndef SECOND_INCLUDED
#error next header absent
#endif
int main() { return FIRST_INCLUDED+SECOND_INCLUDED-42; }
