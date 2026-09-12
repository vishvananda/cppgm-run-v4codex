#include "semantic/analyzer.h"

namespace cppgm { namespace semantic {
std::uint32_t Analyzer::constant_constructor(EntityId ctor)
{
    if (auto known = constant_constructors.get(ctor)) return known;
    auto index = constructor_constants.size(); constructor_constants.push_back(ConstantObject());
    constant_constructors.put(ctor, index);
    auto member = members[entities[ctor].member_info];
    NodeId body = entities[ctor].body;
    // Keep source-declared copy/move value types on the ordinary O0 lifetime
    // path. Their initialization is not part of the early scalar-field policy.
    EntityId cls = scopes[entities[ctor].owner].entity;
    if (polymorphic(cls) || (class_facts[entities[cls].class_info].declared_transfers & 3)) return index;
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
        auto t = types[action.type];
        if (!action.field || !action.initializer || (t.cv & 2) || field_fact(action.field).bit_field ||
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
        constructor_constant_actions.push_back({action.field, action.type, source, parameter});
    }
    summary.count = constructor_constant_actions.size()-summary.first; summary.valid = true;
    constructor_constants[index] = summary; return index;
}
const ConstantObject& Analyzer::constant_construction(NodeId n, TypeId t)
{
    auto k = key(n,t);
    if (auto known = constant_objects.get(k)) return object_constants[known];
    ConstantObject result; result.first = constant_fields.size();
    EntityId ctor = facts[n].entity;
    if (constructor_member(ctor)) {
        auto summary = constructor_constants[constant_constructor(ctor)];
        auto call = expressions[n]; result.valid = summary.valid;
        // Even unused constructor arguments must be evaluated. The early
        // static form requires every argument, including defaults, to be a
        // side-effect-free static value after its selected conversion.
        for (unsigned j = 0; result.valid && j < call.argument_count; ++j) {
            auto conversion = conversions[call.conversions+j];
            result.valid = conversion.kind != Conversion::Kind::Construction &&
                static_value(call_arguments[call.arguments+j], conversion.target).kind != StaticValue::Invalid;
        }
        for (unsigned j = 0; result.valid && j < summary.count; ++j) {
            auto action = constructor_constant_actions[summary.first+j];
            NodeId source = action.argument ? call_arguments[call.arguments+action.argument-1] : action.source;
            if (action.argument) {
                auto converted = conversions[call.conversions+action.argument-1];
                // Do not collapse two different scalar conversions. The
                // bounded direct-forwarding summary requires equal value types.
                if (converted.kind == Conversion::Kind::Construction ||
                    types.unqualified(converted.target) != types.unqualified(action.type)) { result.valid = false; break; }
            }
            StaticValue value = static_value(source, action.type);
            result.valid = value.kind != StaticValue::Invalid;
            if (result.valid) constant_fields.push_back({action.field, action.type, value});
        }
    }
    if (!result.valid) constant_fields.resize(result.first);
    else result.count = constant_fields.size()-result.first;
    auto index = object_constants.size(); object_constants.push_back(result); constant_objects.put(k, index);
    return object_constants[index];
}
} }
