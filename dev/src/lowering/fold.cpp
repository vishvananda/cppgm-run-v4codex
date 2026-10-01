#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::fold_operation(const semantic::RangeOperation& op, Value a, Value b, Value* callable)
{
    if (op.function) return range_operation(op,{a,b});
    if (op.op == OP_COMMA) return b;
    if (op.op == OP_DOTSTAR || op.op == OP_ARROWSTAR) {
        auto object = op.op == OP_ARROWSTAR ? load(a) : address(a);
        object = base_projection(object,op.adjustment);
        auto member = load(b);
        if (op.result.form == semantic::ExpressionForm::BoundMember) {
            if (!callable) throw std::logic_error("bound fold requires a callable consumer");
            *callable = coerce(member,IRType::I64,true,true);
            *callable = emit(Opcode::Copy,IRType::Ptr,{callable->operand});
            if (!member.member_zero_adjustment) {
                auto high = emit(Opcode::Binary,IRType::I128,{member.operand,Operand::integer(64)},Operation::Shr);
                auto adjustment = coerce(high,IRType::I64,true,true);
                object = emit(Opcode::Index,IRType::I8,{object.operand,adjustment.operand});
            }
            return object;
        }
        auto offset = emit(Opcode::Binary,IRType::I64,{member.operand,Operand::integer(1)},Operation::Sub);
        Instruction index(Opcode::Index,IRType::I8); index.projection = ir_model::IPK_FIELD;
        auto value = emit(index,{object.operand,offset.operand});
        value.type = op.result.type; value.address = true; return value;
    }
    auto first = sem.conversion_fact(op.result.conversions);
    auto second = sem.conversion_fact(op.result.conversions+1);
    if (syntax::expression_precedence(op.op) == 2) {
        auto dest = typed_conversion(a,first); dest.type = op.result.type; dest.address = true;
        auto rhs = typed_conversion(b,second);
        if (op.op != OP_ASS) {
            auto binary = sem.compound_operation(op.op);
            auto common = sem.conversion_fact(op.result.conversions+2).target;
            if (sem.types[dest.type].cv & 4) return atomic_update(dest,rhs,binary,common,false);
            auto lhs = convert(load(dest),common);
            rhs = operation(binary,lhs,rhs,common);
        }
        rhs = store(convert(rhs,dest.type),dest); dest.cached = true; dest.stored = rhs.operand; return dest;
    }
    return operation(op.op,typed_conversion(a,first),typed_conversion(b,second),op.result.type);
}
Value Procedural::fold(NodeId n, bool location, Value* callable)
{
    struct Frame {
        unsigned id, phase = 0; Value left;
        SlotId result, selector; BlockId short_path, end;
        std::uint32_t common = 0;
        explicit Frame(unsigned i) : id(i) {}
    };
    std::vector<Frame> work; work.emplace_back(sem.fold_root(n)); Value value;
    while (!work.empty()) {
        auto& frame = work.back(); auto step = sem.fold_step(frame.id); auto op = step.operation;
        if (step.source) { value = expression(step.source,location); work.pop_back(); continue; }
        bool logical = !op.function && (op.op == OP_LAND || op.op == OP_LOR);
        bool land = op.op == OP_LAND;
        if (!frame.phase) { frame.phase = 1; work.emplace_back(step.left); continue; }
        if (frame.phase == 1) {
            frame.left = value; frame.phase = 2;
            if (logical) {
                auto test = truth_operand(typed_conversion(value,sem.conversion_fact(op.result.conversions)));
                if (test.ir.floating()) test = emit(Opcode::Compare,test.ir,{test.operand,Operand::floating(0)},Operation::Ne);
                frame.result = builder->add_slot(0,IRType::I64);
                auto rhs = block(); frame.short_path = block(); frame.end = block();
                frame.selector = cleanup_selector(test,true); frame.common = live;
                emit(Opcode::Branch,IRType(),{test.operand,Operand::label(land ? rhs : frame.short_path),Operand::label(land ? frame.short_path : rhs)});
                start(rhs);
            } else if (!op.function && op.op == OP_COMMA) {
                // A discarded volatile glvalue is still an observable load.
                if (sem.types[value.type].cv & 2) load(value);
            }
            work.emplace_back(step.right); continue;
        }
        if (logical) {
            value = truth_operand(typed_conversion(value,sem.conversion_fact(op.result.conversions+1)));
            auto ir = value.ir.floating() || value.ir == IRType::Ptr ? value.ir : IRType(IRType::I64);
            value = emit(Opcode::Compare,ir,{value.operand,value.ir.floating() ? Operand::floating(0) : Operand::integer(0)},Operation::Ne);
            emit(Opcode::Store,IRType::I64,{value.operand,Operand::slot(frame.result)});
            auto rhs_live = live; jump(frame.end);
            start(frame.short_path); live = frame.common;
            emit(Opcode::Store,IRType::I64,{Operand::integer(!land),Operand::slot(frame.result)}); jump(frame.end);
            start(frame.end);
            merge_temporaries(frame.common,land ? rhs_live : frame.common,land ? frame.common : rhs_live,frame.selector);
            value = emit(Opcode::Load,IRType::I64,{Operand::slot(frame.result)}); value.type = op.result.type;
        } else value = fold_operation(op,frame.left,value,callable);
        work.pop_back();
    }
    return value;
}
} }
