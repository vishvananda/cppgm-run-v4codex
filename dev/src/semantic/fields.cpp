#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
Constant Analyzer::constant_field_value(EntityId field, Constant value)
{
    auto f = field_fact(field);
    if (!value.valid || !f.bit_field || !f.width) return value;
    // Match the target's stored bit-field value before any later initializer
    // or constexpr read observes it. Signed fields use the runtime sign policy.
    if (f.width < 128) {
        auto mask = (WideInteger(1) << f.width)-1;
        auto bits = integer_value(value) & mask;
        if (!is_unsigned(f.storage_type) && (bits & (WideInteger(1) << (f.width-1))))
            bits |= ~mask;
        value = integer_constant(value.type,bits);
    }
    return value;
}
std::uint64_t Analyzer::alignment_attributes(NodeId n, ScopeId s, bool* strict)
{
    std::uint64_t result = 0;
    for (auto a = ast.alignment_owners.get(n); a; a = ast.alignments[a].next) {
        auto attribute = ast.alignments[a];
        std::uint64_t value;
        if (attribute.gnu && !attribute.operand) value = 16;
        else if (attribute.type) value = size(type_id(attribute.operand, s), true);
        else {
            auto c = evaluate(attribute.operand, s);
            if (!c.valid || !integral(c.type) || (negative_constant(c) || integer_value(c) > ~std::uint64_t(0)))
                throw std::runtime_error("invalid alignment constant");
            value = std::uint64_t(integer_value(c));
        }
        if (value && (value & (value-1))) throw std::runtime_error("alignment is not a power of two");
        if (strict && value && !attribute.gnu) *strict = true;
        if (attribute.gnu && !value) throw std::runtime_error("GNU alignment must be nonzero");
        result = std::max(result, value);
    }
    return result;
}
std::uint64_t Analyzer::gnu_alignment_constant(Constant c)
{
    if (!c.valid || !integral(c.type) || negative_constant(c) || integer_value(c) > ~std::uint64_t(0))
        throw std::runtime_error("invalid GNU alignment constant");
    auto value = std::uint64_t(integer_value(c));
    if (!value || (value & (value-1))) throw std::runtime_error("GNU alignment must be a nonzero power of two");
    return value;
}
TypeId Analyzer::aligned_typedef(TypeId type, NodeId source, NodeId specs, NodeId declarator, ScopeId scope)
{
    std::uint64_t alignment = 0; bool present = false;
    std::vector<ArgumentId> pending;
    for (auto n : {source,specs,declarator})
        for (auto a = ast.alignment_owners.get(n); a; a = ast.alignments[a].next) {
            auto attribute = ast.alignments[a]; present = true;
            if (!attribute.gnu || types[type].kind == TypeKind::Function)
                throw std::runtime_error("invalid typedef alignment");
            if (!attribute.operand) { alignment = std::max(alignment,std::uint64_t(16)); continue; }
            if (pattern_scope(scope)) bind_template_expression(attribute.operand,scope);
            auto q = expression_query(attribute.operand,scope);
            if (query_fact(q).dependent) pending.push_back(0x80000000U | q);
            else alignment = std::max(alignment,gnu_alignment_constant(constants[query_value(q)]));
        }
    return present ? types.aligned(type,alignment,pending.empty() ? 0 : intern_arguments(pending)) : type;
}
std::uint64_t Analyzer::storage_alignment(EntityId e)
{
    auto f = field_fact(e);
    return std::max(f.alignment,f.type_alignment ? std::uint64_t(1) << (f.type_alignment-1) : size(entities[e].type,true));
}
FieldFacts& Analyzer::field_metadata(EntityId e)
{
    auto index = field_index.get(e);
    if (!index) { index = field_facts.size(); field_facts.push_back(FieldFacts()); field_index.put(e, index); }
    return field_facts[index];
}
void Analyzer::bit_field_declaration(NodeId n, ScopeId s)
{
    if (scopes[s].kind != ScopeKind::Class) throw std::runtime_error("bit-field outside class");
    NodeId specs = ast[n].first;
    if (ast.alignment_owners.get(n) || ast.alignment_owners.get(specs)) throw std::runtime_error("aligned bit-field");
    TypeId base = specifiers(specs, s);
    if (spec_has(specs, KW_STATIC)) throw std::runtime_error("static bit-field");
    for (NodeId field = ast[specs].next; field; field = ast[field].next) {
        NodeId first = ast[field].first;
        NodeId d = ast[first].kind == Kind::Declarator ? first : 0;
        NodeId bound = d ? ast[d].next : first;
        TypeId t = declarator(d, base, s);
        if (!integral(t)) throw std::runtime_error("nonintegral bit-field");
        Constant count = evaluate(bound, s);
        if (!count.valid || !integral(count.type) || (negative_constant(count) || integer_value(count) > ~std::uint64_t(0)))
            throw std::runtime_error("invalid bit-field width");
        IdentifierId name = terminal(decl_name(d));
        if (name && !count.bits) throw std::runtime_error("named zero-width bit-field");
        EntityId e;
        if (name) e = declare_object(d, 0, t, specs, s, n);
        else {
            e = make_entity(EntityKind::Variable, s, 0, field); entities[e].type = t;
            record(s, e, field, t, EntityKind::Variable);
        }
        bit_field_properties(e,count);
    }
}
void Analyzer::bit_field_properties(EntityId e, Constant count)
{
    auto t = entities[e].type;
    if (!integral(t)) throw std::runtime_error("nonintegral bit-field");
    if (!count.valid || !integral(count.type) || (negative_constant(count) || integer_value(count) > ~std::uint64_t(0)))
        throw std::runtime_error("invalid bit-field width");
    if (entities[e].name && !count.bits) throw std::runtime_error("named zero-width bit-field");
    auto& f = field_metadata(e); f.bit_field = true; f.declared_width = std::uint64_t(integer_value(count));
    f.storage_type = types[t].kind == TypeKind::Named ? entities[types[t].entity].underlying : types.unqualified(t);
    f.width = std::min<std::uint64_t>(std::uint64_t(integer_value(count)),width(f.storage_type));
}
} }
