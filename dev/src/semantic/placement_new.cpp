#include "semantic/analyzer.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace cppgm { namespace semantic {
Expression Analyzer::placement_new(NodeId n, ScopeId s)
{
    using syntax::Kind;
    NodeId placement = child(n, Kind::Placement);
    PlacementNew use;
    NodeId type_node = child(n,Kind::TypeId), specs = ast[type_node].first, d = ast[specs].next;
    NodeId suffix = child(d,Kind::Array);
    use.type = declarator(d,specifiers(specs,s),s,suffix);
    facts[type_node].type = use.type; facts[type_node].scope = s;
    if (types[use.type].kind == TypeKind::Array) {
        use.array = true; use.bound = suffix ? ast[suffix].first : 0;
        use.fixed_count = types[use.type].bound; use.type = types[use.type].child;
        if (suffix && !use.bound) throw std::runtime_error("missing array allocation extent");
    }
    use.leaf = use.type;
    while (types[use.leaf].kind == TypeKind::Array) use.leaf = types[use.leaf].child;
    size(use.type); use.stride = size(use.type);
    if (use.array && class_value(use.leaf)) use.cookie = std::max<std::uint64_t>(8,size(use.type,true));
    if (use.array && use.fixed_count > (std::numeric_limits<std::uint64_t>::max()-use.cookie)/use.stride)
        throw std::runtime_error("array allocation size exceeds size_t");
    use.initializer = child(n, Kind::Initializer);
    std::vector<NodeId> args;
    for (NodeId a = ast[ast[placement].first].first; a; a = ast[a].next) { expression(a, s); args.push_back(a); }
    EntityId family = 0;
    IdentifierId name = operator_name(KW_NEW,use.array);
    if (!child(n, Kind::Global) && class_value(use.leaf))
        family = lookup(entities[types[use.leaf].entity].scope, name, Lookup::Ordinary, true);
    if (!family) {
        if (!placement) global_allocation(KW_NEW,use.array);
        family = lookup(global, name);
    }
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
    if (use.array) {
        NodeId list = use.initializer ? ast[use.initializer].first : 0;
        if (list && ast[list].first) throw std::runtime_error("array allocation initializer list is not yet represented");
        use.zero = use.initializer != 0;
        use.constructor = default_constructor(use.leaf,s);
        use.destructor = default_destructor(use.leaf,s);
        if (use.constructor) {
            auto m = members[entities[use.constructor].member_info];
            use.zero &= m.synthetic && !m.defaulted_late;
        }
        if (use.zero) use.zero_plan = prepare_zero_initialization(use.leaf);
        if (use.constructor) members[entities[use.constructor].member_info].array_entry = true;
        if (use.constructor) use.deallocation = select_deallocation(use.leaf,true,child(n,Kind::Global),s);
    } else if (use.initializer) initialize(use.initializer, use.type, s);
    else {
        use.constructor = default_constructor(use.type, s);
        if (use.constructor) members[entities[use.constructor].member_info].complete_entry = true;
    }
    placement_index.put(n, placements.size()); placements.push_back(use);
    Expression result; result.type = types.compound(TypeKind::Pointer, use.type); return result;
}
} }
