#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
std::uint32_t Analyzer::constant_body(EntityId e)
{
    if (!e || (!entities[e].constexpr_function && !synthetic_member(e))) return 0;
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
            if (m.body)
                function_body({m.body,m.declarator,m.body_environment ? m.body_environment : entities[e].owner,e,m.source});
        }
        // Evaluation consumes checked expressions, not runtime jump/cleanup facts.
    } catch (...) { active_constant = saved; throw; }
    active_constant = saved;
    if (constructor_member(e) && synthetic_member(e)) {
        constructor_actions(e);
    } else if (entities[e].body_state != FactState::Success) return 0;

    ConstantBody result; result.function = e; result.parameters = constant_parameters.size();
    result.valid = !types[entities[e].type].variadic;
    for (auto d = scopes[entities[e].scope].first_decl; d; d = declarations[d].next) {
        auto parameter = declarations[d].entity;
        if (entities[parameter].kind != EntityKind::Parameter) continue;
        ++result.count;
        constant_parameters.push_back(parameter);
    }
    result.statement = entities[e].body;
    result.valid &= synthetic_member(e) || ast[result.statement].kind == Kind::Compound;
    auto id = constant_bodies.size(); constant_bodies.push_back(result); constant_body_index.put(e,id);
    return id;
}
Constant Analyzer::execute_constant(EntityId e, const std::vector<Constant>& args, std::uint32_t object, bool zero)
{
    if (!constant_depth) { constant_remaining = 1000000; constant_limited = constant_unavailable = false; }
    auto body_id = constant_body(e);
    if (!body_id) { constant_unavailable = true; return Constant(); }
    auto body = constant_bodies[body_id];
    if (!body.valid || body.count != args.size()) return Constant();
    if (entities[e].member_info && !entities[e].is_static && !object && !constructor_member(e)) return Constant();
    // The activation key contains typed values and the receiver path, plus
    // snapshots of mutable or retired storage reachable through addresses.
    std::vector<ArgumentId> key_args; key_args.reserve(args.size()+2); key_args.push_back(object);
    key_args.push_back(zero);
    Index dependencies;
    if (object) constant_dependencies(Constant(types.compound(TypeKind::LRef,constant_addresses[object].type),object),key_args,dependencies);
    for (unsigned i = 0; i < args.size(); ++i) {
        auto value = convert(args[i],entities[constant_parameters[body.parameters+i]].type);
        if (!value.valid) return Constant();
        TypeQuery q; q.type = types.unqualified(value.type); q.value = value.bits;
        key_args.push_back(intern_query(q,{}) | 0x80000000U);
        constant_dependencies(value,key_args,dependencies);
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
        if (constructor_member(e)) {
            auto member = members[entities[e].member_info];
            auto t = entities[scopes[entities[e].owner].entity].type;
            ConstantBuilder builder;
            auto& parts = builder.parts;
            auto receiver = object ? object : constant_storage_address(t,Constant(),0,0,true);
            auto storage = constant_addresses[receiver].storage;
            constant_storage[storage].builder = &builder;
            constant_activations[id].object = receiver;
            struct Building {
                Analyzer& sem; std::uint32_t storage;
                ~Building(){sem.constant_storage[storage].builder=0;}
            } building{*this,storage};
            if (zero) {
                auto v = constant_zero(t); auto o = evaluated_objects[v.bits];
                parts.assign(evaluated_parts.begin()+o.first,evaluated_parts.begin()+o.first+o.count);
            }
            auto& slots = builder.slots;
            for (unsigned i = 0; i < parts.size(); ++i) slots.put(parts[i].selector,i+1);
            bool valid = true;
            for (unsigned j = 0; valid && j < member.action_count; ++j) {
                auto action = subobject_actions[member.action_begin+j];
                auto v = constant_initialize(action.initializer,action.type,entities[e].scope,action.initializer ? 0 : action.constructor);
                if (!v.valid) { valid = false; break; }
                if (member.delegated_constructor) { value = v; break; }
                EvaluatedPart p; p.selector = action.field ? action.field : (0x80000000U | types[action.type].entity); p.value = v;
                auto slot = slots.get(p.selector);
                if (slot) parts[slot-1] = p;
                else { slots.put(p.selector,parts.size()+1); parts.push_back(p); }
                ++constant_storage[storage].version;
            }
            if (valid && !member.delegated_constructor) value = evaluated_object(t,parts);
            constant_storage[storage].value = value;
            constant_storage[storage].readable = value.valid;
            if (valid && !synthetic_member(e)) {
                auto result = execute_constant_statement(body.statement,entities[e].scope);
                if (result.flow != ConstantFlow::Next) value = Constant();
            }
        } else {
            auto result = execute_constant_statement(body.statement,entities[e].scope);
            if (result.flow == ConstantFlow::Return) value = result.value;
        }
    } catch (...) {
        constant_activations[id].state = FactState::Failure;
        active_constant = saved; constant_frame = saved_frame; --constant_depth; throw;
    }
    for (auto storage : frame.storage) constant_storage[storage].live = false;
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
    if (expressions[n].form == ExpressionForm::OperatorCall) return constant_indirect(constant_call(n,s));
    return evaluate_value(n,s);
}
Constant Analyzer::constant_call(NodeId n, ScopeId s)
{
    auto e = facts[n].entity;
    auto call = expressions[n];
    if (call.form == ExpressionForm::Expect) {
        auto first = constant_node_conversion(call_argument(call,0),conversions[call.conversions],s);
        auto second = constant_node_conversion(call_argument(call,1),conversions[call.conversions+1],s);
        return second.valid ? first : Constant();
    }
    if (!e || entities[e].kind != EntityKind::Function) {
        auto use = object_uses[call.object_use];
        auto callee = ast[n].first;
        auto v = use.callee_conversion ? constant_node_conversion(callee,conversions[use.callee_conversion],s) : evaluate(callee,s);
        if (!v.valid || !v.bits || !pointer(v.type)) return Constant();
        e = constant_storage[constant_addresses[v.bits].storage].entity;
    }
    if (!e || entities[e].kind != EntityKind::Function || !entities[e].constexpr_function) return Constant();
    if (entities[e].is_static && object_uses[call.object_use].node &&
        !constant_arrow(object_uses[call.object_use].node,object_uses[call.object_use].arrow)) return Constant();
    std::uint32_t object = 0;
    if (entities[e].member_info && !entities[e].is_static) {
        auto use = object_uses[call.object_use];
        if (use.virtual_slot || use.member_pointer) return Constant();
        if (use.node) {
            object = constant_arrow(use.node,use.arrow);
        } else if (active_constant) object = constant_activations[active_constant].object;
        object = constant_base_address(object,entities[scopes[entities[e].owner].entity].type);
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
std::uint32_t Analyzer::constant_query_object(QueryId id)
{
    if (auto known = constant_query_receivers.get(id)) return known;
    auto q = type_queries[id]; auto fact = query_fact(id);
    if (q.kind == QueryKind::Parenthesized) return constant_query_object(query_edges[q.offset]);
    if (q.kind == QueryKind::Name || q.kind == QueryKind::QualifiedValue) return constant_entity_address(fact.expression.entity);
    if (q.kind == QueryKind::Member) {
        if (!nonstatic_field(fact.expression.entity)) return constant_entity_address(fact.expression.entity);
        auto parent = constant_query_object(query_edges[q.offset]);
        auto field = fact.expression.entity;
        parent = constant_base_address(parent,entities[scopes[entities[field].owner].entity].type);
        return constant_subobject(parent,entities[field].type,field);
    }
    auto v = constants[query_value(id)];
    if (!v.valid) return 0;
    if (types[v.type].kind == TypeKind::LRef || types[v.type].kind == TypeKind::RRef) return v.bits;
    auto result = constant_storage_address(v.type,v);
    constant_query_receivers.put(id,result); return result;
}
std::uint32_t Analyzer::constant_node_object(NodeId n)
{
    if (pointer(expressions[n].type)) {
        auto value = evaluate(n,facts[n].scope); return value.valid ? value.bits : 0;
    }
    return constant_address(n,facts[n].scope);
}
Constant Analyzer::constant_node_conversion(NodeId n, Conversion c, ScopeId s)
{
    if (c.kind == Conversion::Kind::Discarded) {
        auto value = evaluate(n,s);
        return value.valid ? Constant(types.fundamental(FT_VOID),0) : Constant();
    }
    if (ast[n].kind == Kind::BracedInit && !ast[n].first && (integral(c.target) || floating_type(c.target)))
        return convert(Constant(types.fundamental(FT_INT),0),c.target,true);
    if (c.kind == Conversion::Kind::User) {
        auto object = constant_node_object(n);
        object = constant_base_address(object,entities[scopes[entities[c.function].owner].entity].type);
        if (!object || members[entities[c.function].member_info].virtual_member) return Constant();
        return convert(execute_constant(c.function,{},object),c.target,true);
    }
    if (c.kind == Conversion::Kind::Construction) {
        auto material = conversion_objects[c.materialization];
        auto call = material.call; std::vector<Constant> args;
        for (unsigned i = 0; i < call.argument_count; ++i) {
            auto value = constant_node_conversion(call_argument(call,i),conversions[call.conversions+i],s);
            if (!value.valid) return Constant();
            args.push_back(value);
        }
        auto value = constant_construct(material.constructor,args);
        return c.reference && value.valid ? Constant(c.target,constant_storage_address(value.type,value)) : value;
    }
    if (c.kind == Conversion::Kind::List) {
        auto object = list_objects[c.materialization]; auto plan = list_plans[object.plan];
        if (plan.direct_binding) return constant_node_conversion(call_argument(object.call),conversions[object.call.conversions],s);
        Constant value;
        if (plan.aggregate) value = constant_init_plan(object.initializer,s);
        else if (plan.constructor) {
            std::vector<Constant> args;
            for (unsigned i = 0; i < object.call.argument_count; ++i) args.push_back(constant_node_conversion(call_argument(object.call,i),conversions[object.call.conversions+i],s));
            value = constant_construct(plan.constructor,args,plan.zero);
        } else if (object.call.argument_count) value = constant_node_conversion(call_argument(object.call),conversions[object.call.conversions],s);
        else value = constant_zero(value_type(c.target));
        if (c.reference && value.valid) return Constant(c.target,constant_storage_address(value.type,value));
        return value;
    }
    auto target = types[c.target];
    if (fundamental(c.target,FT_BOOL) && (types[expressions[n].type].kind == TypeKind::Array || types[expressions[n].type].kind == TypeKind::Function)) {
        auto address = constant_address(n,s);
        return address ? Constant(c.target,1) : Constant();
    }
    if (target.kind == TypeKind::LRef || target.kind == TypeKind::RRef || c.reference) {
        auto address = constant_address(n,s);
        if (address && class_value(target.child)) address = constant_base_address(address,target.child);
        return address ? Constant(c.target,address) : Constant();
    }
    if (target.kind == TypeKind::Pointer && (types[expressions[n].type].kind == TypeKind::Array || types[expressions[n].type].kind == TypeKind::Function)) {
        auto address = constant_address(n,s);
        if (types[expressions[n].type].kind == TypeKind::Array) address = constant_subobject(address,target.child,0);
        return address ? Constant(c.target,address) : Constant();
    }
    return convert(evaluate(n,s),c.target,true);
}
Constant Analyzer::constant_call_result(NodeId n, ScopeId s)
{
    auto x = expressions[n];
    if (x.form == ExpressionForm::Construction) return constant_initialize(n,x.type,s);
    if (x.form == ExpressionForm::ListValue) {
        auto args = ast[ast[n].first].next;
        return constant_node_conversion(args,conversions[x.conversions],s);
    }
    return constant_call(n,s);
}
Constant Analyzer::constant_construct(EntityId e, const std::vector<Constant>& args, bool zero)
{
    if (!e) return Constant();
    auto member = members[entities[e].member_info];
    if (member.synthetic && member.transfer != TransferKind::None) {
        if (!constexpr_constructor(e) || args.size() != 1) return Constant();
        return constant_indirect(args[0]);
    }
    if (!constexpr_constructor(e) && !(zero && member.synthetic)) return Constant();
    auto destination = constant_destination;
    if (destination && types.unqualified(constant_addresses[destination].type) != entities[scopes[entities[e].owner].entity].type) destination = 0;
    if (!destination) destination = constant_storage_address(entities[scopes[entities[e].owner].entity].type,Constant());
    auto saved = constant_destination; constant_destination = 0;
    try { auto value = execute_constant(e,args,destination,zero); constant_destination = saved; return value; }
    catch (...) { constant_destination = saved; throw; }
}
Constant Analyzer::constant_query_conversion(QueryId source, Conversion c)
{
    if (c.kind == Conversion::Kind::User) {
        auto object = constant_query_object(source);
        object = constant_base_address(object,entities[scopes[entities[c.function].owner].entity].type);
        if (!object || members[entities[c.function].member_info].virtual_member) return Constant();
        return convert(execute_constant(c.function,{},object),c.target,true);
    }
    if (c.kind == Conversion::Kind::Construction) {
        auto material = conversion_objects[c.materialization];
        auto call = material.call;
        if (call.argument_count != 1) return Constant();
        auto v = constant_query_conversion(source,conversions[call.conversions]);
        return v.valid ? constant_construct(material.constructor,{v}) : Constant();
    }
    auto t = types[c.target]; auto from = query_fact(source).expression.type;
    if (t.kind == TypeKind::LRef || t.kind == TypeKind::RRef ||
        (t.kind == TypeKind::Pointer && (types[from].kind == TypeKind::Array || types[from].kind == TypeKind::Function))) {
        auto address = constant_query_object(source);
        if (t.kind == TypeKind::Pointer && types[from].kind == TypeKind::Array) address = constant_subobject(address,t.child,0);
        if (class_value(t.child)) address = constant_base_address(address,t.child);
        return address ? Constant(c.target,address) : Constant();
    }
    return convert(constants[query_value(source)],c.target,true);
}
Constant Analyzer::constant_query_call(QueryId id)
{
    auto q = type_queries[id]; auto fact = query_fact(id);
    auto e = fact.selected;
    if (!e || (!entities[e].constexpr_function && !synthetic_member(e))) return Constant();
    std::uint32_t object = 0;
    auto callee = type_queries[query_edges[q.offset]];
    if (entities[e].member_info && !entities[e].is_static && !constructor_member(e)) {
        if (callee.kind != QueryKind::Member || callee.op == OP_ARROW) return Constant();
        object = constant_query_object(query_edges[callee.offset]);
        if (!object) return Constant();
    }
    auto count = types[entities[e].type].count;
    auto offset = fact.expression.count-count;
    std::vector<Constant> args;
    for (unsigned i = 0; i < count; ++i) {
        auto c = conversions[fact.expression.conversions+offset+i];
        Constant value;
        if (i+1 < q.count) value = constant_query_conversion(query_edges[q.offset+i+1],c);
        else { auto node = default_argument(e,i,0,DefaultReason::Recipe); value = evaluate(node,facts[node].scope); }
        value = convert(value,c.target);
        if (!value.valid) return Constant();
        args.push_back(value);
    }
    if (constructor_member(e)) return constant_construct(e,args,q.count == 1);
    return execute_constant(e,args,object);
}
} }
