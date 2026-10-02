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
FRONTEND_OBJ_BASENAMES_cppgm++ := $(FRONTEND_OBJ_BASENAMES_preproc) syntax/ast syntax/cursor syntax/names syntax/parser syntax/attributes syntax/prediction syntax/name_parser syntax/declarator syntax/expression syntax/output syntax/statement syntax/declaration syntax/class_parser syntax/template_parser syntax/driver semantic/model semantic/lookup semantic/type_builder semantic/native_attributes semantic/declaration semantic/class_enum semantic/nested_class semantic/constant semantic/constant_integer semantic/output semantic/overload semantic/conversion semantic/expression semantic/operators semantic/statement semantic/resolved_output semantic/member semantic/template_call
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_lexical_frame semantic/template_definition_environment
FRONTEND_OBJ_BASENAMES_cppgm++ += syntax/deduction_guide semantic/deduction_guide
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/preprocess_output
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_address_arguments semantic/template_member_arguments
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/query_list
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/initializer_list
FRONTEND_OBJ_BASENAMES_lowiropt :=
FRONTEND_OBJ_BASENAMES_lowir := preprocess/source preprocess/identifier_table lowir/model lowir/opcode lowir/instruction_shape lowir/reader lowir/metadata lowir/metadata_vocabulary lowir/top_level lowir/instruction_reader lowir/writer lowir/instruction_writer lowir/signature_validation lowir/validator lowir/validation_values lowir/validation_instructions lowir/exercises
FRONTEND_OBJ_BASENAMES_lowir2native :=

# Source lowering uses typed PA8 construction and PA9 encoding only.
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/default_arguments semantic/jump_validation semantic/static_initializer lowering/driver lowering/symbols lowering/values lowering/expression lowering/control_flow lowering/initialization lowir/model lowir/opcode lowir/instruction_shape lowir/writer lowir/instruction_writer lowir/metadata_vocabulary abi/itanium/graph abi/itanium/graph_validation abi/itanium/vocabulary abi/itanium/encoder abi/itanium/function_encoder abi/itanium/expression_encoder

FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/signature_validation lowir/validator lowir/validation_values lowir/validation_instructions

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/construction lowering/construction
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/inherited_constructors
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/inherited_forwarding lowering/inherited_forwarding
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_array lowering/constant_array
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/converting_constructors
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/literal_calls semantic/numeric_literal_calls semantic/literal_constants lowering/literal_calls
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/floating_builtins lowering/floating_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/statement_expression lowering/statement_expression
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/placement_new lowering/placement_new
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/array_allocation
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/allocation_cleanup
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/allocation_cleanup
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/tls_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_construction semantic/constant_storage_construction lowering/constant_construction
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/field_projection
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/global_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/static_initialization lowering/local_static

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/destruction semantic/access semantic/friends lowering/destruction lowering/arrays lowering/cleanup
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/operator_names semantic/operator_call lowering/operator_abi
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/layout semantic/empty_layout semantic/fields
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/bit_fields

