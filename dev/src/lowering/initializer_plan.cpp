#include "lowering/procedural.h"
#include <cstring>
#include <algorithm>
#include <stdexcept>
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
void Procedural::global_plan(std::uint32_t plan)
{
    auto action = sem.initializers[plan];
    auto target = sem.types[action.type];
    if (action.kind == InitKind::Constructor) { global_construction(action.source, action.type); return; }
    if (semantic::vector_kind(target.kind) && sem.types[target.child].fundamental == FT_BOOL && action.kind == InitKind::Group) {
        std::vector<unsigned char> bytes(sem.object_size(action.type),0);
        for (auto child = action.first; child; child = sem.initializers[child].next) {
            auto item = sem.initializers[child]; auto value = sem.static_value(item.source,item.type);
            if (value.kind != semantic::StaticValue::Integer) throw std::logic_error("missing boolean vector initializer fact");
            if (value.bits) for (std::uint64_t j = 0; j < item.count; ++j) bytes[(item.index+j)/8] |= 1u<<((item.index+j)%8);
        }
        for (auto byte : bytes) { DataItem item; item.kind = DataItem::Scalar; item.type = IRType::U8; item.value = Operand::integer(byte); p.data.push_back(item); }
        return;
    }
    if ((action.kind == InitKind::Scalar || action.kind == InitKind::Value) &&
        target.kind == TypeKind::MemberPointer && sem.types[target.child].kind == TypeKind::Function) {
        member_pointer_data(sem.static_value(action.source,action.type)); return;
    }
    if ((action.kind == InitKind::Scalar || action.kind == InitKind::Value) && sem.complex_type(action.type)) {
        auto value = sem.static_value(action.source,action.type);
        if (value.kind != semantic::StaticValue::Complex) throw std::logic_error("missing static complex initializer fact");
        complex_data(action.type,semantic::Constant(action.type,value.bits)); return;
    }
    if (action.kind == InitKind::Scalar) {
        auto item = constant_data(action.source, action.type);
        if (item.type == IRType::Ptr && item.kind == DataItem::Scalar && !item.value.data.integer) {
            item.kind = DataItem::Zero; item.zero_bytes = 8;
        }
        p.data.push_back(item); return;
    }
    if (action.kind == InitKind::Value) {
        auto item = constant_data(0, action.type);
        if (item.type == IRType::Ptr) { item.kind = DataItem::Zero; item.zero_bytes = 8; }
        p.data.push_back(item); return;
    }
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
        if (sem.field_fact(item.field).no_unique_address && sem.empty_class(item.type)) continue;
        auto at = item.field ? sem.entities[item.field].member_offset : item.index * sem.object_size(item.type);
        if (at > bytes) { DataItem zero; zero.zero_bytes = at-bytes; p.data.push_back(zero); bytes = at; }
        if (sem.field_fact(item.field).bit_field) { global_bit_field(child, bytes); continue; }
        if (item.count > 8 && !item.source && sem.zero_value(item.type) && sem.constant_plan(child)) {
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
    if (target.cv & 4) atomic_padding(action.type,address(location));
    if (action.kind == InitKind::ArrayCopy) {
        initialize_array_copy(plan,location,expression(action.source)); return;
    }
    if (action.kind == InitKind::Converted) {
        auto c = sem.conversion_fact(action.conversion);
        if (sem.class_value(action.type)) construct_value(action.source,c,address(location));
        else if (target.kind == TypeKind::Array) list_conversion(c,address(location));
        else store(converted(action.source,c),location);
        return;
    }
    if (action.kind == InitKind::Scalar) {
        Value value = action.source ? incoming(action.source) : initialization_value(0,action.type);
        store(value, location); return;
    }
    if (action.kind == InitKind::Constructor) {
        auto value = sem.class_initialization(action.source,action.type);
        if (value.source) construct_value(value.source,sem.conversion_fact(value.conversion),address(location));
        else construct(sem.facts[action.source].entity, action.source, address(location));
        return;
    }
    if (action.kind == InitKind::Value) {
        if (auto ctor = sem.value_constructor(action.type)) {
            if (action.zero) zero_object(action.type,address(location));
            construct(ctor, 0, address(location));
        }
        else store(initialization_value(0, action.type), location);
        return;
    }
    if (semantic::vector_kind(target.kind)) {
        zero_object(action.type,address(location));
        for (auto child = action.first; child; child = sem.initializers[child].next) {
            auto item = sem.initializers[child];
            if (!item.source) continue; // The omitted suffix is already zero.
            for (std::uint64_t j = 0; j < item.count; ++j) {
                auto value = item.kind == InitKind::Converted ? converted(item.source,sem.conversion_fact(item.conversion)) : incoming(item.source);
                vector_write(location,item.index+j,value);
            }
        }
        return;
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
    auto initial = live;
    semantic::Index retired;
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        bool cleanup = sem.destructor_needed(sem.type_destructor(item.type));
        bool later = cleanup && !sem.initializer_suffix_nonthrowing(item.next);
        if (item.count > 8 / initialization_expansion) {
            auto offset = item.index*sem.object_size(item.type);
            Value at = offset ? emit(Opcode::Index, IRType::I8, {base.operand, Operand::integer(offset)}) : base;
            at.type = item.type; at.address = true;
            auto count = repeat_initializer(child, at);
            if (later) activate_subobject(item.type,address(at),count,retired);
            continue;
        }
        auto saved_expansion = initialization_expansion;
        initialization_expansion *= item.count;
        for (std::uint64_t j = 0; j < item.count; ++j) {
            auto offset = item.field ? sem.entities[item.field].member_offset : (item.index+j)*sem.object_size(item.type);
            Instruction index(Opcode::Index, IRType::I8); index.projection = item.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
            Value at = !item.field && !offset ? base : emit(index, {base.operand, Operand::integer(offset)});
            at.type = item.type; at.address = true; at.overlapping = sem.field_fact(item.field).no_unique_address; at.init_offset = location.init_offset+offset;
            if (sem.field_fact(item.field).bit_field) { at.bit_field = item.field; at.initializing = true; }
            auto before = live;
            if (!(target.kind == TypeKind::Array && call_aggregate_helper(child, at))) initialize_plan(child, at);
            bool retain = later || (cleanup && j+1 < item.count && !sem.initializer_nonthrowing(child));
            bool defaults = target.kind == TypeKind::Array && item.kind == InitKind::Value && sem.value_constructor(item.type);
            complete_subobject(item.type,address(at),before,retain,defaults,retired);
        }
        initialization_expansion = saved_expansion;
    }
    if (!retired.empty()) {
        close_expression_region(); semantic::Index cache;
        live = retire_construction(live,initial,retired,cache);
    }
}
void Procedural::aggregate_plan(std::uint32_t plan, Value root, bool indirect, std::vector<InitProjection>& path, Value* initialized)
{
    auto action = sem.initializers[plan];
    auto target = sem.types[action.type];
    if (target.cv & 4) atomic_padding(action.type,address(initialization_address(root,indirect,path)));
    if (action.kind == InitKind::Value || action.kind == InitKind::Converted || action.kind == InitKind::Constructor) {
        Value at = initialization_address(root, indirect, path); at.type = action.type;
        initialize_plan(plan, at); if (initialized) *initialized = at; return;
    }
    if (action.kind == InitKind::Scalar) {
        // Nested constructor aggregate bindings establish the reference member's
        // storage before materializing the bound address. No reference load is made.
        if (indirect && path.size() > 1 && reference(action.type)) {
            Value at = initialization_address(root, indirect, path); at.type = action.type;
            store(initialization_value(action.source, action.type), at); return;
        }
        Value value = path.empty() || path.back().field ? initialization_value(action.source, action.type) :
            action.source ? incoming(action.source) : initialization_value(0,action.type);
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
    auto initial = live;
    semantic::Index retired;
    for (auto child = action.first; child; child = sem.initializers[child].next) {
        auto item = sem.initializers[child];
        bool destruction = sem.destructor_needed(sem.type_destructor(item.type));
        bool later = destruction && !sem.initializer_suffix_nonthrowing(item.next);
        if (item.count > 8 / initialization_expansion) {
            path.push_back(InitProjection(item.index*sem.object_size(item.type), false));
            Value at = initialization_address(root, indirect, path); at.type = item.type;
            auto count = repeat_initializer(child, at);
            if (later) activate_subobject(item.type,address(at),count,retired);
            path.pop_back(); continue;
        }
        auto saved_expansion = initialization_expansion;
        initialization_expansion *= item.count;
        for (std::uint64_t j = 0; j < item.count; ++j) {
            InitProjection step(item.field ? sem.entities[item.field].member_offset : (item.index+j)*sem.object_size(item.type), item.field != 0, item.field);
            if (!item.field) { step.offset = item.index+j; step.element = item.type; }
            bool cleanup = later || (destruction && j+1 < item.count && !sem.initializer_nonthrowing(child));
            bool defaults = target.kind == TypeKind::Array && item.kind == InitKind::Value && sem.value_constructor(item.type);
            Value at;
            auto before = live;
            path.push_back(step); aggregate_plan(child, root, indirect, path,cleanup || defaults ? &at : nullptr); path.pop_back();
            if (cleanup || defaults) complete_subobject(item.type,address(at),before,cleanup,defaults,retired);
        }
        initialization_expansion = saved_expansion;
    }
    if (!retired.empty()) {
        close_expression_region(); semantic::Index cache;
        live = retire_construction(live,initial,retired,cache);
    }
    if (initialized) *initialized = initialization_address(root,indirect,path);
}
SlotId Procedural::repeat_initializer(std::uint32_t plan, Value location, std::uint64_t count, TypeId type)
{
    auto action = sem.initializers[plan];
    if (!plan) { action.type = type; action.count = count; action.kind = InitKind::Value; }
    Value base = address(location);
    if (!action.source && sem.zero_value(action.type) && (!plan || sem.constant_plan(plan))) {
        Instruction zero(Opcode::ZeroInit); zero.bytes = action.count*sem.object_size(action.type);
        zero.alignment = sem.object_alignment(action.type); emit(zero, {base.operand}); return SlotId();
    }
    SlotId counter = builder->add_slot(0, IRType::I64);
    emit(Opcode::Store, IRType::I64, {Operand::integer(0), Operand::slot(counter)});
    auto initial = live;
    semantic::Index retired;
    if (plan && !sem.initializer_nonthrowing(plan)) activate_subobject(action.type,base,counter,retired);
    auto prefix = live;
    BlockId test = block(), body = block(), end = block(); jump(test); start(test);
    Value current = emit(Opcode::Load, IRType::I64, {Operand::slot(counter)});
    Value condition = emit(Opcode::Compare, IRType::I64, {current.operand, Operand::integer(action.count)}, Operation::Ult);
    emit(Opcode::Branch, IRType(), {condition.operand, Operand::label(body), Operand::label(end)});
    start(body);
    Value offset = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(sem.object_size(action.type))}, Operation::Mul);
    Value at = emit(Opcode::Index, IRType::I8, {base.operand, offset.operand}); at.type = action.type; at.address = true;
    if (plan) initialize_plan(plan, at);
    else store(initialization_value(0,action.type), at);
    Value next = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(1)}, Operation::Add);
    emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(counter)});
    clean_inline(live,prefix); close_expression_region(); jump(test); start(end);
    if (!retired.empty()) {
        semantic::Index cache; live = retire_construction(live,initial,retired,cache);
    }
    return counter;
}
} }
