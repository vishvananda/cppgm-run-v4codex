#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
Expression Analyzer::placement_new(NodeId n, ScopeId s)
{
    using syntax::Kind;
    NodeId placement = child(n, Kind::Placement);
    if (!placement) throw std::runtime_error("allocation without placement is outside the current object subset");
    PlacementNew use; use.type = type_id(child(n, Kind::TypeId), s); size(use.type);
    use.initializer = child(n, Kind::Initializer);
    std::vector<NodeId> args;
    for (NodeId a = ast[ast[placement].first].first; a; a = ast[a].next) { expression(a, s); args.push_back(a); }
    EntityId family = 0;
    IdentifierId name = operator_name(KW_NEW);
    if (!child(n, Kind::Global) && types[use.type].kind == TypeKind::Named)
        family = lookup(entities[types[use.type].entity].scope, name, Lookup::Ordinary, true);
    if (!family) family = lookup(global, name);
    struct Candidate { EntityId entity; std::size_t begin; };
    std::vector<Candidate> viable;
    std::vector<Conversion> conversions_work;
    for (EntityId e : candidates(family)) {
        if (!e || entities[e].kind != EntityKind::Function) continue;
        ++candidate_work;
        Type f = types[entities[e].type];
        if (!f.count || !fundamental(types.parameters[f.offset], FT_UNSIGNED_LONG_INT) || f.count != args.size()+1) continue;
        bool valid = true; std::size_t begin = conversions_work.size();
        for (unsigned j = 0; valid && j < args.size(); ++j) {
            auto c = conversion(args[j], types.parameters[f.offset+j+1]); valid = c.valid(); conversions_work.push_back(c);
        }
        if (valid) viable.push_back({e, begin}); else conversions_work.resize(begin);
    }
    if (viable.empty()) throw std::runtime_error("no placement allocation function");
    std::size_t best = 0;
    for (std::size_t j = 1; j < viable.size(); ++j)
        if (better(conversions_work.data()+viable[j].begin, conversions_work.data()+viable[best].begin, args.size())) best = j;
    for (std::size_t j = 0; j < viable.size(); ++j)
        if (j != best && !better(conversions_work.data()+viable[best].begin, conversions_work.data()+viable[j].begin, args.size())) throw std::runtime_error("ambiguous placement allocation");
    use.allocation = viable[best].entity;
    check_access(use.allocation, s, entities[use.allocation].owner); demand_member(use.allocation);
    std::vector<Conversion> selected(conversions_work.begin()+viable[best].begin, conversions_work.begin()+viable[best].begin+args.size());
    record_call(use.call, args, selected);
    if (use.initializer) initialize(use.initializer, use.type, s);
    else {
        use.constructor = default_constructor(use.type, s);
        if (use.constructor) members[entities[use.constructor].member_info].complete_entry = true;
    }
    placement_index.put(n, placements.size()); placements.push_back(use);
    Expression result; result.type = types.compound(TypeKind::Pointer, use.type); return result;
}
} }
