#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::complete_subobject(TypeId type, Value at, std::uint32_t before,
    bool retain, bool defaults, semantic::Index& retired)
{
    if (!defaults) {
        if (retain) activate_subobject(type,at,SlotId(),retired);
        return;
    }
    // [class.temporary]/4: default-argument temporaries die before the next
    // array element. The successfully constructed element is already alive
    // if one of those destructors throws. Rebase only the new temporary suffix;
    // previously published unwind states still describe incomplete construction.
    auto temporaries = live; live = before;
    if (retain || temporaries != before) activate_subobject(type,at,SlotId(),retired);
    auto prefix = live;
    if (temporaries != before) {
        semantic::Index empty, cache;
        live = retire_construction(temporaries,before,empty,cache,prefix);
        clean_inline(live,prefix); close_expression_region();
    }
}
void Procedural::activate_subobject(TypeId type, Value at, SlotId count, semantic::Index& retired)
{
    auto destructor = sem.type_destructor(type);
    if (!sem.destructor_needed(destructor)) return;
    close_expression_region();
    // This is a completed destination subobject, not a synthesized frontend
    // variable. Its canonical type and exact emitted address are sufficient
    // for cleanup; the enclosing aggregate retires the prefix on success.
    TemporaryState state; state.destroyed_type = type; state.destructor = destructor;
    // Keep the address in explicit EH-live storage. Shared cleanup suffixes
    // can be entered after many later calls, across the native unwind edge.
    state.saved_location = builder->add_slot(0,IRType::Ptr);
    emit(Opcode::Store,IRType::Ptr,{at.operand,Operand::slot(state.saved_location)});
    state.constructed = count;
    state.tail = live; state.depth = lifetime_state(live).depth+1;
    temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
    retired.put(live,1);
}
} }
