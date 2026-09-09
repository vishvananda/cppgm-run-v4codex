#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
Conversion Analyzer::converting_constructor(NodeId n, TypeId target)
{
    Conversion result; result.target = target;
    EntityId cls = types[target].entity;
    struct Candidate { EntityId entity; Conversion argument; };
    std::vector<Candidate> viable;
    for (EntityId e : candidates(class_facts[entities[cls].class_info].constructor)) {
        if (!e) continue;
        ++candidate_work;
        auto m = members[entities[e].member_info];
        Type f = types[entities[e].type];
        if (m.explicit_constructor || !f.count || (f.count > 1 &&
            (!entities[e].defaults || !default_arguments[entities[e].defaults+1]))) continue;
        Conversion argument = conversion(n, types.parameters[f.offset], false);
        if (argument.valid()) viable.push_back({e, argument});
    }
    if (viable.empty()) return result;
    std::size_t best = 0;
    for (std::size_t j = 1; j < viable.size(); ++j)
        if (better(&viable[j].argument, &viable[best].argument, 1)) best = j;
    for (std::size_t j = 0; j < viable.size(); ++j)
        if (j != best && !better(&viable[best].argument, &viable[j].argument, 1)) return result;
    result.kind = Conversion::Kind::Construction; result.rank = 5;
    result.function = viable[best].entity;
    return result;
}
void Analyzer::materialize_conversion(NodeId n, Conversion& conversion)
{
    if (conversion.materialization) return;
    EntityId ctor = conversion.function;
    auto m = entities[ctor].member_info;
    if (members[m].deleted) throw std::runtime_error("deleted converting constructor");
    check_access(ctor, facts[n].scope, entities[ctor].owner);
    demand_member(ctor); members[m].complete_entry = true;
    TypeId t = value_type(conversion.target);
    EntityId object = make_entity(EntityKind::Variable, make_scope(ScopeKind::Block, facts[n].scope), 0, n);
    entities[object].type = types.unqualified(t); register_destruction(object);
    ConversionObject materialized; materialized.constructor = ctor; materialized.temporary = object;
    Type f = types[entities[ctor].type];
    std::vector<NodeId> arguments;
    std::vector<Conversion> selected;
    for (unsigned j = 0; j < f.count; ++j) {
        NodeId a = j ? default_arguments[entities[ctor].defaults+j] : n;
        Conversion c = this->conversion(a, types.parameters[f.offset+j], false);
        if (!c.valid()) throw std::runtime_error("invalid converting constructor argument");
        apply_conversion(a, c); arguments.push_back(a); selected.push_back(c);
    }
    materialized.call.arguments = call_arguments.size(); materialized.call.argument_count = f.count;
    materialized.call.conversions = conversions.size(); materialized.call.count = f.count;
    call_arguments.insert(call_arguments.end(), arguments.begin(), arguments.end());
    conversions.insert(conversions.end(), selected.begin(), selected.end());
    conversion.materialization = conversion_objects.size(); conversion_objects.push_back(materialized);
}
} }
