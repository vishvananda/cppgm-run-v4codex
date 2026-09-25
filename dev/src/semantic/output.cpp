#include "semantic/analyzer.h"
#include <ostream>
#include <stdexcept>

namespace cppgm { namespace semantic {
namespace {
const char* binding_name(EntityKind k) {
    switch (k) {
    case EntityKind::Type: return "type";
    case EntityKind::Alias: return "type-alias";
    case EntityKind::Variable: return "variable";
    case EntityKind::Function: return "function";
    case EntityKind::Parameter: return "parameter";
    case EntityKind::Enumerator: return "enumerator";
    default: throw std::logic_error("non-renderable binding");
    }
}
void indent(std::ostream& out, unsigned depth) { for (unsigned i = 0; i < depth; ++i) out << "  "; }
const char* keyword(ETokenType k) {
    switch (k) {
    case KW_STRUCT: return "struct";
    case KW_CLASS: return "class";
    case KW_UNION: return "union";
    case KW_ENUM: return "enum";
    case KW_TYPENAME: return "typename";
    case KW_TEMPLATE: return "template-parameter";
    default: throw std::logic_error("invalid named type key");
    }
}
}
void Analyzer::spelling(std::ostream& out, IdentifierId n) const
{
    if (!n) return;
    TextView v = ids.spelling(n); out.write(v.data, v.size);
}
void Analyzer::write_name(std::ostream& out, NodeId n) const
{
    if (ast[n].op == OP_COLON2) out << "::";
    for (NodeId p = ast[n].first; p; p = ast[p].next) {
        if (p != ast[n].first) out << "::";
        spelling(out, ast[p].text);
    }
}
void Analyzer::write_type(std::ostream& out, TypeId id, NodeId display_name, ETokenType key_op, TypeId completed) const
{
    const Type& t = types[id];
    if (t.kind != TypeKind::Function && (t.cv & 1)) out << "const ";
    if (t.kind != TypeKind::Function && (t.cv & 2)) out << "volatile ";
    switch (t.kind) {
    case TypeKind::AliasApplication: write_type(out,t.child); break;
    case TypeKind::Fundamental: out << fundamental_name(t.fundamental); break;
    case TypeKind::Decltype: out << "dependent decltype query " << t.entity; break;
    case TypeKind::DependentArray: out << "array bound query " << t.bound << " of "; write_type(out,t.child); break;
    case TypeKind::DependentName:
        write_type(out,t.child); out << "::"; spelling(out,t.entity); break;
    case TypeKind::Named: {
        const Entity& e = entities[t.entity];
        out << keyword(key_op == TOK_INVALID ? e.key : key_op);
        if (e.scoped) out << " class";
        out << ' ';
        if (display_name) write_name(out, display_name);
        else if (calls && e.key != KW_ENUM) write_entity_name(out, t.entity);
        else spelling(out, e.name);
        break;
    }
    case TypeKind::MemberPointer:
        out << "member-pointer of "; write_type(out, entities[t.entity].type); out << " to "; write_type(out, t.child); break;
    case TypeKind::Pointer: out << "pointer to "; write_type(out, t.child); break;
    case TypeKind::LRef: out << "lvalue-reference to "; write_type(out, t.child); break;
    case TypeKind::RRef: out << "rvalue-reference to "; write_type(out, t.child); break;
    case TypeKind::Array:
        out << "array of " << (completed ? types[completed].bound : t.bound) << ' ';
        write_type(out, t.child, 0, TOK_INVALID, completed ? types[completed].child : 0); break;
    case TypeKind::Function:
        out << "function of (";
        for (unsigned i = 0; i < t.count; ++i) {
            if (i) out << ", ";
            write_type(out, types.parameters[t.offset + i]);
        }
        if (t.variadic) out << (t.count ? ", ..." : "...");
        out << ")";
        if (t.cv & 1) out << " const";
        if (t.cv & 2) out << " volatile";
        out << " returning "; write_type(out, t.child); break;
    }
}
void Analyzer::write_scope(std::ostream& out, ScopeId s, unsigned depth) const
{
    const Scope& scope = scopes[s];
    indent(out, depth); out << "scope ";
    switch (scope.kind) {
    case ScopeKind::Namespace: out << "namespace "; break;
    case ScopeKind::Class: out << "class "; break;
    case ScopeKind::Enum: out << "enum "; break;
    case ScopeKind::Function: out << "function "; break;
    case ScopeKind::Template: out << "template-parameters"; break;
    case ScopeKind::Block: out << "block"; break;
    }
    if (s == global) out << "<global>";
    else if (scope.display_name) write_name(out, scope.display_name);
    else if (scope.name) spelling(out, scope.name);
    else if (scope.kind == ScopeKind::Namespace) out << "<unnamed>";
    out << '\n';
    for (std::uint32_t d = scope.first_decl; d; d = declarations[d].next) {
        const Declaration& decl = declarations[d];
        const Entity& e = entities[decl.entity];
        indent(out, depth + 1); out << binding_name(decl.kind) << ' ';
        if (decl.display_name && decl.kind == EntityKind::Type) write_name(out, decl.display_name);
        else spelling(out, e.name);
        out << ' ';
        write_type(out, decl.type, decl.display_name, decl.key, decl.kind == EntityKind::Variable ? e.type : 0);
        if (decl.kind == EntityKind::Enumerator) {
            out << ' ';
            if (is_unsigned(e.constant.type)) out << e.constant.bits;
            else out << static_cast<std::int64_t>(e.constant.bits);
        }
        out << '\n';
    }
    for (ScopeId c = scope.first_child; c; c = scopes[c].next) write_scope(out, c, depth + 1);
}
void Analyzer::write(std::ostream& out) const { out << "translation-unit\n"; write_scope(out, global, 1); }
void Analyzer::telemetry(std::ostream& out) const
{
    out << ",\"semantic_ms\":" << analysis_ms
        << ",\"semantic_fact_slots\":" << facts.slot_count()
        << ",\"semantic_fact_records\":" << facts.fact_count()
        << ",\"semantic_fact_storage_bytes\":" << facts.storage_bytes()
        << ",\"semantic_template_declaration_work\":" << template_declaration_work
        << ",\"semantic_declaration_publications\":" << declaration_publications
        << ",\"semantic_dependence_work\":" << dependence_work
        << ",\"semantic_specializations\":" << specializations.size() - 1
        << ",\"semantic_type_substitution_work\":" << substitution_work
        << ",\"semantic_type_substitution_hits\":" << substitution_hits
        << ",\"semantic_type_substitution_records\":" << substitution_records
        << ",\"semantic_substitution_frames\":" << substitution_frames.size()-1
        << ",\"semantic_template_type_work\":" << template_type_work
        << ",\"semantic_template_type_access_work\":" << template_type_access_work
        << ",\"semantic_template_type_uses\":" << template_type_uses
        << ",\"semantic_template_signature_work\":" << template_signature_work
        << ",\"semantic_template_signature_uses\":" << template_signature_uses
        << ",\"semantic_parameter_publications\":" << parameter_publications
        << ",\"semantic_template_default_binding_work\":" << template_default_binding_work
        << ",\"semantic_template_default_binding_queued\":" << template_default_binding_queued
        << ",\"semantic_template_initializer_binding_work\":" << template_initializer_binding_work
        << ",\"semantic_template_initializer_binding_queued\":" << template_initializer_binding_queued
        << ",\"template_body_transitions\":" << template_bodies
        << ",\"semantic_body_checks\":" << body_checks
        << ",\"semantic_body_lifetime_checks\":" << body_lifetime_checks
        << ",\"semantic_statement_conversion_work\":" << statement_conversion_work
        << ",\"semantic_statement_conversion_uses\":" << statement_conversion_uses
        << ",\"semantic_constant_array_work\":" << constant_array_work
        << ",\"semantic_base_adjustment_paths\":" << base_adjustments.size()-1
        << ",\"semantic_base_adjustment_work\":" << base_adjustment_work
        << ",\"semantic_base_adjustment_hits\":" << base_adjustment_hits
        << ",\"semantic_static_initialization_work\":" << static_initialization_work
        << ",\"semantic_static_plan_work\":" << static_plan_work
        << ",\"semantic_constant_array_index_entries\":" << constant_array_children.size()
        << ",\"semantic_floating_constants\":" << floating_constants.size()-1
        << ",\"semantic_constant_bodies\":" << constant_bodies.size()-1
        << ",\"semantic_constant_activations\":" << constant_activations.size()-1
        << ",\"semantic_constant_execution_steps\":" << constant_steps
        << ",\"semantic_constant_object_work\":" << constant_object_work
        << ",\"semantic_constant_address_work\":" << constant_address_work
        << ",\"semantic_constant_dependency_work\":" << constant_dependency_work
        << ",\"semantic_constant_persistence_work\":" << constant_persistence_work
        << ",\"semantic_constant_execution_hits\":" << constant_hits
        << ",\"semantic_initializer_recipe_work\":" << initializer_recipe_work
        << ",\"semantic_initializer_recipe_uses\":" << initializer_recipe_uses
        << ",\"semantic_default_initialization_work\":" << default_initialization_work
        << ",\"semantic_default_initialization_uses\":" << default_initialization_uses
        << ",\"template_class_completions\":" << template_completions
        << ",\"explicit_specialization_selections\":" << explicit_selections
        << ",\"variable_template_initializers\":" << variable_initializers
        << ",\"variable_template_reuses\":" << variable_reuses
        << ",\"variable_template_candidates\":" << variable_candidates
        << ",\"function_ordering_work\":" << function_ordering_work
        << ",\"function_ordering_hits\":" << function_ordering_hits
        << ",\"class_pattern_ordering_work\":" << class_ordering_work
        << ",\"class_pattern_ordering_hits\":" << class_ordering_hits
        << ",\"template_definition_applications\":" << template_definition_work
        << ",\"template_definition_direct_applications\":" << definition_direct_work
        << ",\"template_definition_requests\":" << definition_requests
        << ",\"template_definition_hits\":" << definition_hits
        << ",\"template_definition_edges\":" << definition_edges
        << ",\"template_definition_signature_requests\":" << definition_signature_requests
        << ",\"template_definition_signature_work\":" << definition_signature_work
        << ",\"semantic_expression_bytes\":" << sizeof(Expression)
        << ",\"semantic_type_queries\":" << type_queries.size()-1
        << ",\"semantic_type_query_work\":" << query_work
        << ",\"semantic_candidate_substitutions\":" << candidate_substitution_work
        << ",\"semantic_candidate_cycles\":" << candidate_substitution_cycles
        << ",\"query_completion_edges\":" << query_dependencies.size()-1
        << ",\"query_completion_invalidations\":" << query_invalidations
        << ",\"semantic_deferred_function_uses\":" << deferred_function_uses.size()-1
        << ",\"semantic_deferred_function_use_work\":" << deferred_function_use_cursor
        << ",\"semantic_template_binding_work\":" << template_binding_work
        << ",\"semantic_template_parameter_check_work\":" << template_parameter_check_work
        << ",\"semantic_template_parameter_check_reuses\":" << template_parameter_check_reuses
        << ",\"semantic_template_bindings\":" << template_bindings.size()-1
        << ",\"semantic_template_fixed_expressions\":" << template_fixed_work
        << ",\"semantic_template_fixed_uses\":" << template_fixed_uses
        << ",\"semantic_template_fixed_calls\":" << template_fixed_call_work
        << ",\"semantic_template_fixed_call_uses\":" << template_fixed_call_uses
        << ",\"semantic_template_object_contexts\":" << template_object_contexts.size()-1
        << ",\"semantic_template_member_uses\":" << template_member_uses.size()-1
        << ",\"parsed_nodes\":" << ast.nodes.parsed_size()
        << ",\"template_occurrences\":" << ast.nodes.size() - ast.nodes.parsed_size()
        << ",\"template_deferred_regions\":" << static_cast<const syntax::Ast&>(ast).deferred_regions
        << ",\"template_demanded_regions\":" << static_cast<const syntax::Ast&>(ast).demanded_regions
        << ",\"template_source_regions\":" << static_cast<const syntax::Ast&>(ast).source_regions.size()
        << ",\"template_region_nodes\":" << static_cast<const syntax::Ast&>(ast).region_nodes.size()
        << ",\"template_region_roots\":" << static_cast<const syntax::Ast&>(ast).region_roots.size()
        << ",\"template_default_environment_work\":" << default_environment_work
        << ",\"template_default_argument_work\":" << default_argument_work
        << ",\"template_default_facts\":" << default_argument_facts.size()-1
        << ",\"template_default_dependencies\":" << default_dependencies.size()
        << ",\"template_default_dependency_work\":" << default_dependency_work
        << ",\"template_default_demands\":" << default_demand_work
        << ",\"semantic_pack_expansion_work\":" << expansion_work
        << ",\"semantic_pack_expansion_lanes\":" << expansion_lanes
        << ",\"semantic_substitution_frames\":" << substitution_frames.size()-1
        << ",\"semantic_argument_packs\":" << argument_packs.size() - 1
        << ",\"semantic_object_uses\":" << object_uses.size() - 1
        << ",\"semantic_expression_facts\":" << expressions.fact_count()
        << ",\"semantic_expression_uses\":" << expressions.use_count()
        << ",\"semantic_expression_slots\":" << expressions.slot_count()
        << ",\"semantic_expression_inherited\":" << expressions.inherited
        << ",\"semantic_expression_variants\":" << expressions.variants
        << ",\"semantic_call_argument_edges\":" << call_arguments.size()
        << ",\"semantic_constructor_actions\":" << subobject_actions.size()
        << ",\"semantic_conversion_objects\":" << conversion_objects.size()-1
        << ",\"semantic_user_conversions\":" << user_conversions.size()-1
        << ",\"semantic_list_plans\":" << list_plans.size()-1
        << ",\"semantic_list_objects\":" << list_objects.size()-1
        << ",\"semantic_value_initializations\":" << value_initializations.size()-1
        << ",\"semantic_value_returns\":" << value_returns.size()-1
        << ",\"semantic_function_returns\":" << function_returns.size()-1
        << ",\"semantic_placement_objects\":" << placements.size()-1
        << ",\"semantic_constant_constructor_actions\":" << constructor_constant_actions.size()
        << ",\"semantic_constant_fields\":" << constant_fields.size()
        << ",\"semantic_initializer_actions\":" << initializers.size()-1
        << ",\"semantic_zero_plans\":" << zero_initializations.size()-1
        << ",\"semantic_zero_parts\":" << zero_parts.size()
        << ",\"semantic_field_descriptors\":" << field_facts.size()-1
        << ",\"semantic_transfer_unit_fields\":" << unit_transfer_fields
        << ",\"semantic_scalar_observations\":" << scalar_observation_count
        << ",\"semantic_scalar_consumptions\":" << scalar_consumptions.size()-1
        << ",\"semantic_scalar_consumption_work\":" << scalar_consumption_work
        << ",\"semantic_parameter_queries\":" << parameter_queries
        << ",\"semantic_parameter_query_work\":" << parameter_query_work
        << ",\"semantic_destruction_actions\":" << destruction_actions.size()
        << ",\"semantic_transfer_actions\":" << transfers.size()
        << ",\"semantic_scalar_transfer_work\":" << scalar_transfer_work
        << ",\"semantic_scalar_transfer_cache_bytes\":" << scalar_transfer_nodes.size()
        << ",\"semantic_lifetime_states\":" << lifetimes.size()-1
        << ",\"semantic_reference_binding_work\":" << reference_binding_work
        << ",\"semantic_reference_alternatives\":" << reference_alternatives.size()-1
        << ",\"semantic_lifetime_uses\":" << lifetime_uses.size()-1
        << ",\"semantic_virtual_classes\":" << virtual_classes.size()-1
        << ",\"semantic_virtual_slot_work\":" << virtual_slot_work
        << ",\"semantic_virtual_declaration_work\":" << virtual_declaration_work
        << ",\"semantic_virtual_demands\":" << virtual_demands
        << ",\"semantic_key_vtable_notifications\":" << key_vtable_demand.size()
        << ",\"semantic_key_vtable_processed\":" << key_vtable_cursor
        << ",\"semantic_vtable_emissions\":" << vtable_emission.size()
        << ",\"semantic_vtable_queue_bytes\":" << (key_vtable_demand.capacity()+vtable_emission.capacity())*sizeof(EntityId)
        << ",\"semantic_member_demands\":" << demand_queue.size()
        << ",\"semantic_friend_definitions\":" << friend_definitions.size()
        << ",\"semantic_friend_demands\":" << friend_definition_demand.size()
        << ",\"semantic_friend_processed\":" << friend_definition_cursor
        << ",\"semantic_demand_processed\":" << demand_cursor
        << ",\"semantic_expression_work\":" << expression_work
        << ",\"semantic_candidate_work\":" << candidate_work
        << ",\"semantic_conversion_work\":" << conversion_work
        << ",\"semantic_conversions\":" << conversions.size() - 1
        << ",\"semantic_entity_bytes\":" << sizeof(Entity)
        << ",\"semantic_class_facts\":" << class_facts.size() - 1
        << ",\"semantic_constants\":" << constants.size() - 2
        << ",\"semantic_value_queries\":" << query_values.size() - 1
        << ",\"semantic_value_query_work\":" << query_value_work
        << ",\"semantic_template_value_sources\":" << template_value_work
        << ",\"semantic_template_value_uses\":" << template_value_uses
        << ",\"semantic_value_conversion_variants\":" << value_conversion_variants
        << ",\"semantic_value_conversion_records\":" << value_conversion_records
        << ",\"semantic_types\":" << types.records.size() - 1
        << ",\"semantic_entities\":" << entities.size() - 1
        << ",\"semantic_scopes\":" << scopes.size() - 1
        << ",\"semantic_edges\":" << edges.size() - 1
        << ",\"semantic_declarations\":" << declarations.size() - 1
        << ",\"semantic_regions\":" << analyzed
        << ",\"semantic_lookup_work\":" << lookup_work
        << ",\"semantic_signature_work\":" << types.signature_work
        << ",\"semantic_template_signature_shape_work\":" << template_signature_shape_work
        << ",\"semantic_template_signature_shape_hits\":" << template_signature_shape_hits
        << ",\"semantic_type_probes\":" << types.probes
        << ",\"semantic_constant_work\":" << constant_work
        << ",\"constexpr_validity_work\":" << constexpr_validity_work
        << ",\"exception_work\":" << exception_work
        << ",\"closures\":" << closures.size()-1;
}
} }
