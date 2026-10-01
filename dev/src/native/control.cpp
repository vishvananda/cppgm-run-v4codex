#include "native/selection.h"
#include "native/abi.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
X86Condition branch_condition(Operation op);
void Selector::edge_transfers(unsigned pred, unsigned target)
{
    EdgeMove key; key.pred = pred; key.target = target;
    auto first = std::lower_bound(edge_moves.begin(),edge_moves.end(),key,[](const EdgeMove& a,const EdgeMove& b) {
        return a.pred != b.pred ? a.pred < b.pred : a.target < b.target;
    });
    auto last = first;
    while (last != edge_moves.end() && last->pred == pred && last->target == target) ++last;
    // Only sources that are themselves changed phi destinations need staging.
    // Their definition/block identities prove overlap without comparing offsets.
    for (auto e = first; e != last; ++e) if (e->staging.kind != Operand::None)
        move(e->staging,value(e->source,p.values[e->destination-1].type),value_type(e->source,p.values[e->destination-1].type));
    for (auto e = first; e != last; ++e) {
        if (e->source.kind == lowir_model::Operand::Temporary && root(e->source.ref) == e->destination) continue;
        auto from = e->staging.kind == Operand::None ? value(e->source,p.values[e->destination-1].type) : e->staging;
        move(state(e->destination).location,from,p.values[e->destination-1].type);
    }
}
unsigned Selector::edge_target(unsigned target)
{
    EdgeMove key; key.pred = block_id; key.target = target;
    auto found = std::lower_bound(edge_moves.begin(),edge_moves.end(),key,[](const EdgeMove& a,const EdgeMove& b) {
        return a.pred != b.pred ? a.pred < b.pred : a.target < b.target;
    });
    if (found == edge_moves.end() || found->pred != block_id || found->target != target) return target;
    if (!found->label) {
        found->label = next_label++;
        edge_blocks.push_back({block_id,target,found->label});
    }
    return found->label;
}
void Selector::control(const lowir_model::Instruction& i)
{
    switch (i.opcode) {
    case Opcode::Jump:
        edge_transfers(block_id,arg(i,0).ref);
        emit(Op::Jump,Type(),{Operand::label(arg(i,0).ref)}); break;
    case Opcode::Branch: {
        auto cond = arg(i,0);
        X86Condition cc = XC_NE;
        if (cond.kind == lowir_model::Operand::Temporary && state(cond.ref).compare_branch &&
            p.instructions[state(cond.ref).definition-1].type.floating()) {
            const auto& comparison = p.instructions[state(cond.ref).definition-1];
            auto op = comparison.operation;
            cc = op == Operation::Eq || op == Operation::Not ? XC_E : op == Operation::Ne ? XC_NE :
                (op == Operation::Lt || op == Operation::Ult) ? XC_B :
                (op == Operation::Le || op == Operation::Ule) ? XC_BE :
                (op == Operation::Gt || op == Operation::Ugt) ? XC_A : XC_AE;
            emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,op == Operation::Ne ? 1 : 2).ref))}).condition = XC_P;
        } else if (cond.kind == lowir_model::Operand::Temporary && state(cond.ref).compare_branch)
            cc = p.instructions[state(cond.ref).definition-1].operation == Operation::Not ? XC_E :
                branch_condition(p.instructions[state(cond.ref).definition-1].operation);
        else {
            Type t = value_type(cond,Type::I64);
            if (t.floating()) {
                f.scratch_bytes = 48;
                emit(Op::Fcompare,t,{value(cond,t),Operand::floating(lowir_model::Operand::integer(0),t)});
                emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,1).ref))}).condition = XC_P;
            } else if (t == Type::I128) {
                auto from = value(cond,t);
                move(Operand::r(XR_R10),fragment(from,0),Type::I64);
                emit(Op::Or,Type::I64,{Operand::r(XR_R10),fragment(from,8)});
                emit(Op::Compare,Type::I64,{Operand::r(XR_R10),Operand::imm(0)});
            } else {
                Operand test = in_register(value(cond,t),t,XR_R10);
                emit(Op::Compare,t,{test,Operand::imm(0)});
            }
        }
        if (cond.kind == lowir_model::Operand::Temporary && state(cond.ref).compare_branch &&
            p.instructions[state(cond.ref).definition-1].type == Type::I128)
            cc = cc == XC_L ? XC_B : cc == XC_LE ? XC_BE : cc == XC_G ? XC_A : cc == XC_GE ? XC_AE : cc;
        emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,1).ref))}).condition = cc;
        emit(Op::Jump,Type(),{Operand::label(edge_target(arg(i,2).ref))});
        break;
    }
    case Opcode::Switch: {
        Type t = value_type(arg(i,0),Type::I64);
        if (t == Type::I128) {
            auto selector = value(arg(i,0),t);
            for (unsigned k = 2; k != i.operands.count; k += 2) {
                auto candidate = value(arg(i,k),t);
                unsigned next = next_label++;
                move(Operand::r(XR_R10),fragment(selector,8),Type::I64);
                emit(Op::Compare,Type::I64,{Operand::r(XR_R10),fragment(candidate,8)});
                emit(Op::Jcc,Type(),{Operand::label(next)}).condition = XC_NE;
                move(Operand::r(XR_R10),fragment(selector,0),Type::I64);
                emit(Op::Compare,Type::I64,{Operand::r(XR_R10),fragment(candidate,0)});
                emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,k+1).ref))}).condition = XC_E;
                begin_block(next,0);
            }
            emit(Op::Jump,Type(),{Operand::label(edge_target(arg(i,1).ref))}); break;
        }
        auto selector = in_register(value(arg(i,0),t),t,XR_R10);
        for (unsigned k = 2; k != i.operands.count; k += 2) {
            auto candidate = value(arg(i,k),t);
            if (candidate.kind == Operand::Memory && value_type(arg(i,k),t) != t)
                candidate = in_register(candidate,value_type(arg(i,k),t),XR_R11);
            emit(Op::Compare,t,{selector,candidate});
            emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,k+1).ref))}).condition = XC_E;
        }
        emit(Op::Jump,Type(),{Operand::label(edge_target(arg(i,1).ref))}); break;
    }
    case Opcode::Return:
        if (i.type == Type()) emit(Op::Return,i.type,{});
        else if (aggregate(i.type)) {
            Operand from = value(arg(i,0),i.type);
            if (i.type.complex() && i.type.component() == Type::F80) {
                f.scratch_bytes = 48;
                emit(Op::Fpush,Type::F80,{fragment(from,16)});
                emit(Op::Fpush,Type::F80,{fragment(from,0)});
            } else if (indirect_return(i.type)) {
                move(Operand::r(XR_R10),indirect_result,Type::Ptr);
                object_move(Operand::mem(XR_R10),from,i.type);
                move(Operand::r(XR_RAX),indirect_result,Type::Ptr);
            } else {
                move(Operand::r(i.type.complex() ? xmm(0) : XR_RAX),fragment(from,0),abi_chunk_type(i.type,0));
                if (i.type.bytes() > 8) move(Operand::r(i.type.complex() ? xmm(1) : XR_RDX),fragment(from,8),abi_chunk_type(i.type,1));
            }
            emit(Op::Return,Type(),{});
        }
        else if (i.type.floating()) {
            auto result = convert_value(value(arg(i,0),i.type),value_type(arg(i,0),i.type),i.type);
            f.scratch_bytes = 48;
            if (i.type == Type::F80) emit(Op::Freturn,i.type,{result});
            else { move(Operand::r(xmm(0)),result,i.type); emit(Op::Return,Type(),{}); }
        } else {
            auto result = in_register(value(arg(i,0),i.type),consumed_type(arg(i,0),i.type),XR_RAX);
            emit(Op::Return,i.type,{result});
        }
        break;
    case Opcode::Unreachable: emit(Op::Trap,Type(),{}); break;
    default: throw ParseError("invalid control instruction");
    }
}
} // namespace native
