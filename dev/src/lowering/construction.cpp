#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using syntax::Kind;
void Procedural::construct(EntityId ctor, NodeId init, Value object)
{
    if (!sem.constructor_needed(ctor)) return;
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(ctor))); call_work.push_back(object.operand);
    if (init) {
        auto fact = sem.expression_fact(init);
        for (unsigned j = 0; j < fact.argument_count; ++j)
            call_work.push_back(converted(sem.call_arguments[fact.arguments+j], sem.conversion_fact(fact.conversions+j)).operand);
    } else {
        auto e = sem.entities[ctor]; auto f = sem.types[e.type];
        for (unsigned j = 0; j < f.count; ++j)
            call_work.push_back(convert(expression(sem.default_arguments[e.defaults+j]), sem.types.parameters[f.offset+j]).operand);
    }
    guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
    call_work.resize(begin);
}
void Procedural::constructor_body(EntityId e)
{
    initialized_units = semantic::Index();
    auto m = sem.member_fact(e);
    for (unsigned j = 0; j < m.action_count; ++j) {
        auto action = sem.subobject_actions[m.action_begin+j];
        if (!action.initializer && !sem.constructor_needed(action.constructor)) { constructor_cleanup(action); continue; }
        auto t = sem.types[action.type];
        if (t.kind == TypeKind::Array && !action.initializer) {
            array_construct(action.constructor, action.type, Value(Operand::slot(this_slot), IRType::Ptr), true,
                {{action.field ? sem.entities[action.field].member_offset : 0, action.field != 0}});
            constructor_cleanup(action); continue;
        }
        if (action.initializer && (t.kind == TypeKind::Array || (t.kind == TypeKind::Named && sem.entities[t.entity].class_info)) &&
            !sem.facts[action.initializer].entity) {
            std::vector<InitProjection> path(1, {action.field ? sem.entities[action.field].member_offset : 0, action.field != 0, action.field});
            aggregate_initialize(action.initializer, action.type, Value(Operand::slot(this_slot), IRType::Ptr), true, path);
            clean_inline(live, 0); constructor_cleanup(action); continue;
        }
        bool scalar = t.kind != TypeKind::Array && !(t.kind == TypeKind::Named && sem.entities[t.entity].class_info);
        Value value;
        if (scalar && action.initializer) {
            NodeId n = action.initializer;
            while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) n = ast[n].first;
            value = initialization_value(n, action.type);
        }
        Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        Value at = emit(i, {base.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : 0)});
        at.type = action.type; at.address = true;
        if (sem.field_fact(action.field).bit_field) {
            at.bit_field = action.field; at.initializing = true; at.init_offset = sem.entities[action.field].member_offset;
        }
        if (scalar) store(value, at);
        else if (action.initializer) initialize(action.initializer, action.type, at);
        else { at.address = false; construct(action.constructor, 0, at); }
        clean_inline(live, 0); constructor_cleanup(action);
    }
}
} }

namespace cppgm { namespace lowering {
Value Procedural::initialization_address(Value root, bool indirect, const std::vector<InitProjection>& path)
{
    Value at = indirect ? emit(Opcode::Load, IRType::Ptr, {root.operand}) : root.address ? address(root) : root;
    std::uint64_t offset = 0;
    for (auto step : path) {
        Instruction i(Opcode::Index, IRType::I8); i.projection = step.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        at = emit(i, {at.operand, Operand::integer(step.offset)});
        offset += step.offset;
        if (sem.field_fact(step.entity).bit_field) at.bit_field = step.entity;
    }
    at.init_offset = offset; at.initializing = true;
    at.address = true; return at;
}
void Procedural::aggregate_initialize(NodeId n, TypeId t, Value root, bool indirect, std::vector<InitProjection>& path)
{
    using syntax::Kind;
    EntityId ctor = n && sem.constructor_member(sem.facts[n].entity) ? sem.facts[n].entity : !n ? sem.value_constructor(t) : 0;
    if (ctor) {
        Value at = initialization_address(root, indirect, path); at.address = false;
        construct(ctor, n, at); return;
    }
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    NodeId c = ast[n].first;
    if (target.kind == TypeKind::Named && sem.entities[target.entity].class_info) {
        for (auto d = sem.scopes[sem.entities[target.entity].scope].first_decl; d; d = sem.declarations[d].next) {
            EntityId field = sem.declarations[d].entity;
            if (!sem.nonstatic_field(field)) continue;
            auto member = sem.entities[field];
            path.push_back({member.member_offset, true, field});
            aggregate_initialize(c, member.type, root, indirect, path); path.pop_back();
            if (c) c = ast[c].next;
        }
        return;
    }
    if (target.kind == TypeKind::Array) {
        for (std::uint64_t j = 0; j < target.bound; ++j) {
            path.push_back({j*sem.object_size(target.child), false});
            aggregate_initialize(c, target.child, root, indirect, path); path.pop_back();
            if (c) c = ast[c].next;
        }
        return;
    }
    while (ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) n = ast[n].first;
    Value value = path.empty() || path.back().field ? initialization_value(n, t) :
        n ? incoming(n) : Value(type(t).floating() ? Operand::floating(0) : Operand::integer(0), type(t), t);
    Value at = initialization_address(root, indirect, path); at.type = t;
    store(value, at);
}
} }
