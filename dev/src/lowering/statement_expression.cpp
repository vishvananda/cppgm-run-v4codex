#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
std::uint32_t Procedural::map_lifetime(std::uint32_t state, std::uint32_t overlay)
{
    if (!overlay || (state & 0x80000000u)) return state;
    auto context = lifetime_overlays[overlay];
    if (state == context.anchor) return context.replacement;
    if (sem.lifetimes[state].depth <= sem.lifetimes[context.anchor].depth)
        return map_lifetime(state,context.parent);
    auto key = (std::uint64_t(overlay)<<32)|state;
    if (auto mapped = mapped_lifetimes.get(key)) { ++lifetime_mapping_hits; return mapped; }
    ++lifetime_mapping_work;
    auto tail = map_lifetime(sem.lifetimes[state].tail,overlay);
    if (tail == sem.lifetimes[state].tail) { mapped_lifetimes.put(key,state); return state; }
    TemporaryState mapped;
    static_cast<semantic::LifetimeState&>(mapped) = sem.lifetimes[state];
    mapped.tail = tail; mapped.depth = lifetime_state(tail).depth+1;
    temporary_states.push_back(mapped);
    auto result = 0x80000000u | temporary_states.size();
    mapped_lifetimes.put(key,result); return result;
}
std::uint32_t Procedural::object_lifetime(EntityId object)
{
    auto state = sem.object_lifetime(object);
    return state ? map_lifetime(state,lifetime_overlay) : 0;
}
semantic::LifetimeUse Procedural::lifetime_use(NodeId n)
{
    auto use = sem.lifetime_use(n);
    auto overlay_for = [&](NodeId region) {
        if (!region) return std::uint32_t(0);
        auto context = lifetime_overlay;
        while (context && lifetime_overlays[context].region != region) context = lifetime_overlays[context].parent;
        if (!context) throw std::logic_error("missing statement lifetime region");
        return context;
    };
    auto context = overlay_for(use.expression_region);
    use.entry = map_lifetime(use.entry,context); use.exit = map_lifetime(use.exit,context);
    use.target = map_lifetime(use.target,overlay_for(use.target_region));
    return use;
}
Value Procedural::statement_expression(NodeId n, Value destination)
{
    ++statement_regions;
    auto fact = sem.expression_fact(n);
    auto body = ast[n].first, result = sem.facts[n].target;
    bool supplied = destination.ir != IRType::Void;
    bool object = sem.class_value(fact.type);
    close_expression_region();
    auto outer = full_expression;
    auto initial = live;
    auto parent = lifetime_overlay;
    lifetime_overlays.push_back({n,sem.lifetime_use(body).entry,initial,parent});
    lifetime_overlay = lifetime_overlays.size()-1;
    full_expression = FullExpression();
    if (object && !supplied) destination = class_address(sem.object_fact(n).temporary,fact.type);
    Value value(Operand::integer(0),type(fact.type),fact.type);
    for (auto c = ast[body].first; c; c = ast[c].next) {
        if (result && c == ast[body].last) {
            if (ended) break;
            auto prefix = live;
            auto conversion = sem.conversion_fact(fact.conversions);
            bool elided = object && conversion.kind == semantic::Conversion::Kind::Construction &&
                sem.conversion_objects[conversion.materialization].elided;
            begin_full_expression(result,elided);
            if (object) construct_value(result,conversion,destination);
            else if (type(fact.type) == IRType::Void) expression(result);
            else value = converted(result,conversion);
            if (!ended) finish_full_expression(prefix);
        } else statement(c);
    }
    if (!ended) clean_inline(live,initial);
    lifetime_overlay = parent;
    full_expression = outer;
    if (object) {
        if (!ended && !supplied) activate_temporary(sem.object_fact(n).temporary);
        destination.type = fact.type; destination.address = true; return destination;
    }
    return value;
}
} }
