#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::converting_transfer(EntityId ctor, const Expression& call) const
{
    if (!transfer_member(ctor) || !call.argument_count) return false;
    const auto c = conversions[call.conversions];
    if (c.kind != Conversion::Kind::User) return false;
    TypeId returned = types[entities[c.function].type].child;
    return class_value(returned) && types[returned].entity == scopes[entities[ctor].owner].entity;
}
Conversion Analyzer::elided_conversion(const Expression& call, TypeId target)
{
    Conversion c = conversions[call.conversions];
    UserConversion record = user_conversions[c.materialization];
    record.result = Conversion(); record.result.target = target; record.result.rank = 0;
    c.target = target; c.reference = false; c.materialization = user_conversions.size(); user_conversions.push_back(record);
    return c;
}
std::vector<EntityId> Analyzer::conversion_candidates(TypeId source)
{
    std::vector<EntityId> result;
    if (!class_value(source)) return result;
    Index hidden;
    EntityId cls = types[source].entity;
    while (cls) {
        auto info = entities[cls].class_info;
        std::vector<TypeId> targets;
        for (EntityId e = class_facts[info].first_conversion; e; e = members[entities[e].member_info].next_conversion) {
            TypeId t = members[entities[e].member_info].conversion_target;
            if (!hidden.get(t)) result.push_back(e);
            targets.push_back(t);
        }
        for (TypeId t : targets) hidden.put(t,1);
        auto base = class_facts[info].first_base; cls = base ? bases[base].base : 0;
    }
    return result;
}
EntityId Analyzer::conversion_lookup(ScopeId owner, TypeId target)
{
    while (owner && scopes[owner].kind == ScopeKind::Class) {
        if (auto e = conversion_bindings.get(key(owner,target))) return e;
        auto b = class_facts[entities[scopes[owner].entity].class_info].first_base;
        owner = b ? entities[bases[b].base].scope : 0;
    }
    return 0;
}
Conversion Analyzer::conversion_function(NodeId n, TypeId to, bool explicit_allowed, bool direct_reference, EntityId object_entity)
{
    Conversion result; result.target = to;
    Expression source = expressions[n];
    if (object_entity) { source.type = value_type(entities[object_entity].type); source.category = ValueCategory::Lvalue; }
    struct Candidate { EntityId function; Conversion object, second; };
    std::vector<Candidate> viable;
    for (EntityId e : conversion_candidates(source.type)) {
        ++candidate_work;
        auto m = members[entities[e].member_info];
        if (entities[e].template_info || (m.explicit_constructor && !explicit_allowed)) continue;
        Conversion object = object_conversion(e,source.type,source.category);
        if (!object.valid()) continue;
        TypeId declared = types[entities[e].type].child;
        Expression value; value.type = value_type(declared);
        value.category = types[declared].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[declared].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        Conversion second = standard_conversion(value,to);
        if (!second.valid() || (direct_reference && (value.category == ValueCategory::Prvalue || second.temporary))) continue;
        if (m.explicit_constructor && second.rank != 0) continue;
        if (second.kind == Conversion::Kind::Construction && value.category == ValueCategory::Prvalue && types.unqualified(value.type) == types.unqualified(to)) {
            // The call result initializes this complete object directly.
            second.kind = Conversion::Kind::Standard;
        }
        viable.push_back({e,object,second});
    }
    if (viable.empty()) return result;
    auto preferred = [&](const Candidate& a, const Candidate& b) {
        if (better(&a.object,&b.object,1)) return true;
        if (better(&b.object,&a.object,1)) return false;
        return better(&a.second,&b.second,1);
    };
    std::size_t best = 0;
    for (std::size_t i = 1; i < viable.size(); ++i) if (preferred(viable[i],viable[best])) best = i;
    for (std::size_t i = 0; i < viable.size(); ++i) if (i != best && !preferred(viable[best],viable[i])) return result;
    auto selected = viable[best];
    UserConversion sequence; sequence.object = selected.object; sequence.result = selected.second; sequence.object_entity = object_entity;
    sequence.adjustment = base_steps(source.type,scopes[entities[selected.function].owner].entity);
    result.kind = Conversion::Kind::User; result.function = selected.function; result.rank = 5;
    result.reference = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    result.materialization = user_conversions.size(); user_conversions.push_back(sequence);
    return result;
}
Conversion Analyzer::conversion(NodeId n, TypeId to, bool user)
{
    if (ast[n].kind == syntax::Kind::BracedInit) return list_initialization(n,to);
    Conversion result = standard_conversion(expressions[n],to,n);
    if (result.valid() || !user) return result;
    bool ref = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    if (ref) {
        auto direct = conversion_function(n,to,false,true);
        if (direct.valid()) return direct;
    }
    Conversion function = conversion_function(n,to);
    Conversion constructor; constructor.target = to;
    TypeId target = value_type(to);
    if (class_value(target) && (!ref || types[to].kind == TypeKind::RRef || types[target].cv == 1)) {
        constructor = converting_constructor(n,types.unqualified(target));
        constructor.target = to; constructor.reference = ref; constructor.temporary = ref;
        constructor.preference = ref && types[to].kind == TypeKind::LRef;
        constructor.qualification = ref ? types[target].cv : 0;
    }
    if (function.valid() && constructor.valid()) {
        Type f = types[entities[constructor.function].type];
        Conversion argument = standard_conversion(expressions[n],types.parameters[f.offset],n);
        auto object = user_conversions[function.materialization].object;
        if (better(&argument,&object,1)) return constructor;
        if (better(&object,&argument,1)) return function;
        return result;
    }
    return function.valid() ? function : constructor;
}
void Analyzer::prepare_user_conversion(NodeId n, Conversion& c)
{
    if (user_conversions[c.materialization].prepared) return;
    if (deleted_transfer(c.function)) throw std::runtime_error("selected deleted conversion function");
    auto entity = user_conversions[c.materialization].object_entity;
    TypeId source = entity ? value_type(entities[entity].type) : expressions[n].type;
    check_access(c.function,facts[n].scope,entities[types[source].entity].scope,source);
    TypeId owner = entities[scopes[entities[c.function].owner].entity].type;
    if (user_conversions[c.materialization].adjustment) check_base_access(source,owner,facts[n].scope);
    demand_member(c.function);
    TypeId returned = types[entities[c.function].type].child;
    Conversion second = user_conversions[c.materialization].result;
    if (second.function) {
        if (deleted_transfer(second.function)) throw std::runtime_error("deleted conversion result transfer");
        check_access(second.function,facts[n].scope,entities[second.function].owner);
    }
    auto temporary = [&](TypeId type) {
        EntityId temporary = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,facts[n].scope),0,n);
        entities[temporary].type = type; register_destruction(temporary);
        return temporary;
    };
    if (second.kind == Conversion::Kind::Construction) {
        ConversionObject transfer; transfer.constructor = second.function;
        transfer.temporary = temporary(value_type(c.target));
        user_conversions[c.materialization].temporary = transfer.temporary;
        if (class_value(returned)) user_conversions[c.materialization].source_temporary = temporary(returned);
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        Type ctor = types[entities[second.function].type];
        std::vector<NodeId> args(1,0);
        std::vector<Conversion> selected(1,standard_conversion(value,types.parameters[ctor.offset]));
        for (unsigned j = 1; j < ctor.count; ++j) {
            NodeId arg = default_arguments[entities[second.function].defaults+j];
            args.push_back(arg); selected.push_back(conversion(arg,types.parameters[ctor.offset+j]));
        }
        record_call(transfer.call,args,selected);
        user_conversions[c.materialization].result.materialization = conversion_objects.size();
        conversion_objects.push_back(transfer); demand_member(second.function);
    } else if (class_value(returned)) {
        user_conversions[c.materialization].temporary = temporary(returned);
    }
    user_conversions[c.materialization].prepared = true;
}
} }
