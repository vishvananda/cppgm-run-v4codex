#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::initialize_array_copy(std::uint32_t plan, Value location, Value source)
{
    auto action = sem.initializers[plan];
    auto c = sem.conversion_fact(action.conversion);
    auto leaf = action.type, from = source.type;
    std::uint64_t count = 1;
    while (sem.types[leaf].kind == TypeKind::Array) {
        count *= sem.types[leaf].bound; leaf = sem.types[leaf].child; from = sem.types[from].child;
    }
    if (!count) return;
    auto target = address(location), origin = address(source);
    auto initial = live;
    semantic::Index retired;
    bool cleanup = sem.destructor_needed(sem.type_destructor(leaf));
    bool throwing = !sem.initializer_nonthrowing(plan);
    auto one = [&](Operand index, bool more) {
        auto dst = array_element(target,false,{},index,sem.object_size(leaf)); dst.type = leaf; dst.address = true;
        auto src = array_element(origin,false,{},index,sem.object_size(from)); src.type = from; src.address = true;
        auto before = live;
        typed_conversion(src,c,dst);
        complete_subobject(leaf,address(dst),before,cleanup && more && throwing,true,retired);
    };
    // Bound emitted work independently of array extent, including all nested
    // dimensions. Larger arrays use one loop and one completed-prefix counter.
    if (count <= 8) {
        for (std::uint64_t i = 0; i < count; ++i) one(Operand::integer(i),i+1 < count);
    } else {
        auto cursor = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(cursor)});
        if (cleanup && throwing) activate_subobject(leaf,target,cursor,retired);
        auto prefix = live;
        auto test = block(), body = block(), end = block(); jump(test); start(test);
        auto at = emit(Opcode::Load,IRType::I64,{Operand::slot(cursor)});
        auto more = emit(Opcode::Compare,IRType::I64,{at.operand,Operand::integer(count)},Operation::Ult);
        emit(Opcode::Branch,IRType(),{more.operand,Operand::label(body),Operand::label(end)});
        start(body);
        auto dst = array_element(target,false,{},at.operand,sem.object_size(leaf)); dst.type = leaf; dst.address = true;
        auto src = array_element(origin,false,{},at.operand,sem.object_size(from)); src.type = from; src.address = true;
        typed_conversion(src,c,dst);
        auto next = emit(Opcode::Binary,IRType::I64,{at.operand,Operand::integer(1)},Operation::Add);
        emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(cursor)});
        clean_inline(live,prefix); close_expression_region(); jump(test); start(end);
    }
    if (!retired.empty()) {
        close_expression_region(); semantic::Index cache;
        live = retire_construction(live,initial,retired,cache);
    }
}
} }