FRONTEND_OBJ_BASENAMES_cppgm++ += support/id_index
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/initializers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/value_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/static_bit_fields
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/initializer_plan
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/aggregate_helpers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/special_members semantic/transfer_actions lowering/transfers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/class_values lowering/class_values lowering/branch_lifetimes
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/closure_conversion
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/lambda
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/lambda
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/placeholder
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/placeholder_return
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/initializer_effects
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/range_statement semantic/range_operations lowering/range_statement lowering/typed_operations
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/structured_binding
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_array_copy lowering/array_copy
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/conversion_functions lowering/user_conversions
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/builtin_operators
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/assignment_operators
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/query_destructor
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/reference_storage lowering/reference_storage
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/allocation lowering/deallocation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/list_initialization lowering/list_initialization lowering/full_expression
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/member_pointers semantic/member_pointer_values lowering/member_pointers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/member_pointer_flow
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/access_paths
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/zero_initialization lowering/zero_initialization
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/scalar_transfer semantic/parameter_representation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/scalar_consumption lowering/scalar_consumption
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/virtuals lowering/virtuals
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/virtual_subobjects semantic/virtual_primary semantic/virtual_storage
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/virtual_views
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/local_abi lowering/standard_abi
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/lifecycle_order lowering/program_lifecycle
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/destructor_exception
FRONTEND_OBJ_BASENAMES_cppgm++ += syntax/occurrence semantic/template_instantiation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_declaration
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_specialization
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_variable
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_packs semantic/template_pack_expansion semantic/query_new
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_pack_recipe
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/fold semantic/fold_expression semantic/fold_constant lowering/fold
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/fold_query_value
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_class
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/class_pattern_selection semantic/template_deduction semantic/template_ordering
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/deduction_parameters
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/dependent_type
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/angle_interpretation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_definition
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_binding semantic/template_binding_declarations semantic/template_binding_statements semantic/template_checks
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_expression
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/expression_store
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/fact_store
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_call_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_conversion_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_object_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/member_value
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_member_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_type_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/query_dependencies semantic/type_query semantic/query_call semantic/query_cast semantic/callable semantic/invoke
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_value_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/call_selection
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/query_operator
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/candidate_substitution
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/member_receiver
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/deferred_function_uses
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/query_abi
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/explicit_instantiation
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_statement_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_initializers
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_operator_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/default_initialization_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/default_destructor_facts

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_arguments semantic/template_entities
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_signature_shape
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/exception_query
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_objects semantic/constant_addresses semantic/constant_execution
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_queries
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/explicit_conversion semantic/discarded
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/arrow lowering/arrow
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_statements semantic/constant_overlay
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constant_floating
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/constexpr_validity
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/exception_expression
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/exception_specification
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/dynamic_exceptions
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/exception_override
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_type_access

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_definition_owner

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/explicit_specifier
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/conversion_result
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/template_conversion_deduction

FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/function_declaration
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/lifecycle_layout lowering/lifecycle_abi lowering/construction_tables
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/parameter_abi
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/closure_capture
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/fallthrough lowering/flow_constants lowering/flow_regions lowering/flow_reachability
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/rtti lowering/rtti
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/rtti_facts
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/source_exception lowering/source_exception
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/exception_handlers
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/exception_boundary
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/aggregate_lifetimes
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/exception_continuation

FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/member_thunks
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/construction_vtables

FRONTEND_OBJ_BASENAMES_lowir2native := $(filter-out lowir/exercises,$(FRONTEND_OBJ_BASENAMES_lowir))
FRONTEND_OBJ_BASENAMES_lowir2native += native/model native/placement native/selection native/arithmetic native/control native/calls native/encoding native/integer_encoding native/layout native/elf native/dump native/driver
FRONTEND_OBJ_BASENAMES_lowir2native += native/parameter_slots
FRONTEND_OBJ_BASENAMES_lowir2native += native/object_demand
FRONTEND_OBJ_BASENAMES_lowir2native += native/bulk_encoding
FRONTEND_OBJ_BASENAMES_lowir2native += native/builtins
FRONTEND_OBJ_BASENAMES_lowir2native += native/floating native/float_encoding native/float_conversion
FRONTEND_OBJ_BASENAMES_lowir2native += native/variadic
FRONTEND_OBJ_BASENAMES_lowir2native += native/carry
FRONTEND_OBJ_BASENAMES_lowir2native += native/parameter_flow

FRONTEND_OBJ_BASENAMES_lowir2native += native/fragments
FRONTEND_OBJ_BASENAMES_lowir2native += native/wide

FRONTEND_OBJ_BASENAMES_lowir2native += native/wide_shift
FRONTEND_OBJ_BASENAMES_lowir2native += native/wide_division
FRONTEND_OBJ_BASENAMES_lowir2native += native/wide_float_encoding
FRONTEND_OBJ_BASENAMES_lowir2native += native/tls native/runtime native/runtime_encoding

FRONTEND_OBJ_BASENAMES_lowir2native += native/host_regions native/host_encoding native/host_tables

