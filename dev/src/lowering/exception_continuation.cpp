#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
bool Procedural::unwind_live() const
{
    auto context = exception_context;
    while (context && exception_contexts[context].handler) context = exception_contexts[context].parent;
    return (returning_object && sem.object_lifetime(returning_object)) || live != (context ? exception_contexts[context].live : 0);
}
std::uint32_t Procedural::return_unwind_state()
{
    auto state = returning_object ? sem.object_lifetime(returning_object) : 0;
    if (!state || lifetime_state(live).depth >= sem.lifetimes[state].depth) return live;
    if (auto known = return_unwind_states.get(live+1)) return known;
    // Normal return skipped the result's local destructor. A later destructor
    // may still fail, in which case the constructed result is owned here too.
    TemporaryState result; result.object = returning_object;
    result.destructor = sem.object_destructor(returning_object);
    result.location = object_addresses[returning_object]; result.tail = live;
    result.depth = lifetime_state(live).depth+1;
    temporary_states.push_back(result);
    auto id = 0x80000000u | temporary_states.size();
    return_unwind_states.put(live+1,id); return id;
}
BlockId Procedural::unwind_suffix(std::uint32_t state, std::uint32_t context, bool pop)
{
    if (!context) {
        if (!resume_terminal) resume_terminal = block();
        // Crossing a nonthrowing function boundary terminates. Its incoming
        // value parameters are destroyed on ordinary exit, but need not be
        // unwound before terminate ([except.terminate]). Local expression
        // ownership and all source catches remain inside that boundary.
        auto stop = nonthrowing_parameters;
        if (lifetime_state(state).depth < lifetime_state(stop).depth) stop = state;
        return cleanup_suffix(state,resume_terminal,stop);
    }
    auto c = exception_contexts[context];
    auto key = std::uint64_t(context)*2+pop;
    BlockId terminal(unwind_terminals.get(key));
    if (!terminal) {
        auto next = c.handler ? unwind_suffix(c.live,c.parent,false) : c.entry;
        terminal = block(); unwind_terminals.put(key,terminal.index);
        unwind_continuations.push_back({c.handler ? UnwindKind::HandlerExit : UnwindKind::TryExit,context,terminal,next,pop});
    }
    return cleanup_suffix(state,terminal,c.live);
}
BlockId Procedural::unwind_target()
{
    auto state = return_unwind_state();
    if (full_expression.lexical && state && !exception_context) {
        auto entry = block(); cleanup_blocks.push_back({state,BlockId(),entry}); return entry;
    }
    auto next = unwind_suffix(state,exception_context);
    // A terminal with no live objects and no catch clauses is already the
    // complete dispatch. Object-bearing paths retain their cleanup entry.
    if (!exception_context || (!state && !exception_contexts[exception_context].has_catches)) return next;
    auto key = (std::uint64_t(exception_context)<<32)|state;
    if (auto old = unwind_dispatches.get(key)) return BlockId(old);
    auto dispatch = block(); unwind_dispatches.put(key,dispatch.index);
    unwind_continuations.push_back({UnwindKind::Dispatch,exception_context,dispatch,next,false});
    return dispatch;
}
void Procedural::flush_unwind_continuations()
{
    while (unwind_cursor < unwind_continuations.size()) {
        auto record = unwind_continuations[unwind_cursor++]; start(record.block);
        if (record.kind == UnwindKind::Dispatch) exception_clauses(record.context,true);
        else {
            if (record.pop) emit(Opcode::EhEnd,IRType(),{});
            if (record.kind == UnwindKind::HandlerExit)
                emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(2))});
            emit(Opcode::EhEnd,IRType(),{});
        }
        jump(record.next);
    }
}
void Procedural::resume_exception(std::uint32_t state, std::uint32_t context, bool pop)
{
    bool saved_cleanup = emitting_cleanup; emitting_cleanup = true;
    if (pop) emit(Opcode::EhEnd,IRType(),{});
    for (auto i = context; i; i = exception_contexts[i].parent) {
        auto c = exception_contexts[i]; clean_inline(state,c.live); state = c.live;
        if (!c.handler) {
            emit(Opcode::EhEnd,IRType(),{});
            jump(c.entry); emitting_cleanup = saved_cleanup; return;
        }
        emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(2))});
        emit(Opcode::EhEnd,IRType(),{});
    }
    clean_inline(state,0); emit(Opcode::Resume,IRType(),{});
    emitting_cleanup = saved_cleanup;
}
} }
