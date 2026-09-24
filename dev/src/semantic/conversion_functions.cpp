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
Conversion Analyzer::result_conversion(EntityId ctor, const Expression& call, TypeId target)
{
    Conversion c = conversions[call.conversions];
    UserConversion record = user_conversions[c.materialization];
    record.result = Conversion(); record.result.target = target; record.result.rank = 0;
    // The nonempty trivial case can reuse the final result destination without
    // observable transfer work. Empty results retain their O0 address boundary;
    // nontrivial explicit transfers retain the selected constructor's effects.
    bool retained = members[entities[c.function].member_info].explicit_constructor &&
        (empty_class(target) || !trivial_transfer(ctor));
    if (retained) {
        // An explicit conversion admitted by direct initialization supplies
        // the selected transfer's source object. Retain that O0 boundary.
        record.result.kind = Conversion::Kind::Construction; record.result.function = ctor;
        record.prepared = false; record.temporary = record.source_temporary = 0;
    }
    c.target = target; c.reference = false; c.materialization = user_conversions.size(); user_conversions.push_back(record);
    if (retained) {
        prepare_user_conversion(call_argument(call),c);
        auto transfer = user_conversions[c.materialization].result.materialization;
        conversion_objects[transfer].retained = true;
        auto m = entities[ctor].member_info;
        members[m].retained_root |= empty_class(target);
        members[m].complete_entry = true;
    }
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
            if (auto canonical = members[entities[e].member_info].conversion_hiding_target) t = canonical;
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
    if (!owner || scopes[owner].kind != ScopeKind::Class) return 0;
    EntityId result = 0;
    for (auto e : conversion_candidates(entities[scopes[owner].entity].type)) {
        ++candidate_work;
        if (entities[e].template_info) e = deduce_conversion(e,target);
        // Explicit member calls name the exact conversion-type-id. The
        // qualification alternatives for initialization do not change it.
        if (e && types[entities[e].type].child == target) result = merge_lookup(result,e);
    }
    return result;
}
Conversion Analyzer::conversion_function(NodeId n, TypeId to, bool explicit_allowed, bool direct_reference, EntityId object_entity)
{
    Expression source = expressions[n];
    if (object_entity) { source.type = value_type(entities[object_entity].type); source.category = ValueCategory::Lvalue; }
    return conversion_function_value(source,to,explicit_allowed,direct_reference,object_entity);
}
Conversion Analyzer::conversion_function_value(Expression source, TypeId to, bool explicit_allowed, bool direct_reference, EntityId object_entity)
{
    Conversion result; result.target = to;
    struct Candidate { EntityId function; Conversion object, second; };
    std::vector<Candidate> viable;
    for (EntityId e : conversion_candidates(source.type)) {
        ++candidate_work;
        auto m = members[entities[e].member_info];
        if (m.explicit_constructor && !explicit_allowed) continue;
        Conversion object = object_conversion(e,source.type,source.category);
        if (!object.valid()) continue;
        bool templated = entities[e].template_info != 0;
        if (templated) {
            auto declared = types[entities[e].type].child;
            bool returns_reference = types[declared].kind == TypeKind::LRef || types[declared].kind == TypeKind::RRef;
            if (direct_reference && !returns_reference) continue;
            // A value result initializes the referred-to object. Its required
            // type is unqualified; the final sequence records the binding's
            // cv/category. Reference-return deduction retains that reference.
            auto required = returns_reference ? to : types.unqualified(value_type(to));
            e = deduce_conversion(e,required);
            if (!e) continue;
            m = members[entities[e].member_info];
            if (m.explicit_constructor && !explicit_allowed) continue;
        }
        TypeId declared = types[entities[e].type].child;
        Expression value; value.type = value_type(declared);
        value.category = types[declared].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[declared].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        Conversion second = standard_conversion(value,to);
        if (!second.valid() || (direct_reference && (value.category == ValueCategory::Prvalue || second.temporary))) continue;
        if ((templated || m.explicit_constructor) && second.rank != 0) continue;
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
        if (better(&a.second,&b.second,1)) return true;
        if (better(&b.second,&a.second,1)) return false;
        bool at = entities[a.function].specialization != 0, bt = entities[b.function].specialization != 0;
        return (!at && bt) || (at && bt && template_more_specialized(a.function,b.function,~0u,false,true));
    };
    std::size_t best = 0;
    for (std::size_t i = 1; i < viable.size(); ++i) if (preferred(viable[i],viable[best])) best = i;
    for (std::size_t i = 0; i < viable.size(); ++i) if (i != best && !preferred(viable[best],viable[i])) {
        result.ambiguous = true; return result;
    }
    auto selected = viable[best];
    UserConversion sequence; sequence.object = selected.object; sequence.result = selected.second; sequence.object_entity = object_entity;
    sequence.adjustment = base_steps(source.type,scopes[entities[selected.function].owner].entity);
    sequence.virtual_slot = members[entities[selected.function].member_info].virtual_slot;
    result.kind = Conversion::Kind::User; result.function = selected.function; result.rank = 5;
    result.reference = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    result.materialization = user_conversions.size(); user_conversions.push_back(sequence);
    return result;
}
Conversion Analyzer::conversion(NodeId n, TypeId to, bool user)
{
    if (ast[n].kind == syntax::Kind::BracedInit) return list_initialization(n,to);
    return conversion_value(expressions[n],to,user,n);
}
Conversion Analyzer::conversion_value(Expression source, TypeId to, bool user, NodeId n)
{
    Conversion result = standard_conversion(source,to,n);
    if (result.valid() || !user) return result;
    bool ref = types[to].kind == TypeKind::LRef || types[to].kind == TypeKind::RRef;
    if (ref) {
        auto direct = conversion_function_value(source,to,false,true);
        if (direct.valid()) return direct;
    }
    Conversion function = conversion_function_value(source,to);
    Conversion constructor; constructor.target = to;
    TypeId target = value_type(to);
    if (class_value(target) && (!ref || types[to].kind == TypeKind::RRef || types[target].cv == 1)) {
        constructor = converting_constructor_value(source,types.unqualified(target),n);
        constructor.target = to; constructor.reference = ref; constructor.temporary = ref;
        constructor.preference = ref && types[to].kind == TypeKind::LRef;
        constructor.qualification = ref ? types[target].cv : 0;
    }
    if (function.valid() && constructor.valid()) {
        Type f = types[entities[constructor.function].type];
        Conversion argument = standard_conversion(source,types.parameters[f.offset],n);
        auto object = user_conversions[function.materialization].object;
        if (better(&argument,&object,1)) return constructor;
        if (better(&object,&argument,1)) return function;
        bool ct = entities[constructor.function].specialization != 0;
        bool ft = entities[function.function].specialization != 0;
        if (ct != ft) return ct ? function : constructor;
        result.ambiguous = true; return result;
    }
    if (!function.valid() && !constructor.valid() && (function.ambiguous || constructor.ambiguous)) {
        result.ambiguous = true; return result;
    }
    return function.valid() ? function : constructor;
}
void Analyzer::prepare_user_conversion(NodeId n, Conversion& c, ConversionUse use)
{
    if (user_conversions[c.materialization].prepared) return;
    if (use == ConversionUse::Recipe || (use == ConversionUse::Destination && c.reference))
        throw std::logic_error("invalid user conversion destination");
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
        transfer.use = use;
        if (use == ConversionUse::Temporary) transfer.temporary = temporary(value_type(c.target));
        else destination_destructor(value_type(c.target),facts[n].scope);
        user_conversions[c.materialization].temporary = transfer.temporary;
        if (class_value(returned)) user_conversions[c.materialization].source_temporary = temporary(returned);
        Expression value; value.type = value_type(returned);
        value.category = types[returned].kind == TypeKind::LRef ? ValueCategory::Lvalue :
            types[returned].kind == TypeKind::RRef ? ValueCategory::Xvalue : ValueCategory::Prvalue;
        Type ctor = types[entities[second.function].type];
        std::vector<NodeId> args(1,0);
        auto recipe = second.materialization;
        auto call = recipe ? conversion_objects[recipe].call : Expression();
        std::vector<Conversion> selected(1,recipe ? conversions[call.conversions] : standard_conversion(value,types.parameters[ctor.offset]));
        for (unsigned j = 1; j < ctor.count; ++j) {
            Conversion argument;
            NodeId arg = recipe ? call_argument(call,j) : default_argument(second.function,j,&argument);
            if (recipe) { default_argument(second.function,j); expression(arg,facts[n].scope); argument = copy_conversion_recipe(conversions[call.conversions+j]); }
            args.push_back(arg); selected.push_back(argument);
        }
        record_call(transfer.call,args,selected);
        user_conversions[c.materialization].result.materialization = conversion_objects.size();
        conversion_objects.push_back(transfer); demand_member(second.function);
    } else if (class_value(returned)) {
        if (use == ConversionUse::Temporary) user_conversions[c.materialization].temporary = temporary(returned);
        else destination_destructor(returned,facts[n].scope);
    }
    user_conversions[c.materialization].use = use;
    user_conversions[c.materialization].prepared = true;
}
} }
