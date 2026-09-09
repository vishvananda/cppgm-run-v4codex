#include "semantic/analyzer.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
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
    TypeId base = specifiers(specs, s);
    if (spec_has(specs, KW_STATIC)) throw std::runtime_error("static bit-field");
    for (NodeId field = ast[specs].next; field; field = ast[field].next) {
        NodeId first = ast[field].first;
        NodeId d = ast[first].kind == Kind::Declarator ? first : 0;
        NodeId bound = d ? ast[d].next : first;
        TypeId t = declarator(d, base, s);
        if (!integral(t)) throw std::runtime_error("nonintegral bit-field");
        Constant count = evaluate(bound, s);
        if (!count.valid || !integral(count.type) || (!is_unsigned(count.type) && std::int64_t(count.bits) < 0))
            throw std::runtime_error("invalid bit-field width");
        IdentifierId name = terminal(decl_name(d));
        if (name && !count.bits) throw std::runtime_error("named zero-width bit-field");
        EntityId e;
        if (name) e = declare_object(d, 0, t, specs, s, n);
        else {
            e = make_entity(EntityKind::Variable, s, 0, field); entities[e].type = t;
            record(s, e, field, t, EntityKind::Variable);
        }
        auto& f = field_metadata(e); f.bit_field = true; f.declared_width = count.bits;
        f.storage_type = types[t].kind == TypeKind::Named ? entities[types[t].entity].underlying : types.unqualified(t);
        f.width = std::min<std::uint64_t>(count.bits, width(f.storage_type));
    }
}
} }
