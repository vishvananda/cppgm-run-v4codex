#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
CallSelection Analyzer::select_call(EntityId family, const std::vector<Expression>& values,
    const std::vector<NodeId>* nodes, TypeId object, ValueCategory category,
    ScopeId naming, std::uint32_t explicit_arguments, std::vector<Conversion>& selected)
{
    // The value vector itself is borrowed, not its data pointer: recursive
    // declaration/layout demands may grow the TU expression arena.
    auto count = nodes ? nodes->size() : values.size();
    struct Candidate { EntityId entity; unsigned offset; };
    std::vector<Candidate> viable; std::vector<Conversion> sequences;
    Index concrete_candidates;
    auto declarations = candidates(family);
    for (auto e : declarations) {
        ++candidate_work;
        if (entities[e].template_info) {
            if (explicit_arguments) {
                auto pack = argument_packs[explicit_arguments];
                std::vector<TypeId> arguments(argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
                e = specialize(e,arguments,true);
            }
            if (e && entities[e].template_info) e = nodes ? deduce_function(e,*nodes) : deduce_function(e,values);
        } else if (explicit_arguments) continue;
        if (!e) continue;
        if (declarations.size() > 1) {
            if (concrete_candidates.get(e)) continue;
            concrete_candidates.put(e,1);
        }
        auto f = types[entities[e].type];
        if ((!f.variadic && count > f.count) || (count < f.count &&
            (!entities[e].defaults || !default_arguments[entities[e].defaults+count]))) continue;
        auto begin = sequences.size(); bool valid = true;
        if (object) {
            Conversion c; c.rank = 0; c.target = object;
            if (entities[e].member_info && !entities[e].is_static) c = object_conversion(e,object,category,naming);
            valid = c.valid(); sequences.push_back(c);
        } else if (entities[e].member_info && !entities[e].is_static) valid = false;
        for (unsigned i = 0; valid && i < count; ++i) {
            Conversion c;
            if (nodes) {
                auto n = (*nodes)[i];
                c = i < f.count ? conversion(n,types.parameters[f.offset+i]) : ellipsis_conversion(n);
            } else c = i < f.count ? conversion_value(values[i],types.parameters[f.offset+i]) : ellipsis_conversion_value(values[i]);
            valid = c.valid(); sequences.push_back(c);
        }
        if (valid) viable.push_back({e,unsigned(begin)}); else sequences.resize(begin);
    }
    CallSelection result;
    if (viable.empty()) return result;
    auto preferred = [&](unsigned a, unsigned b) {
        auto x = sequences.data()+viable[a].offset, y = sequences.data()+viable[b].offset;
        auto arguments = count+(object!=0);
        if (better(x,y,arguments)) return true;
        for (unsigned i = 0; i < arguments; ++i)
            if (better(y+i,x+i,1)) return false;
        auto ae = viable[a].entity, be = viable[b].entity;
        bool conversion = members[entities[ae].member_info].conversion_target && members[entities[be].member_info].conversion_target;
        return (!entities[ae].specialization && entities[be].specialization) ||
            template_more_specialized(ae,be,conversion ? ~0u : count,false,conversion);
    };
    unsigned best = 0;
    for (unsigned i = 1; i < viable.size(); ++i) if (preferred(i,best)) best = i;
    result.entity = viable[best].entity;
    for (unsigned i = 0; i < viable.size(); ++i)
        if (i != best && viable[i].entity != result.entity && !preferred(best,i)) {
            result.failure = CallFailure::Ambiguous; result.conflicting = viable[i].entity; return result;
        }
    auto begin = viable[best].offset;
    selected.assign(sequences.begin()+begin,sequences.begin()+begin+count+(object!=0));
    result.failure = CallFailure::None; return result;
}
} }
