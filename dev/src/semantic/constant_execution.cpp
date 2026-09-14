#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
std::uint32_t Analyzer::constant_body(EntityId e)
{
    if (!e || !entities[e].constexpr_function) return 0;
    if (auto known = constant_body_index.get(e)) return known;
    if (entities[e].body_state == FactState::Active) return 0;
    // Establish the selected definition without requesting its emission. Body
    // checking must not inherit an enclosing constant call's parameter values.
    auto saved = active_constant; active_constant = 0;
    try {
        if (entities[e].specialization) instantiate_function(e);
        if (entities[e].member_info && !entities[e].definition) {
            instantiate_member_definition(e);
            auto m = members[entities[e].member_info];
            if (m.body && entities[scopes[entities[e].owner].entity].complete)
                function_body({m.body,m.declarator,m.body_environment ? m.body_environment : entities[e].owner,e,m.source});
        }
        if (entities[e].body_state == FactState::Success) finish_body(e);
    } catch (...) { active_constant = saved; throw; }
    active_constant = saved;
    if (entities[e].body_state != FactState::Success) return 0;
    require_body_facts(e);
    ConstantBody result; result.function = e; result.parameters = constant_parameters.size();
    result.valid = (integral(types[entities[e].type].child) || floating_type(types[entities[e].type].child)) && !types[entities[e].type].variadic;
    for (auto d = scopes[entities[e].scope].first_decl; d; d = declarations[d].next) {
        auto parameter = declarations[d].entity;
        if (entities[parameter].kind != EntityKind::Parameter) continue;
        result.valid &= (integral(entities[parameter].type) || floating_type(entities[parameter].type)) &&
            types[entities[parameter].type].kind != TypeKind::LRef && types[entities[parameter].type].kind != TypeKind::RRef;
        ++result.count;
        constant_parameters.push_back(parameter);
    }
    result.statement = entities[e].body;
    result.valid &= ast[result.statement].kind == Kind::Compound;
    auto id = constant_bodies.size(); constant_bodies.push_back(result); constant_body_index.put(e,id);
    return id;
}
Constant Analyzer::execute_constant(EntityId e, const std::vector<Constant>& args, std::uint32_t object)
{
    if (!constant_depth) { constant_remaining = 1000000; constant_limited = constant_unavailable = false; }
    auto body_id = constant_body(e);
    if (!body_id) { constant_unavailable = true; return Constant(); }
    auto body = constant_bodies[body_id];
    if (!body.valid || body.count != args.size()) return Constant();
    if (entities[e].member_info && !entities[e].is_static && !object) return Constant();
    // Slot zero is the receiver identity; the remaining slots are scalar
    // value identities. This private activation pack never enters deduction.
    std::vector<ArgumentId> key_args(1,object);
    for (unsigned i = 0; i < args.size(); ++i) {
        auto value = convert(args[i],entities[constant_parameters[body.parameters+i]].type);
        if (!value.valid) return Constant();
        TypeQuery q; q.type = types.unqualified(value.type); q.value = value.bits;
        key_args.push_back(intern_query(q,{}) | 0x80000000U);
    }
    auto pack = intern_arguments(key_args);
    auto key_value = key(body_id,pack);
    auto id = constant_activation_index.get(key_value);
    if (id && constant_activations[id].state != FactState::NotStarted) {
        ++constant_hits;
        // Recursive demand of the same immutable frame has no constant value.
        return constant_activations[id].state == FactState::Success ? constant_activations[id].result : Constant();
    }
    if (constant_depth == 512) { constant_limited = true; return Constant(); }
    ConstantActivation activation; activation.body = body_id; activation.arguments = pack; activation.object = object;
    if (!id) { id = constant_activations.size(); constant_activations.push_back(activation); constant_activation_index.put(key_value,id); }
    else constant_activations[id] = activation;
    ConstantFrame frame;
    for (unsigned i = 0; i < args.size(); ++i) {
        auto parameter = constant_parameters[body.parameters+i];
        frame.bindings.put(parameter,frame.values.size());
        frame.values.push_back(convert(args[i],entities[parameter].type));
    }
    auto saved = active_constant; auto saved_frame = constant_frame;
    active_constant = id; constant_frame = &frame; ++constant_depth;
    Constant value;
    try {
        auto result = execute_constant_statement(body.statement,entities[e].scope);
        if (result.flow == ConstantFlow::Return) value = result.value;
    } catch (...) {
        constant_activations[id].state = FactState::Failure;
        active_constant = saved; constant_frame = saved_frame; --constant_depth; throw;
    }
    constant_activations[id].result = value;
    // Exhaustion and not-yet-defined constants are unavailable prerequisites,
    // not semantic failures of this immutable key. A later shallower request
    // or definition may succeed; completed independent values remain reusable.
    constant_activations[id].state = value.valid ? FactState::Success :
        (constant_limited || constant_unavailable) ? FactState::NotStarted : FactState::Failure;
    active_constant = saved; constant_frame = saved_frame; --constant_depth;
    return value;
}
Constant Analyzer::execute_constant_node(NodeId n, ScopeId s)
{
    if (!constant_step()) return Constant();
    // Values are recomputed on each executed visit: locals and parameters can
    // change within an activation. Only immutable semantic query facts bypass
    // execution. Completed calls remain memoized by their complete input key.
    if (expressions[n].form == ExpressionForm::ConstantQuery && facts[n].value) return constants[facts[n].value];
    if (expressions[n].form == ExpressionForm::OperatorCall) return Constant();
    return evaluate_value(n,s);
}
Constant Analyzer::constant_call(NodeId n, ScopeId s)
{
    auto e = facts[n].entity;
    if (!e || entities[e].kind != EntityKind::Function || !entities[e].constexpr_function) return Constant();
    auto call = expressions[n];
    std::uint32_t object = 0;
    if (entities[e].member_info && !entities[e].is_static) {
        auto use = object_uses[call.object_use];
        if (use.virtual_slot || use.member_pointer) return Constant();
        if (use.node) {
            object = constant_node_object(use.node);
        } else if (active_constant) object = constant_activations[active_constant].object;
        if (!object) return Constant();
    }
    std::vector<Constant> args;
    for (unsigned i = 0; i < call.argument_count; ++i) {
        auto conversion = conversions[call.conversions+i];
        auto value = constant_node_conversion(call_argument(call,i),conversion,s);
        if (!value.valid) return Constant();
        args.push_back(value);
    }
    return execute_constant(e,args,object);
}
bool Analyzer::constant_receiver_type(TypeId type)
{
    if (!class_value(type)) return false;
    auto cls = types[type].entity;
    return class_facts[entities[cls].class_info].aggregate &&
        !class_facts[entities[cls].class_info].first_base && empty_class(type) &&
        !polymorphic(cls) && trivial_destructor(type) && !(types[type].cv & 2);
}
std::uint32_t Analyzer::constant_query_object(QueryId id)
{
    if (auto known = constant_query_receivers.get(id)) return known-1;
    auto q = type_queries[id];
    if (q.kind == QueryKind::Parenthesized) return constant_query_object(query_edges[q.offset]);
    if (q.kind != QueryKind::Call || q.count != 1) return 0;
    auto fact = query_fact(id); auto type = fact.expression.type;
    if (!class_value(type) || !fact.selected || !constructor_member(fact.selected)) return 0;
    // The PA15 object subset is stateless literal temporaries with implicit
    // construction. Nonempty objects/constructor execution belong to PA16.
    std::uint32_t result = 0;
    if (members[entities[fact.selected].member_info].synthetic && constant_receiver_type(type)) {
        result = constant_receivers.size(); constant_receivers.push_back({type,0,id});
    }
    constant_query_receivers.put(id,result+1); return result;
}
std::uint32_t Analyzer::constant_node_object(NodeId n)
{
    if (auto known = constant_node_receivers.get(n)) return known-1;
    if (ast[n].kind == Kind::Parenthesized) return constant_node_object(ast[n].first);
    auto x = expressions[n];
    std::uint32_t result = 0;
    if (ast[n].kind == Kind::Call && !ast[ast[ast[n].first].next].first &&
        (x.form == ExpressionForm::Construction || x.form == ExpressionForm::ListValue) && constant_receiver_type(x.type)) {
        result = constant_receivers.size(); constant_receivers.push_back({x.type,n,0});
    }
    constant_node_receivers.put(n,result+1); return result;
}
Constant Analyzer::constant_node_conversion(NodeId n, Conversion c, ScopeId s)
{
    if (c.kind == Conversion::Kind::User) {
        auto object = constant_node_object(n);
        if (!object || members[entities[c.function].member_info].virtual_member) return Constant();
        return convert(execute_constant(c.function,{},object),c.target,true);
    }
    if (c.kind == Conversion::Kind::Construction) return Constant();
    return convert(evaluate(n,s),c.target,true);
}
Constant Analyzer::constant_query_conversion(QueryId source, Conversion c)
{
    auto object = constant_query_object(source);
    if (c.kind != Conversion::Kind::User || !object) return Constant();
    if (members[entities[c.function].member_info].virtual_member) return Constant();
    return execute_constant(c.function,{},object);
}
Constant Analyzer::constant_query_call(QueryId id)
{
    auto q = type_queries[id]; auto fact = query_fact(id);
    auto e = fact.selected;
    if (!e || !entities[e].constexpr_function || constructor_member(e)) return Constant();
    std::uint32_t object = 0;
    auto callee = type_queries[query_edges[q.offset]];
    if (entities[e].member_info && !entities[e].is_static) {
        if (callee.kind != QueryKind::Member || callee.op == OP_ARROW) return Constant();
        object = constant_query_object(query_edges[callee.offset]);
        if (!object) return Constant();
    }
    auto count = types[entities[e].type].count;
    auto offset = fact.expression.count-count;
    std::vector<Constant> args;
    for (unsigned i = 0; i < count; ++i) {
        auto c = conversions[fact.expression.conversions+offset+i];
        if (c.kind == Conversion::Kind::Construction) return Constant();
        Constant value;
        if (i+1 < q.count) value = c.kind == Conversion::Kind::User ?
            constant_query_conversion(query_edges[q.offset+i+1],c) : constants[query_value(query_edges[q.offset+i+1])];
        else { auto node = default_argument(e,i,0,DefaultReason::Recipe); value = evaluate(node,facts[node].scope); }
        value = convert(value,c.target);
        if (!value.valid) return Constant();
        args.push_back(value);
    }
    return execute_constant(e,args,object);
}
} }
