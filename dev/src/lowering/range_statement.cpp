#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::range_statement(NodeId n)
{
    auto plan = sem.range_plan(n); auto lifetime = sem.lifetime_use(n);
    if (plan.initialize_range) object(plan.range);
    SlotId index;
    if (plan.array || plan.list_element) {
        index = builder->add_slot(0,type(plan.index_type));
        emit(Opcode::Store,type(plan.index_type),{Operand::integer(0),Operand::slot(index)});
    } else {
        auto begin_args = plan.first.supplied ? std::vector<EntityId>{plan.range} : std::vector<EntityId>();
        range_initialize(plan.begin,plan.first,begin_args,plan.begin_conversion);
        auto end_args = plan.last.supplied ? std::vector<EntityId>{plan.range} : std::vector<EntityId>();
        range_initialize(plan.end,plan.last,end_args,plan.end_conversion);
    }
    auto cond = block(), body = block(), step = block(), end = block();
    auto old_break = break_target, old_continue = continue_target;
    break_target = end; continue_target = step;
    jump(cond); start(cond); live = plan.loop_live;
    Value test;
    if (plan.array || plan.list_element) {
        auto current = emit(Opcode::Load,type(plan.index_type),{Operand::slot(index)});
        Operand bound = Operand::integer(sem.types[plan.array].bound);
        if (plan.list_element) {
            auto base = address(binding(plan.range));
            auto count = load(field(base,plan.list_size)); bound = count.operand;
            current = coerce(current,IRType::I64,false);
        }
        test = emit(Opcode::Compare,current.ir,{current.operand,bound},Operation::Lt);
    } else {
        full_expression.enabled = full_expression.lexical = live != 0;
        test = range_operation(plan.test,{binding(plan.begin),binding(plan.end)});
        auto c = sem.conversion_fact(plan.condition_conversion);
        // A bool-producing call already supplies the branch operand; widening
        // it would add unnecessary work to every iteration.
        if (!(c.kind == semantic::Conversion::Kind::Standard && c.rank == 0)) test = typed_conversion(test,c);
        else test = load(test);
        finish_full_expression(plan.loop_live);
    }
    auto exit = end;
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(body),Operand::label(exit)});
    start(body); live = plan.loop_live;
    if (plan.array || plan.list_element) {
        auto base = address(binding(plan.range));
        if (plan.list_element) base = load(field(base,plan.list_begin));
        auto current = emit(Opcode::Load,type(plan.index_type),{Operand::slot(index)});
        auto t = plan.list_element ? sem.types[sem.entities[plan.list_begin].type].child : sem.types[plan.array].child;
        auto element_type = type(t);
        if (element_type.kind() == IRType::Object) {
            current = coerce(current,IRType::I64,sem.unsigned_type(plan.index_type));
            current = emit(Opcode::Binary,IRType::I64,{current.operand,Operand::integer(sem.object_size(t))},Operation::Mul);
            element_type = IRType::I8;
        }
        Instruction projection(Opcode::Index,element_type); projection.projection = ir_model::IPK_ARRAY_ELEMENT;
        auto value = emit(projection,{base.operand,current.operand}); value.type = t; value.address = true;
        auto target = sem.entities[plan.variable].type;
        if (!objects[plan.variable] || p.slots[objects[plan.variable].index-1].owner.index != function.index)
            objects[plan.variable] = source_slot(plan.variable);
        Value location(Operand::slot(objects[plan.variable]),type(target),target,true);
        auto c = sem.conversion_fact(plan.element_conversion);
        full_expression.enabled = full_expression.lexical = live != 0;
        if (sem.class_value(target)) typed_conversion(value,c,address(location));
        else store(typed_conversion(value,c),location);
        finish_full_expression(plan.loop_live);
    } else range_initialize(plan.variable,plan.element,{plan.begin},plan.element_conversion);
    live = plan.body_live; statement(plan.body);
    if (!ended) clean_inline(live,plan.loop_live);
    jump(step); start(step); live = plan.loop_live;
    if (plan.array || plan.list_element) {
        auto current = emit(Opcode::Load,type(plan.index_type),{Operand::slot(index)});
        auto next = emit(Opcode::Binary,current.ir,{current.operand,Operand::integer(1)},Operation::Add);
        emit(Opcode::Store,current.ir,{next.operand,Operand::slot(index)});
    } else {
        full_expression.enabled = full_expression.lexical = live != 0;
        range_operation(plan.next,{binding(plan.begin)});
        finish_full_expression(plan.loop_live);
    }
    jump(cond);
    start(end); clean_inline(plan.loop_live,lifetime.entry);
    break_target = old_break; continue_target = old_continue;
}
} }
