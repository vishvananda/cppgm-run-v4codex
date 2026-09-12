#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
void Procedural::construct(EntityId ctor, NodeId init, Value object, bool base)
{
    guard_expression(init,true);
    if (init && sem.object_fact(init).value_initialize) {
        auto cls = sem.scopes[sem.entities[ctor].owner].entity;
        zero_object(sem.entities[cls].type,object);
    }
    if (sem.direct_transfer(ctor)) {
        auto fact = sem.expression_fact(init);
        Value source = converted(sem.call_arguments[fact.arguments], sem.conversion_fact(fact.conversions));
        TypeId target = sem.entities[sem.scopes[sem.entities[ctor].owner].entity].type;
        Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(target); copy.alignment = sem.object_alignment(target);
        emit(copy, {source.operand, object.operand}); return;
    }
    if (!sem.constructor_needed(ctor)) return;
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(ctor, base))); call_work.push_back(object.operand);
    if (init) {
        auto fact = sem.expression_fact(init);
        for (unsigned j = 0; j < fact.argument_count; ++j)
            call_work.push_back(converted(sem.call_arguments[fact.arguments+j], sem.conversion_fact(fact.conversions+j)).operand);
    } else {
        auto e = sem.entities[ctor]; auto f = sem.types[e.type];
        for (unsigned j = 0; j < f.count; ++j)
            call_work.push_back(converted(sem.default_arguments[e.defaults+j],
                sem.conversion_fact(sem.member_fact(ctor).default_conversions+j)).operand);
    }
    guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
    call_work.resize(begin);
}
void Procedural::constructor_body(EntityId e)
{
    initialized_units = semantic::Index();
    auto m = sem.member_fact(e);
    if (m.delegated_constructor) {
        auto action = sem.subobject_actions[m.action_begin];
        begin_full_expression(action.initializer);
        Value object = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        construct(m.delegated_constructor, action.initializer, object);
        finish_full_expression(0); constructor_cleanup(action);
        return;
    }
    EntityId cls = sem.scopes[sem.entities[e].owner].entity;
    bool vptr_written = false;
    for (unsigned j = 0; j < m.action_count; ++j) {
        auto action = sem.subobject_actions[m.action_begin+j];
        if (action.field && !vptr_written) { vpointer_store(cls); vptr_written = true; }
        if (!action.initializer && !sem.constructor_needed(action.constructor)) { constructor_cleanup(action); continue; }
        begin_full_expression(action.initializer);
        auto t = sem.types[action.type];
        if (t.kind == TypeKind::Array && !action.initializer) {
            array_construct(action.constructor, action.type, Value(Operand::slot(this_slot), IRType::Ptr), true,
                {{action.field ? sem.entities[action.field].member_offset : sem.base_offset(sem.entities[cls].type), action.field != 0}});
            finish_full_expression(0);
            constructor_cleanup(action); continue;
        }
        if (action.initializer && (t.kind == TypeKind::Array || (t.kind == TypeKind::Named && sem.entities[t.entity].class_info)) &&
            !sem.facts[action.initializer].entity) {
            std::vector<InitProjection> path(1, {action.field ? sem.entities[action.field].member_offset : sem.base_offset(sem.entities[cls].type), action.field != 0, action.field});
            aggregate_initialize(action.initializer, action.type, Value(Operand::slot(this_slot), IRType::Ptr), true, path);
            finish_full_expression(0); constructor_cleanup(action); continue;
        }
        bool scalar = t.kind != TypeKind::Array && !(t.kind == TypeKind::Named && sem.entities[t.entity].class_info);
        Value value;
        if (scalar && action.initializer) {
            NodeId n = action.initializer;
            while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) n = ast[n].first;
            value = initialization_value(n, action.type);
        }
        auto field = sem.field_fact(action.field);
        if (scalar && field.unit_transfer) {
            Value at; at.type = action.type; at.bit_field = action.field; at.initializing = true;
            at.init_offset = sem.entities[action.field].member_offset;
            store_bit_field(value,at,this_slot);
            finish_full_expression(0); constructor_cleanup(action); continue;
        }
        Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        Value at = emit(i, {base.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : sem.base_offset(sem.entities[cls].type))});
        at.type = action.type; at.address = true;
        if (field.bit_field) {
            at.bit_field = action.field; at.initializing = true; at.init_offset = sem.entities[action.field].member_offset;
        }
        if (scalar) store(value, at);
        else if (action.initializer) {
            if (!action.field && sem.constructor_member(sem.facts[action.initializer].entity)) {
                at.address = false; construct(sem.facts[action.initializer].entity, action.initializer, at, true);
            } else initialize(action.initializer, action.type, at);
        }
        else if (!action.field && m.inherited_constructor) {
            std::size_t begin = call_work.size();
            call_work.push_back(Operand::symbol(symbol(m.inherited_constructor, true)));
            call_work.push_back(at.operand);
            for (auto d = sem.scopes[sem.entities[e].scope].first_decl; d; d = sem.declarations[d].next) {
                EntityId param = sem.declarations[d].entity;
                if (sem.entities[param].kind != semantic::EntityKind::Parameter) continue;
                call_work.push_back(emit(Opcode::Load, type(sem.entities[param].type), {Operand::slot(objects[param])}).operand);
            }
            guarded_call(Instruction(Opcode::Call, IRType::Void), call_work.data()+begin, call_work.size()-begin);
            call_work.resize(begin);
        }
        else { at.address = false; construct(action.constructor, 0, at, !action.field); }
        finish_full_expression(0); constructor_cleanup(action);
    }
    if (!vptr_written) vpointer_store(cls);
}
} }

