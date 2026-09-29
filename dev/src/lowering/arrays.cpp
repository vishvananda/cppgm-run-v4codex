#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
namespace { const std::uint64_t array_unroll_limit = 8; }
Value Procedural::array_element(Value root, bool indirect, const std::vector<InitProjection>& path, Operand index, std::uint64_t stride)
{
    Value at = initialization_address(root, indirect, path);
    if (stride != 1) index = emit(Opcode::Binary, IRType::I64, {index, Operand::integer(stride)}, Operation::Mul).operand;
    Instruction i(Opcode::Index, IRType::I8); i.projection = ir_model::IPK_ARRAY_ELEMENT;
    return emit(i, {at.operand, index});
}
void Procedural::array_construct(EntityId ctor, TypeId t, Value root, bool indirect, const std::vector<InitProjection>& path)
{
    if (!sem.constructor_needed(ctor)) return;
    std::uint64_t count = 1;
    while (sem.types[t].kind == TypeKind::Array) { count *= sem.types[t].bound; t = sem.types[t].child; }
    auto stride = sem.object_size(t);
    EntityId dtor = sem.type_destructor(t);
    bool may_throw = !sem.default_construction_nonthrowing(ctor);
    bool temporary_defaults = false;
    for (unsigned i = 0; i < sem.types[sem.entities[ctor].type].count; ++i)
        temporary_defaults |= cleanup_expression(sem.default_argument_value(ctor,i));
    if (temporary_defaults || exception_context) {
        // Default-argument temporaries and the completed element use the same
        // immutable prefix owner. Mixing raw array regions with those suffixes
        // would share a resume block across different protected-region stacks.
        // A source handler also needs this owner: raw prefix cleanup followed
        // by resume cannot dispatch to a catch in the same function.
        auto initial = live;
        semantic::Index retired;
        if (count <= array_unroll_limit) {
            for (std::uint64_t j = 0; j < count; ++j) {
                auto before = live;
                Value at = array_element(root,indirect,path,Operand::integer(j),stride);
                construct(ctor,0,at);
                complete_subobject(t,at,before,j+1 < count && may_throw,temporary_defaults,retired);
            }
        } else {
            SlotId index = builder->add_slot(0,IRType::I64);
            emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(index)});
            if (may_throw) activate_subobject(t,initialization_address(root,indirect,path),index,retired);
            auto prefix = live;
            BlockId cond = block(), body = block(), end = block(); jump(cond); start(cond);
            Value current = emit(Opcode::Load,IRType::I64,{Operand::slot(index)});
            Value test = emit(Opcode::Compare,IRType::I64,{current.operand,Operand::integer(count)},Operation::Ult);
            emit(Opcode::Branch,IRType(),{test.operand,Operand::label(body),Operand::label(end)}); start(body);
            construct(ctor,0,array_element(root,indirect,path,current.operand,stride));
            Value next = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(1)},Operation::Add);
            emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(index)});
            clean_inline(live,prefix); close_expression_region(); jump(cond); start(end);
        }
        if (!retired.empty()) {
            close_expression_region(); semantic::Index cache;
            live = retire_construction(live,initial,retired,cache);
        }
        return;
    }
    bool partial = may_throw && sem.destructor_needed(dtor);
    bool enclosing = may_throw && live;
    auto saved_live = live;
    auto open_enclosing = [&]() {
        if (!enclosing) return;
        if (!resume_terminal) resume_terminal = block();
        emit(Opcode::EhTry, IRType(), {Operand::label(cleanup_suffix(live, resume_terminal))});
    };
    auto close_enclosing = [&]() {
        if (!enclosing) return;
        emit(Opcode::EhEnd, IRType(), {});
        BlockId next = block(); jump(next); flush_cleanups(); start(next);
    };
    if (count <= array_unroll_limit) {
        open_enclosing(); live = 0;
        for (std::uint64_t j = 0; j < count; ++j) {
            BlockId cleanup, next;
            if (j && partial) { cleanup = block(); next = block(); emit(Opcode::EhTry, IRType(), {Operand::label(cleanup)}); }
            construct(ctor, 0, array_element(root, indirect, path, Operand::integer(j), stride));
            clean_inline(live, 0);
            if (cleanup) {
                emit(Opcode::EhEnd, IRType(), {}); jump(next); start(cleanup);
                for (std::uint64_t k = j; k; --k) destroy(dtor, t, array_element(root, indirect, path, Operand::integer(k-1), stride));
                emit(Opcode::Resume, IRType(), {}); start(next);
            }
        }
    } else {
        SlotId index = builder->add_slot(0, IRType::I64);
        emit(Opcode::Store, IRType::I64, {Operand::integer(0), Operand::slot(index)});
        open_enclosing(); live = 0;
        BlockId cond = block(), body = block(), end = block(), cleanup = partial ? block() : BlockId();
        jump(cond); start(cond);
        Value current = emit(Opcode::Load, IRType::I64, {Operand::slot(index)});
        Value test = emit(Opcode::Compare, IRType::I64, {current.operand, Operand::integer(count)}, Operation::Ult);
        emit(Opcode::Branch, IRType(), {test.operand, Operand::label(body), Operand::label(end)});
        start(body);
        if (partial) emit(Opcode::EhTry, IRType(), {Operand::label(cleanup)});
        construct(ctor, 0, array_element(root, indirect, path, current.operand, stride));
        clean_inline(live, 0);
        if (partial) emit(Opcode::EhEnd, IRType(), {});
        Value next = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(1)}, Operation::Add);
        emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(index)}); jump(cond);
        if (partial) {
            BlockId cleanup_body = block(), resume = block(); start(cleanup);
            current = emit(Opcode::Load, IRType::I64, {Operand::slot(index)});
            test = emit(Opcode::Compare, IRType::I64, {current.operand, Operand::integer(0)}, Operation::Ne);
            emit(Opcode::Branch, IRType(), {test.operand, Operand::label(cleanup_body), Operand::label(resume)});
            start(cleanup_body);
            next = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(1)}, Operation::Sub);
            emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(index)});
            destroy(dtor, t, array_element(root, indirect, path, next.operand, stride)); jump(cleanup);
            start(resume); emit(Opcode::Resume, IRType(), {});
        }
        start(end);
    }
    live = saved_live; close_enclosing();
}
void Procedural::array_destroy(EntityId dtor, TypeId t, Value root, bool indirect, const std::vector<InitProjection>& path, bool subobject)
{
    if (!dtor) return;
    auto target = sem.types[t];
    TypeId leaf = t; std::uint64_t elements = 1;
    while (sem.types[leaf].kind == TypeKind::Array) { elements *= sem.types[leaf].bound; leaf = sem.types[leaf].child; }
    if (!indirect && path.empty() && root.operand.kind == Operand::Temporary &&
        sem.function_nonthrowing(dtor)) {
        if (auto first = list_backing_addresses.get(root.operand.ref)) {
            // Reuse the addresses of this exact materialization. A different
            // occurrence, function, or branch cannot share its ValueId key.
            for (std::uint64_t j = elements; j; --j)
                destroy(dtor,leaf,Value(Operand::value(list_element_addresses[first+j-2]),IRType::Ptr));
            return;
        }
    }
    if (!emitting_cleanup && elements > 1 && !sem.function_nonthrowing(dtor)) {
        // Destroying one element does not retire the rest of the array. Keep
        // an explicit remaining prefix in every call's unwind snapshot, just
        // as construction retains its completed prefix. The counter shrinks
        // before the call so a throwing destructor is never called twice.
        auto base = initialization_address(root,indirect,path); base.address = false;
        auto remaining = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{Operand::integer(elements),Operand::slot(remaining)});
        auto initial = live;
        semantic::Index retired;
        activate_subobject(leaf,base,remaining,retired);
        auto one = [&](Operand index) {
            emit(Opcode::Store,IRType::I64,{index,Operand::slot(remaining)});
            destroy(dtor,leaf,array_element(base,false,{},index,sem.object_size(leaf)));
            close_expression_region();
        };
        if (elements <= array_unroll_limit) {
            for (std::uint64_t j = elements; j; --j) one(Operand::integer(j-1));
        } else {
            auto test = block(), body = block(), end = block(); jump(test); start(test);
            auto count = emit(Opcode::Load,IRType::I64,{Operand::slot(remaining)});
            auto more = emit(Opcode::Compare,IRType::I64,{count.operand,Operand::integer(0)},Operation::Ne);
            emit(Opcode::Branch,IRType(),{more.operand,Operand::label(body),Operand::label(end)});
            start(body);
            one(emit(Opcode::Binary,IRType::I64,{count.operand,Operand::integer(1)},Operation::Sub).operand);
            jump(test); start(end);
        }
        live = initial; return;
    }
    if (elements <= array_unroll_limit) {
        for (std::uint64_t j = target.bound; j; --j) {
            BlockId cleanup, next;
            if (subobject && !emitting_cleanup && j > 1) {
                cleanup = block(); next = block(); emit(Opcode::EhCleanup, IRType(), {Operand::label(cleanup)});
            }
            Value at = array_element(root, indirect, path, Operand::integer(j-1), sem.object_size(target.child));
            destroy(dtor, target.child, at);
            if (cleanup) {
                emit(Opcode::EhEnd, IRType(), {}); jump(next); start(cleanup);
                emitting_cleanup = true;
                for (std::uint64_t k = j-1; k; --k)
                    destroy(dtor, target.child, array_element(root, indirect, path, Operand::integer(k-1), sem.object_size(target.child)));
                emit(Opcode::EhEnd, IRType(), {}); emit(Opcode::Resume, IRType(), {});
                emitting_cleanup = false; start(next);
            }
        }
        return;
    }
    target.bound = elements; target.child = leaf;
    Value base = initialization_address(root, indirect, path); base.address = false;
    SlotId index = builder->add_slot(0, IRType::I64);
    emit(Opcode::Store, IRType::I64, {Operand::integer(target.bound), Operand::slot(index)});
    BlockId cond = block(), body = block(), end = block();
    jump(cond); start(cond);
    Value current = emit(Opcode::Load, IRType::I64, {Operand::slot(index)});
    Value test = emit(Opcode::Compare, IRType::I64, {current.operand, Operand::integer(0)}, Operation::Ne);
    emit(Opcode::Branch, IRType(), {test.operand, Operand::label(body), Operand::label(end)});
    start(body);
    Value next = emit(Opcode::Binary, IRType::I64, {current.operand, Operand::integer(1)}, Operation::Sub);
    emit(Opcode::Store, IRType::I64, {next.operand, Operand::slot(index)});
    destroy(dtor, target.child, array_element(base, false, {}, next.operand, sem.object_size(target.child)));
    jump(cond); start(end);
}
} }