FRONTEND_OBJ_BASENAMES_cppgm++ += $(filter native/%,$(FRONTEND_OBJ_BASENAMES_lowir2native)) toolchain/object toolchain/linker toolchain/elf_input toolchain/driver
FRONTEND_OBJ_BASENAMES_cppgm++ += native/process_runtime toolchain/runtime toolchain/runtime_builder toolchain/runtime_cast
FRONTEND_OBJ_BASENAMES_lowir2native += native/exception_clauses
FRONTEND_OBJ_BASENAMES_cppgm++ += native/exception_clauses
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/runtime_exception
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/runtime_eh_match toolchain/runtime_failure

FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/host_elf toolchain/host_unwind
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/host_sections
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/host_config
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/elf_link_input
FRONTEND_OBJ_BASENAMES_cppgm++ += toolchain/dynamic_symbols toolchain/dynamic_relocations toolchain/dynamic_metadata toolchain/dynamic_writer
FRONTEND_OBJ_BASENAMES_lowir2native += support/id_index

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/builtin_traits
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/builtin_template_types

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/builtin_functions
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/function_context
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/intrinsics

FRONTEND_OBJ_BASENAMES_preproc += support/builtin_registry
FRONTEND_OBJ_BASENAMES_cppgm++ += support/builtin_registry semantic/runtime_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/integer_builtins lowering/integer_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/hint_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/overflow_builtins lowering/overflow_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/floating_constants

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/offsetof lowering/offsetof
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/storage_types

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/legacy_traits semantic/reference_traits

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/atomic_builtins lowering/atomic_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/atomic_storage
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/atomic_runtime

FRONTEND_OBJ_BASENAMES_cppgm++ += syntax/assembly syntax/assembly_template semantic/assembly lowering/assembly

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/evaluation_context lowering/context_initialization

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/bit_integer lowering/bit_integer
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/complex lowering/complex
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/vector_types semantic/value_builtins lowering/vector_values
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/inline_validation

FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/source_builtins lowering/source_strings
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/object_initializer
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/inline_variable

FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/force_inline
FRONTEND_OBJ_BASENAMES_lowir2native += lowir/force_inline

# Exact binary16/binary128 literal and constant representations.
$(foreach tool,posttoken ppexpr preproc cppgm++ lowir lowiropt lowir2native,$(eval FRONTEND_OBJ_BASENAMES_$(tool) += support/extended_float))
$(foreach tool,cppgm++ lowir2native,$(eval FRONTEND_OBJ_BASENAMES_$(tool) += native/extended_float))

FRONTEND_OBJ_BASENAMES_preproc += support/packed_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += support/packed_builtins lowering/packed_builtins

FRONTEND_OBJ_BASENAMES_preproc += support/x86_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += support/x86_builtins lowering/x86_builtins native/x86_builtins
FRONTEND_OBJ_BASENAMES_lowir += support/x86_builtins
FRONTEND_OBJ_BASENAMES_lowir2native += support/x86_builtins native/x86_builtins
FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/x86_lanes
FRONTEND_OBJ_BASENAMES_cppgm++ += semantic/vector_shuffle lowering/vector_shuffle

# PA32 shared typed optimizer and explicit LowIR object input.
FRONTEND_OBJ_BASENAMES_lowiropt := $(filter-out lowir/exercises,$(FRONTEND_OBJ_BASENAMES_lowir)) lowir/optimizer
FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/reader lowir/metadata lowir/top_level lowir/instruction_reader lowir/optimizer

FRONTEND_OBJ_BASENAMES_lowiropt += support/id_index lowir/folding lowir/scalar_simplify
FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/folding lowir/scalar_simplify

FRONTEND_OBJ_BASENAMES_lowiropt += lowir/control_simplify
FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/control_simplify

FRONTEND_OBJ_BASENAMES_lowiropt += lowir/slot_forwarding
FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/slot_forwarding

FRONTEND_OBJ_BASENAMES_lowiropt += lowir/local_cse
FRONTEND_OBJ_BASENAMES_cppgm++ += lowir/local_cse

FRONTEND_OBJ_BASENAMES_cppgm++ += lowering/debug_location
