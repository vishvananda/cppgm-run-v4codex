#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
bool Procedural::unwind_live() const
{
    auto context = exception_context;
    while (context && exception_contexts[context].handler) context = exception_contexts[context].parent;
    return live != (context ? exception_contexts[context].live : 0);
}
BlockId Procedural::unwind_suffix(std::uint32_t state, std::uint32_t context, bool pop)
{
    if (!context) {
        if (!resume_terminal) resume_terminal = block();
        return cleanup_suffix(state,resume_terminal);
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
    auto next = unwind_suffix(live,exception_context);
    if (!exception_context) return next;
    auto key = (std::uint64_t(exception_context)<<32)|live;
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
