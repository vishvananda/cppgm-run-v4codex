# Per-tool implementation source lists for the compiler.
#
# Add dev/src/foo.cpp to the tools that use it by adding `foo` below. For
# subdirectories, use the path without `.cpp`, such as `parser/foo`.

FRONTEND_SOURCE_SET_TARGETS := abimangle pptoken posttoken ppexpr preproc cppgm++ lowiropt lowir lowir2native
FRONTEND_TEST_RUNNER_SOURCE_ID := support/testing/test_runner

FRONTEND_OBJ_BASENAMES_abimangle :=
FRONTEND_OBJ_BASENAMES_pptoken := preprocess/source preprocess/identifier_table preprocess/token_cursor preprocess/token_output
FRONTEND_OBJ_BASENAMES_posttoken := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal posttoken/cursor posttoken/output
FRONTEND_OBJ_BASENAMES_ppexpr := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal preprocess/expression_value preprocess/expression
FRONTEND_OBJ_BASENAMES_preproc := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal posttoken/cursor posttoken/output preprocess/expression_value preprocess/expression preprocess/preprocessor preprocess/macro preprocess/directive
FRONTEND_OBJ_BASENAMES_cppgm++ := $(FRONTEND_OBJ_BASENAMES_preproc) syntax/ast syntax/cursor syntax/names syntax/parser syntax/prediction syntax/name_parser syntax/declarator syntax/expression syntax/output syntax/statement syntax/declaration syntax/class_parser syntax/template_parser syntax/driver semantic/model semantic/lookup semantic/type_builder semantic/declaration semantic/class_enum semantic/constant semantic/output semantic/overload semantic/conversion semantic/expression semantic/operators semantic/statement semantic/resolved_output
FRONTEND_OBJ_BASENAMES_lowiropt :=
FRONTEND_OBJ_BASENAMES_lowir :=
FRONTEND_OBJ_BASENAMES_lowir2native :=
