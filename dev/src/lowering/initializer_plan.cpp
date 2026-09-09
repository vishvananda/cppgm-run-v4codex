#include "lowering/procedural.h"
#include <cstring>
#include <algorithm>
namespace cppgm { namespace lowering {
using semantic::InitKind;
using lowir_model::DataItem;
Value Procedural::string_element(NodeId n, TypeId t, std::uint64_t index)
{
    auto literal = ast.literals[ast[n].literal];
    std::uint64_t bits = 0;
    if (index < literal.elements) {
        auto width = fundamental_width(literal.type);
        std::memcpy(&bits, ast.literal_bytes.data()+literal.offset+index*width, width);
    }
    return Value(Operand::integer(bits), type(t), t);
}
bool Procedural::constant_plan(std::uint32_t plan)
{
    auto action = sem.initializers[plan];
    if (action.kind == InitKind::Constructor) return false;
    if (action.kind == InitKind::Value) return constant_initializer(0, action.type);
    if (action.kind == InitKind::String) return true;
    if (action.kind == InitKind::Scalar) return sem.static_value(action.source, action.type).kind != semantic::StaticValue::Invalid;
    for (auto child = action.first; child; child = sem.initializers[child].next)
        if (!constant_plan(child)) return false;
    return true;
}
void Procedural::global_plan(std::uint32_t plan)
{
    auto action = sem.initializers[plan];
    auto target = sem.types[action.type];
    if (action.kind == InitKind::Scalar) {
        auto item = constant_data(action.source, action.type);
        if (item.type == IRType::Ptr && item.kind == DataItem::Scalar && !item.value.data.integer) {
            item.kind = DataItem::Zero; item.zero_bytes = 8;
        }
        p.data.push_back(item); return;
    }
    if (action.kind == InitKind::Value) { global_data(0, action.type); return; }
    if (action.kind == InitKind::String) {
        std::uint64_t length = ast.literals[ast[action.source].literal].elements;
        auto limit = target.bound-length > 8 ? length : target.bound;
        for (std::uint64_t j = 0; j < limit; ++j) {
            DataItem item; item.kind = DataItem::Scalar; item.type = type(target.child);
            item.value = string_element(action.source, target.child, j).operand; p.data.push_back(item);
        }
        if (limit < target.bound) { DataItem zero; zero.zero_bytes = (target.bound-limit)*sem.object_size(target.child); p.data.push_back(zero); }
        return;
    }
    std::uint64_t bytes = 0;
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        auto at = item.field ? sem.entities[item.field].member_offset : item.index * sem.object_size(item.type);
        if (at > bytes) { DataItem zero; zero.zero_bytes = at-bytes; p.data.push_back(zero); }
        if (item.count > 8 && item.kind == InitKind::Value && sem.zero_value(item.type)) {
            DataItem zero; zero.zero_bytes = item.count*sem.object_size(item.type); p.data.push_back(zero);
        } else for (std::uint64_t j = 0; j < item.count; ++j) global_plan(child);
        bytes = at + item.count*sem.object_size(item.type);
    }
    auto total = sem.object_size(action.type);
    if (bytes < total) { DataItem zero; zero.zero_bytes = total-bytes; p.data.push_back(zero); }
}
void Procedural::initialize_plan(std::uint32_t plan, Value location)
{
    auto action = sem.initializers[plan];
    auto target = sem.types[action.type];
    if (action.kind == InitKind::Scalar) {
        Value value = action.source ? incoming(action.source) : Value(Operand::integer(0), type(action.type), action.type);
        store(value, location); return;
    }
    if (action.kind == InitKind::Constructor) { construct(sem.facts[action.source].entity, action.source, address(location)); return; }
    if (action.kind == InitKind::Value) {
        std::vector<InitProjection> path;
        aggregate_initialize(0, action.type, location, false, path); return;
    }
    Value base = address(location);
    if (action.kind == InitKind::String) {
        std::uint64_t length = ast.literals[ast[action.source].literal].elements;
        auto limit = target.bound-length > 8 ? length : target.bound;
        for (std::uint64_t j = 0; j < limit; ++j) {
            Value at = j ? emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(j*sem.object_size(target.child))}) : base;
            at.type = target.child; at.address = true; store(string_element(action.source, target.child, j), at);
        }
        if (limit < target.bound) {
            Value at = emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(limit*sem.object_size(target.child))});
            at.address = true; at.type = target.child; repeat_initializer(0, at, target.bound-limit, target.child);
        }
        return;
    }
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        if (item.count > 8) {
            auto offset = item.index*sem.object_size(item.type);
            Value at = offset ? emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(offset)}) : base;
            at.type = item.type; at.address = true; repeat_initializer(child, at); continue;
        }
        for (std::uint64_t j = 0; j < item.count; ++j) {
            auto offset = item.field ? sem.entities[item.field].member_offset : (item.index+j)*sem.object_size(item.type);
            Instruction index(Opcode::Index, IRType::I8); index.projection = item.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
            Value at = !item.field && !offset ? base : emit(index, {base.operand, Operand::integer(offset)});
            at.type = item.type; at.address = true; at.init_offset = location.init_offset+offset;
            if (sem.field_fact(item.field).bit_field) { at.bit_field = item.field; at.initializing = true; }
            if (target.kind == TypeKind::Array && call_aggregate_helper(child, at)) continue;
            initialize_plan(child, at);
        }
    }
}
void Procedural::aggregate_plan(std::uint32_t plan, Value root, bool indirect, std::vector<InitProjection>& path)
{
    auto action = sem.initializers[plan];
    auto target = sem.types[action.type];
    if (action.kind == InitKind::Value || action.kind == InitKind::Constructor) {
        aggregate_initialize(action.kind == InitKind::Value ? 0 : action.source, action.type, root, indirect, path); return;
    }
    if (action.kind == InitKind::Scalar) {
        // Nested constructor aggregate bindings establish the reference member's
        // storage before materializing the bound address. No reference load is made.
        if (indirect && path.size() > 1 && reference(action.type)) {
            Value at = initialization_address(root, indirect, path); at.type = action.type;
            store(initialization_value(action.source, action.type), at); return;
        }
        Value value = path.empty() || path.back().field ? initialization_value(action.source, action.type) :
            action.source ? incoming(action.source) : Value(Operand::integer(0), type(action.type), action.type);
        Value at = initialization_address(root, indirect, path); at.type = action.type; store(value, at); return;
    }
    if (action.kind == InitKind::String) {
        std::uint64_t length = ast.literals[ast[action.source].literal].elements;
        auto limit = target.bound-length > 8 ? length : target.bound;
        for (std::uint64_t j = 0; j < limit; ++j) {
            InitProjection step(j, false); step.element = target.child; path.push_back(step);
            Value at = initialization_address(root, indirect, path); at.type = target.child;
            store(string_element(action.source, target.child, j), at); path.pop_back();
        }
        if (limit < target.bound) {
            InitProjection step(limit, false); step.element = target.child; path.push_back(step);
            Value at = initialization_address(root, indirect, path); at.type = target.child;
            repeat_initializer(0, at, target.bound-limit, target.child); path.pop_back();
        }
        return;
    }
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        if (item.count > 8) {
            path.push_back(InitProjection(item.index*sem.object_size(item.type), false));
            Value at = initialization_address(root, indirect, path); at.type = item.type;
            repeat_initializer(child, at); path.pop_back(); continue;
        }
        for (std::uint64_t j = 0; j < item.count; ++j) {
            InitProjection step(item.field ? sem.entities[item.field].member_offset : (item.index+j)*sem.object_size(item.type), item.field != 0, item.field);
            if (!item.field) { step.offset = item.index+j; step.element = item.type; }
            path.push_back(step); aggregate_plan(child, root, indirect, path); path.pop_back();
        }
    }
}
void Procedural::repeat_initializer(std::uint32_t plan, Value location, std::uint64_t count, TypeId type)
{
    auto action = sem.initializers[plan];
    if (!plan) { action.type = type; action.count = count; action.kind = InitKind::Value; }
    Value base = address(location);
    if (action.kind == InitKind::Value && sem.zero_value(action.type)) {
        Instruction zero(Opcode::ZeroInit); zero.bytes = action.count*sem.object_size(action.type);
        zero.alignment = sem.object_alignment(action.type); emit(zero, {base.operand}); return;
    }
    SlotId counter = builder->add_slot(0, IRType::I64);
    emit(Opcode::Store, IRType::I64, {Operand::integer(0), Operand::slot(counter)});
    BlockId test = block(), body = block(), end = block(); jump(test); start(test);
    Value current = emit(Opcode::Load, IRType::I64, {Operand::slot(counter)});
    Value condition = emit(Opcode::Compare, IRType::I64, {current.operand, Operand::integer(action.count)}, Operation::Ult);
    emit(Opcode::Branch, IRType(), {condition.operand, Operand::label(body), Operand::label(end)});
    start(body);
    Value offset = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(sem.object_size(action.type))}, Operation::Mul);
    Value at = emit(Opcode::Index, IRType::I8, {base.operand, offset.operand}); at.type = action.type; at.address = true;
    if (plan) initialize_plan(plan, at);
    else store(Value(Operand::integer(0), this->type(action.type), action.type), at);
    Value next = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(1)}, Operation::Add);
    emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(counter)}); jump(test); start(end);
}
} }
