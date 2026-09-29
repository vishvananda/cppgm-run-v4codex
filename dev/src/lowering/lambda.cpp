#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::captured_address(unsigned id)
{
    auto capture = sem.closure_captures[id];
    auto base = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
    auto value = field(base,capture.field);
    return capture.object ? address(value) : load(value);
}
void Procedural::initialize_closure(NodeId n, Value destination)
{
    auto closure = sem.closure(sem.types[sem.expression_fact(n).type].entity);
    auto initial = live;
    semantic::Index retired; bool has_partial = false;
    auto activate = [&](EntityId field, Value at, SlotId count) {
        close_expression_region();
        TemporaryState state; state.object = field; state.destructor = sem.type_destructor(sem.entities[field].type);
        state.location = lowir_model::ValueId(at.operand.ref); state.constructed = count;
        state.tail = live; state.depth = lifetime_state(live).depth+1;
        temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
        retired.put(live,1); has_partial = true;
    };
    for (auto id = closure.first_capture; id; id = sem.closure_captures[id].next) {
        auto capture = sem.closure_captures[id];
        auto offset = sem.entities[capture.field].member_offset;
        auto target = destination;
        if (offset) {
            Instruction projection(Opcode::Index,IRType::I8); projection.projection = ir_model::IPK_FIELD;
            target = emit(projection,{destination.operand,Operand::integer(offset)});
        }
        Value source;
        if (capture.source) {
            source = captured_address(capture.source);
            source.type = capture.source_type; source.address = capture.object != 0;
        } else if (capture.object) source = binding(capture.object);
        else source = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
        if (capture.by_copy) {
            auto c = sem.conversion_fact(capture.conversion);
            if (sem.types[capture.source_type].kind == semantic::TypeKind::Array) {
                TypeId leaf = capture.source_type; std::uint64_t count = 1;
                while (sem.types[leaf].kind == semantic::TypeKind::Array) { count *= sem.types[leaf].bound; leaf = sem.types[leaf].child; }
                auto root = address(source);
                bool cleanup = sem.destructor_needed(sem.type_destructor(leaf));
                SlotId cursor;
                if (cleanup || count > 8) {
                    cursor = builder->add_slot(0,IRType::I64);
                    emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(cursor)});
                }
                if (cleanup) activate(capture.field,target,cursor);
                auto one = [&](Operand index) {
                    auto src = array_element(root,false,{},index,sem.object_size(leaf));
                    src.type = leaf; src.address = true;
                    auto dst = array_element(target,false,{},index,sem.object_size(leaf));
                    auto prior = live;
                    typed_conversion(src,c,dst);
                    if (cleanup) {
                        auto next = emit(Opcode::Binary,IRType::I64,{index,Operand::integer(1)},Operation::Add);
                        emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(cursor)});
                    }
                    // Array element default-argument temporaries end before the
                    // next element starts. The completed prefix is already live.
                    clean_inline(live,prior); close_expression_region();
                };
                if (count <= 8) {
                    for (std::uint64_t i = 0; i < count; ++i) one(Operand::integer(i));
                } else {
                    auto test = block(), body = block(), end = block(); jump(test); start(test);
                    auto index = emit(Opcode::Load,IRType::I64,{Operand::slot(cursor)});
                    auto more = emit(Opcode::Compare,IRType::I64,{index.operand,Operand::integer(count)},Operation::Ult);
                    emit(Opcode::Branch,IRType(),{more.operand,Operand::label(body),Operand::label(end)});
                    start(body); one(index.operand);
                    if (!cleanup) {
                        auto next = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(1)},Operation::Add);
                        emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(cursor)});
                    }
                    jump(test); start(end);
                }
            } else {
                typed_conversion(source,c,target);
                if (sem.destructor_needed(sem.type_destructor(sem.entities[capture.field].type)))
                    activate(capture.field,target,SlotId());
            }
        } else {
            if (capture.object) source = address(source);
            emit(Opcode::Store,IRType::Ptr,{source.operand,target.operand});
        }
    }
    if (has_partial) {
        close_expression_region();
        semantic::Index cache;
        live = retire_construction(live,initial,retired,cache);
    }
}
std::uint32_t Procedural::retire_construction(std::uint32_t state, std::uint32_t stop,
    const semantic::Index& retired, semantic::Index& cache)
{
    if (state == stop) return stop;
    if (auto known = cache.get(state)) return known-1;
    auto record = temporary_states[(state & 0x7fffffffu)-1];
    auto tail = retire_construction(record.tail,stop,retired,cache);
    auto result = tail;
    if (!retired.get(state)) {
        if (record.selector) {
            record.yes = retire_construction(record.yes,stop,retired,cache);
            record.no = retire_construction(record.no,stop,retired,cache);
        }
        record.tail = tail; record.depth = lifetime_state(tail).depth+1;
        temporary_states.push_back(record); result = 0x80000000u | temporary_states.size();
    }
    cache.put(state,result+1); return result;
}
void Procedural::closure_adapter(EntityId e)
{
    auto closure = sem.closure_adapter(e);
    function = FunctionId(p.symbols[symbol(e).index-1].entity);
    builder.reset(new lowir_model::FunctionBuilder(p,function));
    reset_lifetime(e); start(block());
    emit(Opcode::Return,IRType::Ptr,{Operand::symbol(symbol(closure.thunk))});
    builder.reset();
}
} }
