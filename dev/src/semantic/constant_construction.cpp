#include "semantic/analyzer.h"
#include <algorithm>

namespace cppgm { namespace semantic {
const ConstantObject& Analyzer::constant_value_data(Constant value)
{
    auto k = key(value.type,value.bits);
    if (auto known = constant_value_objects.get(k)) return object_constants[known];
    ConstantObject result; result.first = constant_fields.size(); result.valid = constant_object_fields(value);
    if (!result.valid) constant_fields.resize(result.first);
    else {
        result.count = constant_fields.size()-result.first;
        std::stable_sort(constant_fields.begin()+result.first,constant_fields.end(),
            [](const ConstantField& a, const ConstantField& b) { return a.offset < b.offset; });
    }
    auto id = object_constants.size(); object_constants.push_back(result); constant_value_objects.put(k,id);
    return object_constants[id];
}
void Analyzer::prepare_static_vptrs()
{
    // Complete each declaration's default static-initialization fact once.
    // Implicit vptr-only construction is immediate; constexpr constructors
    // retain their evaluated subobjects, including active union members.
    Index seen;
    for (const auto& action : actions) {
        if (!action.object || !action.constructor) continue;
        if (seen.get(action.object)) continue;
        seen.put(action.object,1);
        auto object = entities[action.object];
        if (object.initializer || scopes[object.owner].kind != ScopeKind::Namespace || !class_value(object.type)) continue;
        auto ctor = members[entities[action.constructor].member_info];
        EntityId cls = types[object.type].entity;
        if (ctor.synthetic && ctor.transfer == TransferKind::None && !ctor.inherited_constructor &&
            !ctor.delegated_constructor && !ctor.action_count && dynamic_class(cls)) static_vptr_objects.put(action.object,cls);
        else if (entities[action.constructor].constexpr_function) {
            // A constant pointer to this must relocate to the declaration,
            // never to an anonymous evaluator temporary. Do not publish this
            // value as a constexpr variable: the source object can be mutable.
            auto destination = constant_entity_storage.get(action.object);
            if (!destination) {
                destination = constant_storage_address(object.type,Constant(),action.object,0,true);
                constant_entity_storage.put(action.object,destination);
            }
            auto saved = constant_destination; constant_destination = destination;
            Constant value;
            try { value = constant_construct(action.constructor,{}); }
            catch (...) { constant_destination = saved; throw; }
            constant_destination = saved;
            if (value.valid && constant_persistent(value)) {
                auto data = constant_value_data(value);
                if (data.valid) {
                    if (dynamic_class(cls) && size(object.type) == 8 && data.count == 1 &&
                        constant_fields[data.first].value.kind == StaticValue::Vtable)
                        static_vptr_objects.put(action.object,cls);
                    else static_construction_objects.put(action.object,constant_value_objects.get(key(value.type,value.bits)));
                }
            }
        }
    }
}
std::uint32_t Analyzer::constant_constructor(EntityId ctor)
{
    EvaluationScope runtime(*this,false);
    if (auto known = constant_constructors.get(ctor)) return known;
    auto index = constructor_constants.size(); constructor_constants.push_back(ConstantObject());
    constant_constructors.put(ctor, index);
    auto member = members[entities[ctor].member_info];
    NodeId body = entities[ctor].body;
    // Keep source-declared copy/move value types on the ordinary O0 lifetime
    // path. Their initialization is not part of the early scalar-field policy.
    EntityId cls = scopes[entities[ctor].owner].entity;
    if (dynamic_class(cls) || (class_facts[entities[cls].class_info].declared_transfers & 3)) return index;
    // Early static initialization is permitted only when the demanded body
    // has no effects and every action initializes this object's scalar fields.
    // Other bodies retain their ordinary dynamic initialization path.
    if (member.transfer != TransferKind::None || member.inherited_constructor || ast[body].kind != syntax::Kind::Compound || ast[body].first) return index;
    Index parameters;
    unsigned number = 0;
    for (auto d = scopes[entities[ctor].scope].first_decl; d; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (entities[e].kind == EntityKind::Parameter) parameters.put(e, ++number);
    }
    ConstantObject summary; summary.first = constructor_constant_actions.size();
    for (unsigned j = 0; j < member.action_count; ++j) {
        auto action = subobject_actions[member.action_begin+j];
        SourceInvocationScope invocation(source_invocation,action.source_site,action.source_site != 0,true);
        auto t = types[action.type];
        if (!action.field || !action.initializer || (t.cv & 6) || field_fact(action.field).bit_field ||
            t.kind == TypeKind::Array || t.kind == TypeKind::LRef || t.kind == TypeKind::RRef ||
            (t.kind == TypeKind::Named && entities[t.entity].class_info)) {
            constructor_constant_actions.resize(summary.first); return index;
        }
        NodeId source = action.initializer;
        while (ast[source].kind == syntax::Kind::Initializer || ast[source].kind == syntax::Kind::ParenArguments ||
            ast[source].kind == syntax::Kind::ParenInitializer || ast[source].kind == syntax::Kind::BracedInit || ast[source].kind == syntax::Kind::Parenthesized)
            source = ast[source].first;
        unsigned parameter = ast[source].kind == syntax::Kind::IdExpression ? parameters.get(expressions[source].entity) : 0;
        if (!parameter && static_value(source, action.type).kind == StaticValue::Invalid) {
            constructor_constant_actions.resize(summary.first); return index;
        }
        constructor_constant_actions.push_back({action.field, action.type, source, parameter, action.source_site});
    }
    summary.count = constructor_constant_actions.size()-summary.first; summary.valid = true;
    constructor_constants[index] = summary; return index;
}
const ConstantObject& Analyzer::constant_construction(NodeId n, TypeId t)
{
    SourceInvocationScope invocation(source_invocation,source_site(n));
    auto k = key(n,t);
    if (auto known = source_invocation.defaulted ? 0 : constant_objects.get(k)) return object_constants[known];
    ConstantObject result; result.first = constant_fields.size();
    EntityId ctor = facts[n].entity;
    if (constructor_member(ctor) && entities[ctor].constexpr_function) {
        auto value = constant_initialize(n,t,facts[n].scope);
        result.valid = constant_object_fields(value);
    } else if (constructor_member(ctor)) {
        // Early static initialization of a non-constexpr constructor must
        // preserve the value its dynamic execution would have produced.
        EvaluationScope runtime(*this,false);
        auto summary = constructor_constants[constant_constructor(ctor)];
        auto call = expressions[n]; result.valid = summary.valid;
        // Even unused constructor arguments must be evaluated. The early
        // static form requires every argument, including defaults, to be a
        // side-effect-free static value after its selected conversion.
        for (unsigned j = 0; result.valid && j < call.argument_count; ++j) {
            auto conversion = conversions[call.conversions+j];
            SourceInvocationScope argument(source_invocation,0,conversion.default_argument);
            result.valid = conversion.kind != Conversion::Kind::Construction &&
                static_value(call_argument(call,j), conversion.target).kind != StaticValue::Invalid;
        }
        for (unsigned j = 0; result.valid && j < summary.count; ++j) {
            auto action = constructor_constant_actions[summary.first+j];
            SourceInvocationScope source_context(source_invocation,action.source_site,action.source_site != 0,action.source_site != 0);
            NodeId source = action.argument ? call_argument(call,action.argument-1) : action.source;
            if (action.argument) {
                auto converted = conversions[call.conversions+action.argument-1];
                // Do not collapse two different scalar conversions. The
                // bounded direct-forwarding summary requires equal value types.
                if (converted.kind == Conversion::Kind::Construction ||
                    types.unqualified(converted.target) != types.unqualified(action.type)) { result.valid = false; break; }
            }
            SourceInvocationScope argument_context(source_invocation,0,
                action.argument && conversions[call.conversions+action.argument-1].default_argument);
            StaticValue value = static_value(source, action.type);
            result.valid = value.kind != StaticValue::Invalid;
            if (result.valid) constant_fields.push_back({action.field, action.type, value,entities[action.field].member_offset});
        }
    }
    if (!result.valid) constant_fields.resize(result.first);
    else {
        result.count = constant_fields.size()-result.first;
        std::stable_sort(constant_fields.begin()+result.first,constant_fields.end(),
            [](const ConstantField& a, const ConstantField& b) { return a.offset < b.offset; });
    }
    auto index = object_constants.size(); object_constants.push_back(result); if (!source_invocation.defaulted) constant_objects.put(k, index);
    return object_constants[index];
}
} }
