# Per-tool implementation source lists for the compiler.
#
# Add dev/src/foo.cpp to the tools that use it by adding `foo` below. For
# subdirectories, use the path without `.cpp`, such as `parser/foo`.

FRONTEND_SOURCE_SET_TARGETS := abimangle pptoken posttoken ppexpr preproc cppgm++ lowiropt lowir lowir2native
FRONTEND_TEST_RUNNER_SOURCE_ID := support/testing/test_runner

FRONTEND_OBJ_BASENAMES_abimangle := preprocess/source preprocess/identifier_table abi/itanium/graph abi/itanium/graph_validation abi/itanium/vocabulary abi/itanium/encoder abi/itanium/function_encoder abi/itanium/expression_encoder abi/itanium/fact_reader abi/itanium/type_reader abi/itanium/function_reader abi/itanium/argument_reader abi/itanium/fact_writer
FRONTEND_OBJ_BASENAMES_pptoken := preprocess/source preprocess/identifier_table preprocess/token_cursor preprocess/token_output
FRONTEND_OBJ_BASENAMES_posttoken := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal posttoken/cursor posttoken/output
FRONTEND_OBJ_BASENAMES_ppexpr := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal preprocess/expression_value preprocess/expression
FRONTEND_OBJ_BASENAMES_preproc := preprocess/source preprocess/identifier_table preprocess/token_cursor posttoken/token_types posttoken/number posttoken/literal posttoken/cursor posttoken/output preprocess/expression_value preprocess/expression preprocess/preprocessor preprocess/macro preprocess/directive
FRONTEND_OBJ_BASENAMES_cppgm++ := $(FRONTEND_OBJ_BASENAMES_preproc) syntax/ast syntax/cursor syntax/names syntax/parser syntax/prediction syntax/name_parser syntax/declarator syntax/expression syntax/output syntax/statement syntax/declaration syntax/class_parser syntax/template_parser syntax/driver semantic/model semantic/lookup semantic/type_builder semantic/declaration semantic/class_enum semantic/constant semantic/output semantic/overload semantic/conversion semantic/expression semantic/operators semantic/statement semantic/resolved_output semantic/member semantic/template_call
FRONTEND_OBJ_BASENAMES_lowiropt :=
FRONTEND_OBJ_BASENAMES_lowir := preprocess/source preprocess/identifier_table lowir/model lowir/opcode lowir/instruction_shape lowir/reader lowir/metadata lowir/metadata_vocabulary lowir/top_level lowir/instruction_reader lowir/writer lowir/instruction_writer lowir/signature_validation lowir/validator lowir/validation_values lowir/validation_instructions lowir/exercises
FRONTEND_OBJ_BASENAMES_lowir2native :=

# Source lowering uses typed PA8 construction and PA9 encoding only.
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/default_arguments semantic/jump_validation semantic/static_initializer lowering/driver lowering/symbols lowering/values lowering/expression lowering/control_flow lowering/initialization lowir/model lowir/opcode lowir/instruction_shape lowir/writer lowir/instruction_writer lowir/metadata_vocabulary abi/itanium/graph abi/itanium/graph_validation abi/itanium/vocabulary abi/itanium/encoder abi/itanium/function_encoder abi/itanium/expression_encoder

FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/signature_validation lowir/validator lowir/validation_values lowir/validation_instructions

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/construction lowering/construction
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/inherited_constructors
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/converting_constructors
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/literal_calls
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/floating_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/placement_new lowering/placement_new
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/array_allocation
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/tls_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_construction lowering/constant_construction
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/global_initialization

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/destruction semantic/access semantic/friends lowering/destruction lowering/arrays lowering/cleanup
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/operator_names semantic/operator_call lowering/operator_abi
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/layout semantic/fields
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/bit_fields

FRONTEND_OBJ_BASENAMES_cppgm++ += support/id_index
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/initializers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/value_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/static_bit_fields
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/initializer_plan
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/aggregate_helpers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/special_members semantic/transfer_actions lowering/transfers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/class_values lowering/class_values lowering/branch_lifetimes
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/conversion_functions lowering/user_conversions
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/builtin_operators
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/reference_storage lowering/reference_storage
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/allocation lowering/deallocation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/list_initialization lowering/list_initialization lowering/full_expression
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/member_pointers lowering/member_pointers