namespace cppgm { namespace lowering {
Value Procedural::initialization_address(Value root, bool indirect, const std::vector<InitProjection>& path)
{
    Value at = indirect ? emit(Opcode::Load, IRType::Ptr, {root.operand}) : root.address ? address(root) : root;
    std::uint64_t offset = 0;
    for (auto step : path) {
        IRType element = step.element ? type(step.element) : IRType::I8;
        Instruction i(Opcode::Index, element); i.projection = step.element ? ir_model::IPK_ARRAY_ELEMENT : step.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        Operand index = Operand::integer(step.offset);
        if (element.kind() == IRType::Object) {
            index = emit(Opcode::Binary, IRType::I64, {index, Operand::integer(sem.object_size(step.element))}, Operation::Mul).operand;
            i.type = IRType::I8;
        }
        at = emit(i, {at.operand, index});
        offset += step.offset * (step.element ? sem.object_size(step.element) : 1);
        if (sem.field_fact(step.entity).bit_field) at.bit_field = step.entity;
    }
    at.init_offset = offset; at.initializing = true;
    at.address = true; return at;
}
void Procedural::aggregate_initialize(NodeId n, TypeId t, Value root, bool indirect, std::vector<InitProjection>& path)
{
    using syntax::Kind;
    if (auto source = sem.class_initialization(n,t).source) {
        Value at = initialization_address(root,indirect,path); at.address = false;
        construct_value(source,sem.conversion_fact(sem.class_initialization(n,t).conversion),at); return;
    }
    if (auto plan = sem.initializer_plan(n, t)) { aggregate_plan(plan, root, indirect, path); return; }
    EntityId ctor = n && sem.constructor_member(sem.facts[n].entity) ? sem.facts[n].entity : !n ? sem.value_constructor(t) : 0;
    if (ctor) {
        Value at = initialization_address(root, indirect, path); at.address = false;
        construct(ctor, n, at); return;
    }
    while (ast[n].kind == Kind::Initializer) n = ast[n].first;
    auto target = sem.types[t];
    if ((target.kind == TypeKind::Named && sem.entities[target.entity].class_info) || target.kind == TypeKind::Array)
        throw std::logic_error("missing aggregate initializer plan");
    while (ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments || ast[n].kind == Kind::BracedInit) n = ast[n].first;
    Value value = path.empty() || path.back().field ? initialization_value(n, t) :
        n ? incoming(n) : initialization_value(0,t);
    Value at = initialization_address(root, indirect, path); at.type = t;
    store(value, at);
}
} }
