#include "native/selection.h"
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
    // Staging all incoming values implements parallel assignment, including
    // cycles and frame-address sources, before any destination is overwritten.
    for (auto e = first; e != last; ++e)
        move(e->staging,value(e->source,p.values[e->destination-1].type),value_type(e->source,p.values[e->destination-1].type));
    for (auto e = first; e != last; ++e)
        move(state(e->destination).location,e->staging,p.values[e->destination-1].type);
}
unsigned Selector::edge_target(unsigned target)
{
    EdgeMove key; key.pred = block_id; key.target = target;
    auto found = std::lower_bound(edge_moves.begin(),edge_moves.end(),key,[](const EdgeMove& a,const EdgeMove& b) {
        return a.pred != b.pred ? a.pred < b.pred : a.target < b.target;
    });
    if (found == edge_moves.end() || found->pred != block_id || found->target != target) return target;
    unsigned id = next_label++;
    edge_blocks.push_back({block_id,target,id});
    return id;
}
void Selector::control(const lowir_model::Instruction& i)
{
    switch (i.opcode) {
    case Opcode::Jump:
        emit(Op::Jump,Type(),{Operand::label(edge_target(arg(i,0).ref))}); break;
    case Opcode::Branch: {
        auto cond = arg(i,0);
        X86Condition cc = XC_NE;
        if (cond.kind == lowir_model::Operand::Temporary && state(cond.ref).compare_branch)
            cc = branch_condition(p.instructions[state(cond.ref).definition-1].operation);
        else {
            Type t = value_type(cond,Type::I64);
            Operand test = in_register(value(cond,t),t,XR_R10);
            emit(Op::Compare,t,{test,Operand::imm(0)});
        }
        emit(Op::Jcc,Type(),{Operand::label(edge_target(arg(i,1).ref))}).condition = cc;
        emit(Op::Jump,Type(),{Operand::label(edge_target(arg(i,2).ref))});
        break;
    }
    case Opcode::Switch: {
        Type t = value_type(arg(i,0),Type::I64);
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
        else {
            auto result = in_register(value(arg(i,0),i.type),value_type(arg(i,0),i.type),XR_RAX);
            emit(Op::Return,i.type,{result});
        }
        break;
    case Opcode::Unreachable: emit(Op::Trap,Type(),{}); break;
    default: throw ParseError("invalid control instruction");
    }
}
} // namespace native
