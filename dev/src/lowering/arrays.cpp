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
    bool may_throw = !sem.function_nonthrowing(ctor), partial = may_throw && sem.destructor_needed(dtor);
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
    if (!sem.destructor_needed(dtor)) return;
    auto target = sem.types[t];
    TypeId leaf = t; std::uint64_t elements = 1;
    while (sem.types[leaf].kind == TypeKind::Array) { elements *= sem.types[leaf].bound; leaf = sem.types[leaf].child; }
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
