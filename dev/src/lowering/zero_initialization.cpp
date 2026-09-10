#include "lowering/procedural.h"
#include <algorithm>
#include <stdexcept>
namespace cppgm { namespace lowering {
void Procedural::zero_object(TypeId t, Value object)
{
    auto plan = sem.zero_initialization(t);
    if (!plan) throw std::logic_error("missing zero-initialization plan");
    zero_plan(plan,object);
}
void Procedural::zero_padding(Value object, std::uint64_t offset, std::uint64_t bytes, std::uint64_t alignment)
{
    if (!bytes) return;
    // Expand at most eight padding stores. Larger gaps are pure object
    // representation, so one bulk operation preserves bounded IR and work.
    unsigned stores = 0;
    for (auto at = offset, left = bytes; left && stores <= 8; ++stores) {
        unsigned width = 8;
        while (width > left || width > alignment || at % width) width /= 2;
        at += width; left -= width;
    }
    if (stores > 8) {
        auto at = offset ? emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(offset)}) : object;
        while (offset % alignment) alignment /= 2;
        Instruction zero(Opcode::ZeroInit); zero.bytes = bytes; zero.alignment = alignment;
        emit(zero,{at.operand}); return;
    }
    while (bytes) {
        unsigned width = 8;
        while (width > bytes || width > alignment || offset % width) width /= 2;
        IRType t = width == 8 ? IRType::I64 : width == 4 ? IRType::I32 : width == 2 ? IRType::I16 : IRType::I8;
        auto at = offset ? emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(offset)}) : object;
        emit(Opcode::Store,t,{Operand::integer(0),at.operand}); offset += width; bytes -= width;
    }
}
void Procedural::zero_plan(std::uint32_t id, Value object)
{
    using semantic::ZeroInitialization;
    auto plan = sem.zero_initializations[id];
    switch (plan.kind) {
    case ZeroInitialization::Reference: return;
    case ZeroInitialization::Representation: {
        Instruction zero(Opcode::ZeroInit); zero.bytes = plan.bytes; zero.alignment = plan.alignment;
        emit(zero,{object.operand}); return;
    }
    case ZeroInitialization::Scalar: {
        auto t = type(plan.type);
        emit(Opcode::Store,t,{t.floating() ? Operand::floating(0) : Operand::integer(0),object.operand}); return;
    }
    case ZeroInitialization::MemberPointer:
        if (plan.bytes == 8) emit(Opcode::Store,IRType::I64,{Operand::integer(~std::uint64_t(0)),object.operand});
        else zero_padding(object,0,plan.bytes,plan.alignment);
        return;
    case ZeroInitialization::Composite: {
        std::uint64_t end = 0;
        for (unsigned j = 0; j < plan.count; ++j) {
            auto part = sem.zero_parts[plan.first+j];
            if (part.offset > end) zero_padding(object,end,part.offset-end,plan.alignment);
            auto at = part.offset ? emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(part.offset)}) : object;
            zero_plan(part.plan,at);
            end = std::max(end,part.offset+sem.zero_initializations[part.plan].bytes);
        }
        if (plan.bytes > end) zero_padding(object,end,plan.bytes-end,plan.alignment);
        return;
    }
    case ZeroInitialization::Array: {
        auto stride = sem.zero_initializations[plan.child].bytes;
        if (plan.elements <= 8 / initialization_expansion) {
            auto saved_expansion = initialization_expansion;
            initialization_expansion *= plan.elements;
            for (std::uint64_t j = 0; j < plan.elements; ++j) {
                auto at = j ? emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(j*stride)}) : object;
                zero_plan(plan.child,at);
            }
            initialization_expansion = saved_expansion; return;
        }
        SlotId slot = builder->add_slot(0,IRType::I64);
        emit(Opcode::Store,IRType::I64,{Operand::integer(0),Operand::slot(slot)});
        auto test = block(), body = block(), end = block(); jump(test); start(test);
        auto index = emit(Opcode::Load,IRType::I64,{Operand::slot(slot)});
        auto more = emit(Opcode::Compare,IRType::I64,{index.operand,Operand::integer(plan.elements)},Operation::Ult);
        emit(Opcode::Branch,IRType(),{more.operand,Operand::label(body),Operand::label(end)});
        start(body); auto offset = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(stride)},Operation::Mul);
        auto at = emit(Opcode::Index,IRType::I8,{object.operand,offset.operand}); zero_plan(plan.child,at);
        auto next = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(1)},Operation::Add);
        emit(Opcode::Store,IRType::I64,{next.operand,Operand::slot(slot)}); jump(test); start(end); return;
    }
    }
}
} }
