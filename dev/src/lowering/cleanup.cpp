#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::reset_lifetime(EntityId e)
{
    active_function = e; live = 0; emitting_cleanup = false; resume_emitted = false; cleanup_cursor = 0; slot_names = semantic::Index();
    cleanup_return = class_return_slot = SlotId(); resume_terminal = destructor_handler = destructor_end = destructor_epilogue = BlockId();
    cleanup_index = semantic::Index(); return_terminals = semantic::Index();
    temporary_states.clear(); cleanup_blocks.clear(); constructed_subobjects.clear();
    full_expression = FullExpression();
}
semantic::LifetimeState Procedural::lifetime_state(std::uint32_t state) const
{
    if (state & 0x80000000u) return temporary_states[(state & 0x7fffffffu)-1];
    return sem.lifetimes[state];
}
void Procedural::activate_temporary(EntityId e)
{
    if (sem.static_temporary(e).object) {
        if (auto guard = reference_guards.get(e)) emit(Opcode::Store,IRType::I64,{Operand::integer(1),Operand::symbol(SymbolId(guard))});
        return;
    }
    if (sem.object_lifetime(e)) return;
    EntityId dtor = sem.object_destructor(e);
    if (!sem.temporary_cleanup(e)) return;
    if (full_expression.scalar_unreachable) { close_expression_region(); return; }
    bool reopen = full_expression.open;
    close_expression_region();
    TemporaryState state; state.object = e; state.destructor = dtor; state.tail = live;
    state.depth = lifetime_state(live).depth + 1;
    temporary_states.push_back(state); live = 0x80000000u | temporary_states.size();
    if (reopen) open_expression_region();
}
SlotId Procedural::source_slot(EntityId e)
{
    auto name = p.intern("$" + spelling(sem.entities[e].name));
    if (slot_names.get(name)) name = p.intern("$" + spelling(sem.entities[e].name) + "__" + std::to_string(e));
    slot_names.put(name, 1);
    return builder->add_slot(name, type(sem.entities[e].type));
}
void Procedural::clean_inline(std::uint32_t state, std::uint32_t stop)
{
    while (state != stop) {
        if (!state) throw std::logic_error("cleanup target is not a lexical ancestor");
        auto action = lifetime_state(state);
        // A destructor that can throw must unwind only objects still alive
        // after its own lifetime ends, never re-enter its old cleanup prefix.
        if (full_expression.open && action.object && !sem.function_nonthrowing(action.destructor))
            close_expression_region();
        live = action.tail;
        destroy_lifetime(state); state = action.tail;
    }
    live = stop;
}
BlockId Procedural::cleanup_suffix(std::uint32_t state, BlockId terminal)
{
    if (!state) return terminal;
    auto key = (std::uint64_t(state) << 32) | terminal.index;
    if (auto existing = cleanup_index.get(key)) return BlockId(existing);
    BlockId tail = cleanup_suffix(lifetime_state(state).tail, terminal);
    BlockId head = block(); cleanup_index.put(key, head.index);
    cleanup_blocks.push_back({state, tail, head});
    return head;
}
Value Procedural::guarded_call(Instruction i, const Operand* args, std::size_t count)
{
    bool no_throw = false;
    if (count && args[0].kind == Operand::Symbol) {
        auto s = p.symbols[args[0].ref-1];
        if (s.kind == Symbol::FunctionSymbol)
            no_throw = p.signatures[p.functions[s.entity-1].signature.index-1].boundary.unwind == ir_model::CUM_NO;
    }
    if (!live || emitting_cleanup || no_throw || full_expression.scalar_unreachable) return emit(i, args, count);
    if (full_expression.enabled) { open_expression_region(); return emit(i,args,count); }
    if (!resume_terminal) resume_terminal = block();
    auto cleanup = cleanup_suffix(live, resume_terminal);
    emit(Opcode::EhTry, IRType(), {Operand::label(cleanup)});
    Value result = emit(i, args, count);
    emit(Opcode::EhEnd, IRType(), {});
    auto continuation = block(); jump(continuation); flush_cleanups(); start(continuation);
    return result;
}
void Procedural::return_statement(NodeId n)
{
    auto life = sem.lifetime_use(n);
    bool has_value = result_type() != IRType::Void;
    Value value;
    auto class_return = sem.class_return(n);
    auto conversion = sem.conversion_fact(class_return.conversion);
    bool omit = class_return.source && conversion.kind == semantic::Conversion::Kind::Construction && sem.conversion_objects[conversion.materialization].elided;
    begin_full_expression(ast[n].first,omit);
    if (class_return.source) {
        Value destination = return_destination;
        if (has_value) {
            if (!class_return_slot) class_return_slot = builder->add_slot(0,type(returned));
            SlotId slot = class_return_slot;
            value = Value(Operand::slot(slot),type(returned),returned);
            destination = address(Value(value.operand,value.ir,value.type,true));
        }
        if (!class_return.local || class_return.local != sem.return_object(active_function))
            construct_value(class_return.source,sem.conversion_fact(class_return.conversion),destination,true);
    } else if (has_value) {
        NodeId operand = ast[n].first;
        if (!operand) value = Value(Operand::integer(0), type(returned));
        else if (auto conversion = sem.expression_fact(operand).incoming)
            value = converted(operand, sem.conversion_fact(conversion));
        else value = convert(expression(operand, reference(returned)), returned);
    }
    else if (ast[n].first) expression(ast[n].first);
    finish_full_expression(life.entry);
    if (destructor_handler) {
        clean_inline(life.entry, 0);
        emit(Opcode::EhEnd, IRType(), {});
        if (!destructor_epilogue) destructor_epilogue = block();
        jump(destructor_epilogue); return;
    }
    if (life.entry && sem.return_count(life.entry, life.context) > 1) {
        if (has_value) {
            if (!cleanup_return) cleanup_return = builder->add_slot(0, result_type());
            if (type(returned).kind() == IRType::Object) {
                Value target = address(Value(Operand::slot(cleanup_return),type(returned),returned,true));
                Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(returned); copy.alignment = sem.object_alignment(returned);
                emit(copy,{value.operand,target.operand});
            } else emit(Opcode::Store, result_type(), {value.operand, Operand::slot(cleanup_return)});
        }
        auto terminal = BlockId(return_terminals.get(life.context));
        if (!terminal) { terminal = block(); return_terminals.put(life.context, terminal.index); cleanup_blocks.push_back({0, BlockId(), terminal}); }
        jump(cleanup_suffix(life.entry, terminal)); flush_cleanups(); return;
    }
    clean_inline(life.entry, 0); finish_constructor_handlers();
    if (has_value) emit(Opcode::Return, result_type(), {value.operand});
    else emit(Opcode::Return, IRType(), {});
}
void Procedural::flush_cleanups()
{
    bool saved_cleanup = emitting_cleanup; auto saved_live = live;
    emitting_cleanup = true; live = 0;
    if (resume_terminal && !resume_emitted) { resume_emitted = true; start(resume_terminal); emit(Opcode::Resume, IRType(), {}); }
    while (cleanup_cursor < cleanup_blocks.size()) {
        auto entry = cleanup_blocks[cleanup_cursor++];
        start(entry.block);
        if (!entry.state) {
            if (result_type() == IRType::Void) emit(Opcode::Return, IRType(), {});
            else emit(Opcode::Return, result_type(), {Operand::slot(cleanup_return)});
        } else if (!entry.next) {
            clean_inline(entry.state,0); emit(Opcode::Resume,IRType(),{});
        } else {
            destroy_lifetime(entry.state); jump(entry.next);
        }
    }
    emitting_cleanup = saved_cleanup; live = saved_live;
}
void Procedural::emit_cleanups()
{
    flush_cleanups(); emitting_cleanup = true; live = 0;
    for (auto entry : constructed_subobjects) {
        start(entry.handler);
        auto action = entry.action;
        if (sem.types[action.type].kind == TypeKind::Array) {
            array_destroy(sem.type_destructor(action.type), action.type, Value(Operand::slot(this_slot), IRType::Ptr), true,
                {{action.field ? sem.entities[action.field].member_offset : 0, action.field != 0}});
            emit(Opcode::EhEnd, IRType(), {}); emit(Opcode::Resume, IRType(), {}); continue;
        }
        Value base = emit(Opcode::Load, IRType::Ptr, {Operand::slot(this_slot)});
        Instruction i(Opcode::Index, IRType::I8); i.projection = action.field ? ir_model::IPK_FIELD : ir_model::IPK_NONE;
        Value at = emit(i, {base.operand, Operand::integer(action.field ? sem.entities[action.field].member_offset : 0)});
        EntityId dtor = sem.type_destructor(action.type);
        destroy(dtor, action.type, at);
        emit(Opcode::EhEnd, IRType(), {}); emit(Opcode::Resume, IRType(), {});
    }
}
} }
