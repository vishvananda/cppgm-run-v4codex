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
    case TypeKind::Fundamental: out << fundamental_name(t.fundamental); break;
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
        << ",\"semantic_dependence_work\":" << dependence_work
        << ",\"semantic_specializations\":" << specializations.size() - 1
        << ",\"semantic_argument_packs\":" << argument_packs.size() - 1
        << ",\"semantic_object_uses\":" << object_uses.size() - 1
        << ",\"semantic_constructor_actions\":" << subobject_actions.size()
        << ",\"semantic_conversion_objects\":" << conversion_objects.size()-1
        << ",\"semantic_value_initializations\":" << value_initializations.size()-1
        << ",\"semantic_value_returns\":" << value_returns.size()-1
        << ",\"semantic_function_returns\":" << function_returns.size()-1
        << ",\"semantic_placement_objects\":" << placements.size()-1
        << ",\"semantic_constant_constructor_actions\":" << constructor_constant_actions.size()
        << ",\"semantic_constant_fields\":" << constant_fields.size()
        << ",\"semantic_initializer_actions\":" << initializers.size()-1
        << ",\"semantic_field_descriptors\":" << field_facts.size()-1
        << ",\"semantic_destruction_actions\":" << destruction_actions.size()
        << ",\"semantic_transfer_actions\":" << transfers.size()
        << ",\"semantic_lifetime_states\":" << lifetimes.size()-1
        << ",\"semantic_lifetime_uses\":" << lifetime_uses.size()-1
        << ",\"semantic_member_demands\":" << demand_queue.size()
        << ",\"semantic_demand_processed\":" << demand_cursor
        << ",\"semantic_expression_work\":" << expression_work
        << ",\"semantic_candidate_work\":" << candidate_work
        << ",\"semantic_conversion_work\":" << conversion_work
        << ",\"semantic_conversions\":" << conversions.size() - 1
        << ",\"semantic_entity_bytes\":" << sizeof(Entity)
        << ",\"semantic_class_facts\":" << class_facts.size() - 1
        << ",\"semantic_constants\":" << constants.size() - 2
        << ",\"semantic_types\":" << types.records.size() - 1
        << ",\"semantic_entities\":" << entities.size() - 1
        << ",\"semantic_scopes\":" << scopes.size() - 1
        << ",\"semantic_edges\":" << edges.size() - 1
        << ",\"semantic_declarations\":" << declarations.size() - 1
        << ",\"semantic_regions\":" << analyzed
        << ",\"semantic_lookup_work\":" << lookup_work
        << ",\"semantic_signature_work\":" << types.signature_work
        << ",\"semantic_type_probes\":" << types.probes
        << ",\"semantic_constant_work\":" << constant_work;
}
} }
