#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
bool Procedural::cleanup_expression(NodeId n)
{
    if (!n) return false;
    if (cleanup_expressions.empty()) cleanup_expressions.resize(ast.nodes.size());
    if (cleanup_expressions[n]) return cleanup_expressions[n] == 2;
    bool needed = sem.destructor_needed(sem.object_destructor(sem.object_fact(n).temporary));
    auto incoming = sem.expression_fact(n).incoming;
    if (incoming) {
        auto c = sem.conversion_fact(incoming);
        if (c.kind == semantic::Conversion::Kind::User)
            needed |= sem.destructor_needed(sem.object_destructor(sem.user_conversions[c.materialization].source_temporary));
        if (c.reference && c.materialization) {
            auto object = c.kind == semantic::Conversion::Kind::User ? sem.user_conversions[c.materialization].temporary : sem.conversion_objects[c.materialization].temporary;
            needed |= sem.destructor_needed(sem.object_destructor(object));
        }
    }
    for (NodeId child = ast[n].first; child; child = ast[child].next) needed |= cleanup_expression(child);
    cleanup_expressions[n] = needed ? 2 : 1;
    return needed;
}
SlotId Procedural::cleanup_selector(Value test, bool required)
{
    if (!required) return SlotId();
    Value truth = emit(Opcode::Compare,test.ir,{test.operand,test.ir.floating() ? Operand::floating(0) : Operand::integer(0)},Operation::Ne);
    SlotId slot = builder->add_slot(0,IRType::I64);
    emit(Opcode::Store,IRType::I64,{truth.operand,Operand::slot(slot)});
    return slot;
}
void Procedural::merge_temporaries(std::uint32_t common, std::uint32_t yes, std::uint32_t no, SlotId selector)
{
    if (yes == no) { live = yes; return; }
    if (!selector) throw std::logic_error("missing conditional lifetime selector");
    TemporaryState state; state.tail = common; state.depth = lifetime_state(common).depth+1;
    state.selector = selector; state.yes = yes; state.no = no;
    temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
}
void Procedural::destroy_lifetime(std::uint32_t id)
{
    auto action = lifetime_state(id);
    if (action.object) { destroy_object(action.object,action.destructor); return; }
    auto branch = temporary_states[(id & 0x7fffffffu)-1];
    Value test = emit(Opcode::Load,IRType::I64,{Operand::slot(branch.selector)});
    BlockId yes = block(), no = block(), end = block();
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(yes),Operand::label(no)});
    start(yes); clean_inline(branch.yes,branch.tail); jump(end);
    start(no); clean_inline(branch.no,branch.tail); jump(end);
    start(end); live = branch.tail;
}
} }
