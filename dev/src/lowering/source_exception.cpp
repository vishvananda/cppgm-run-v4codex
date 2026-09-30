#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
using syntax::Kind;
std::string Procedural::support_type_name(TypeId id)
{
    // This spelling is output presentation only. All support caches retain
    // canonical TypeId keys, independently of rendering or first-use order.
    auto t = sem.types[id];
    if (t.kind == TypeKind::Fundamental) {
        std::string name = fundamental_name(t.fundamental);
        for (auto& c : name) if (c == ' ') c = '_';
        return name;
    }
    abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Type; target.type = abi_type(id);
    return abi_mangle::mangle(abi,target);
}
SymbolId Procedural::exception_function(unsigned role)
{
    if (linkage.exception_functions[role]) return linkage.exception_functions[role];
    if (role == 5 && linkage.abort_runtime) p.symbols[linkage.abort_runtime.index-1].metadata.role = SR_NONE;
    static const char* names[] = {"__cxa_allocate_exception","__cxa_begin_catch","__cxa_end_catch","__cxa_rethrow","__cxa_throw","_ZSt9terminatev","__cxa_free_exception"};
    static const SymbolRole roles[] = {SR_EH_ALLOCATE_EXCEPTION,SR_EH_BEGIN_CATCH,SR_EH_END_CATCH,SR_EH_RETHROW,SR_EH_THROW,SR_TERMINATE,SR_EH_FREE_EXCEPTION};
    auto ptr = sem.types.compound(TypeKind::Pointer,sem.types.fundamental(FT_VOID));
    std::vector<TypeId> params;
    if (role == 0) params.push_back(sem.types.fundamental(FT_UNSIGNED_LONG_INT));
    if (role == 1 || role == 6) params.push_back(ptr);
    if (role == 1 && !linkage.presentation && !linkage.host) params.push_back(ptr);
    if (role == 4) params = {ptr,ptr,ptr};
    auto sig = sem.types.function(role < 2 ? ptr : sem.types.fundamental(FT_VOID),params,false);
    Function f; f.symbol = fresh_symbol("@exception_runtime"); f.declaration = true;
    FunctionId owner(p.functions.size()+1); f.signature = signature(sig,owner);
    if (role >= 3 && role <= 5) p.signatures[f.signature.index-1].boundary.returns = ir_model::CRM_NORETURN;
    if (role == 5 || role == 6) p.signatures[f.signature.index-1].boundary.unwind = ir_model::CUM_NO;
    p.functions.push_back(f); linkage.exception_functions[role] = f.symbol;
    auto& s = p.symbols[f.symbol.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
    s.metadata.binding = SBM_STRONG; s.metadata.linkage = LLM_C;
    s.metadata.role = roles[role]; s.metadata.object = p.intern(names[role]);
    return f.symbol;
}
SymbolId Procedural::exception_type(TypeId t)
{
    auto info = rtti_type(t);
    if (!linkage.presentation) return info;
    if (sem.types[t].kind != TypeKind::Fundamental &&
        !(sem.types[t].kind == TypeKind::Pointer && sem.types[sem.types[t].child].kind == TypeKind::Fundamental)) return info;
    if (auto old = exception_rtti.get(t)) return SymbolId(old);
    auto symbol = fresh_symbol("@__external_rtti__"+support_type_name(t));
    Global g; g.symbol = symbol; g.declaration = true; p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
    s.metadata.binding = SBM_STRONG; s.metadata.role = SR_RTTI_DATA;
    s.metadata.object = p.symbols[info.index-1].metadata.object;
    exception_rtti.put(t,symbol.index); return symbol;
}
void Procedural::exception_object(TypeId t)
{
    if (exception_storage.get(t)) return;
    auto symbol = fresh_symbol("@__ehobj_"+support_type_name(t)); exception_storage.put(t,symbol.index);
    Global g; g.symbol = symbol; g.structured = true; g.data.begin = p.data.size(); g.data.count = 1;
    DataItem item; item.kind = DataItem::Zero; item.zero_bytes = sem.object_size(t); p.data.push_back(item); p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
    s.metadata.binding = SBM_INTERNAL; s.metadata.object = s.name;
}
bool Procedural::exception_clauses(std::uint32_t context, bool cleanup)
{
    bool first = true, crossed_handler = false;
    bool has_cleanup = false;
    std::uint32_t prior_live = 0;
    for (auto i = context; i; i = exception_contexts[i].parent) {
        auto c = exception_contexts[i];
        if (c.handler) { crossed_handler = true; continue; }
        if (!first && (crossed_handler || prior_live != c.live)) { emit(Opcode::EhCleanup,IRType(),{}); has_cleanup = true; }
        bool all = false;
        for (auto h = ast[child(c.node,Kind::Compound)].next; h; h = ast[h].next) {
            auto t = sem.facts[h].type;
            auto selector = exception_selector(h);
            if (t) {
                Instruction clause(Opcode::EhCatch); clause.catch_binding = catch_binding(h);
                emit(clause,{Operand::symbol(exception_type(t)),Operand::integer(selector)});
            }
            else { emit(Opcode::EhCatchAll,IRType(),{Operand::integer(selector)}); all = true; }
        }
        if (cleanup) { emit(Opcode::EhCleanup,IRType(),{}); cleanup = false; has_cleanup = true; }
        if (all) return has_cleanup;
        first = false; crossed_handler = false; prior_live = c.live;
    }
    // A miss can escape the function without encountering another typed
    // handler. Keep this landing pad reachable for the still-live lexical
    // prefix and any active handler that must be finished before resuming.
    if (!first && !has_cleanup && (crossed_handler || prior_live)) {
        emit(Opcode::EhCleanup,IRType(),{}); has_cleanup = true;
    }
    return has_cleanup;
}
void Procedural::exception_fallback()
{
    auto result = result_type();
    if (result == IRType::Void) emit(Opcode::Return,IRType(),{});
    else if (result.kind() == IRType::Object) {
        if (!class_return_slot) class_return_slot = builder->add_slot(0,result);
        Instruction zero(Opcode::ZeroInit); zero.bytes = result.bytes(); zero.alignment = result.alignment();
        emit(zero,{Operand::slot(class_return_slot)}); emit(Opcode::Return,result,{Operand::slot(class_return_slot)});
    } else emit(Opcode::Return,result,{result.floating() ? Operand::floating(0) : Operand::integer(0)});
}
Value Procedural::throw_expression(NodeId n)
{
    auto use = sem.throw_use(n);
    auto initial = live;
    if (use.source) {
        if (linkage.presentation) exception_object(use.type);
        Operand allocate[] = {Operand::symbol(exception_function(0)),Operand::integer(sem.object_size(use.type))};
        auto object = guarded_call(Instruction(Opcode::Call,IRType::Ptr),allocate,2);
        if (full_expression.open) {
            auto saved = builder->add_slot(0,IRType::Ptr);
            emit(Opcode::Store,IRType::Ptr,{object.operand,Operand::slot(saved)});
            close_expression_region();
            object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(saved)});
        }
        auto release = !linkage.presentation && sem.class_value(use.type) ? protect_exception(object.operand) : 0;
        object.type = use.type; object.address = true;
        if (sem.class_value(use.type)) construct_value(use.source,sem.conversion_fact(use.conversion),object);
        else store(converted(use.source,sem.conversion_fact(use.conversion)),object);
        if (release) retire_deallocation(release,initial);
        clean_inline(live,initial); close_expression_region();
        if (full_expression.enabled && unwind_live()) open_expression_region();
        auto info = emit(Opcode::Addr,IRType(),{Operand::symbol(exception_type(use.type))});
        Value dtor(Operand::integer(0),IRType::Ptr);
        if (use.destructor && !sem.trivial_destructor(use.type)) dtor = emit(Opcode::Addr,IRType(),{Operand::symbol(symbol(use.destructor))});
        Operand args[] = {Operand::symbol(exception_function(4)),object.operand,info.operand,dtor.operand};
        guarded_call(Instruction(Opcode::Call,IRType::Void),args,4);
    } else {
        Operand arg = Operand::symbol(exception_function(3));
        guarded_call(Instruction(Opcode::Call,IRType::Void),&arg,1);
    }
    if (full_expression.open) { full_expression.open = false; emit(Opcode::EhEnd,IRType(),{}); }
    full_expression = FullExpression(); live = initial;
    if (exception_context && !exception_contexts[exception_context].handler) emit(Opcode::EhEnd,IRType(),{});
    exception_fallback(); return Value();
}
void Procedural::exit_exception_contexts(NodeId target, std::uint32_t stop)
{
    auto saved = exception_context;
    for (auto i = exception_context; i && exception_contexts[i].node != target; i = exception_contexts[i].parent) {
        auto c = exception_contexts[i]; clean_inline(live,c.live);
        emit(Opcode::EhEnd,IRType(),{});
        if (c.handler) emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(2))});
        exception_context = c.parent;
    }
    if (target && (!exception_context || exception_contexts[exception_context].node != target))
        throw std::logic_error("jump exception target is not an active ancestor");
    clean_inline(live,stop);
    exception_context = saved;
}
void Procedural::try_statement(NodeId n)
{
    auto parent = exception_context, initial = live;
    auto protected_body = child(n,Kind::Compound);
    bool function_try = ast[n].kind == Kind::FunctionTry;
    bool lifecycle = function_try && (sem.constructor_member(active_function) || sem.destructor_member(active_function));
    auto dispatch = block(), entry = block(), end = block();
    ExceptionContext c; c.parent = parent; c.live = initial; c.node = n; c.entry = entry; c.has_catches = true;
    bool catches_all = false;
    for (auto h = ast[protected_body].next; h; h = ast[h].next) catches_all |= !sem.facts[h].type;
    // Summarize the same parent-linked clauses emitted by exception_clauses.
    // A catch-all ends the search; otherwise a changed live prefix or an
    // active handler needs cleanup before forwarding to the parent's clauses.
    c.cleanup_dispatch = !catches_all && (parent ?
        exception_contexts[parent].handler || initial != exception_contexts[parent].live ||
            exception_contexts[parent].cleanup_dispatch : initial != 0);
    auto context = exception_contexts.size(); exception_contexts.push_back(c); exception_context = context;
    emit(Opcode::EhTry,IRType(),{Operand::label(dispatch)});
    // Lifecycle cleanups must complete before function-try handlers are entered.
    // Their physical outer region owns dispatch after the subobject protocol.
    if (lifecycle) exception_context = 0;
    if (function_try && sem.constructor_member(active_function)) constructor_body(active_function,active_base_entry);
    if (function_try && sem.destructor_member(active_function)) {
        destructor_prologue(active_function); vpointer_store(sem.scopes[sem.entities[active_function].owner].entity);
    }
    statement(protected_body);
    if (function_try && destructor_handler) { destructor_finish(active_function,false); destructor_handler = BlockId(); }
    if (function_try && !ended) finish_constructor_handlers();
    exception_context = context;
    if (!ended) { emit(Opcode::EhEnd,IRType(),{}); jump(end); }
    exception_context = parent;
    start(dispatch);
    // A cleanup-bearing landing pad retains its protected region while it
    // runs. Retire that region before entering ordinary source catch matching.
    if (exception_clauses(context)) emit(Opcode::EhEnd,IRType(),{});
    jump(entry);
    start(entry);
    auto object = emit(Opcode::Exception,IRType::Ptr,{});
    auto selector = emit(Opcode::ExceptionSelector,IRType::I32,{});
    for (auto h = ast[protected_body].next; h; h = ast[h].next) {
        auto body = block(), next = block(), cleanup = block();
        auto match = emit(Opcode::Compare,IRType::I32,{selector.operand,Operand::integer(exception_selector(h))},Operation::Eq);
        emit(Opcode::Branch,IRType(),{match.operand,Operand::label(body),Operand::label(next)});
        start(body); live = initial;
        auto caught = begin_catch(object.operand,h);
        auto e = sem.facts[h].entity;
        bool named = e && sem.entities[e].name;
        if (named) {
            auto saved = builder->add_slot(0,IRType::Ptr); emit(Opcode::Store,IRType::Ptr,{caught.operand,Operand::slot(saved)});
        }
        emit(Opcode::EhCleanup,IRType(),{Operand::label(cleanup)});
        auto initialization = sem.handler_initializations.get(h);
        if (named || initialization) {
            auto t = sem.entities[e].type; objects[e] = source_slot(e);
            if (initialization) {
                auto failed = block(), ready = block();
                emit(Opcode::EhTry,IRType(),{Operand::label(failed)});
                // Failure while initializing the handler parameter terminates
                // (C++11 [except.throw]/7), before that parameter is live.
                auto suppress = emitting_cleanup; emitting_cleanup = true;
                typed_conversion(Value(caught.operand,IRType::Ptr,sem.facts[h].type,true),sem.conversion_fact(initialization),
                    address(Value(Operand::slot(objects[e]),type(t),t,true)));
                clean_inline(live,initial);
                emitting_cleanup = suppress;
                emit(Opcode::EhEnd,IRType(),{}); jump(ready);
                start(failed); emit(Opcode::EhCatchAll,IRType(),{Operand::integer(1)});
                auto exception = emit(Opcode::Exception,IRType::Ptr,{});
                begin_catch(exception.operand);
                emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(5))});
                emit(Opcode::EhEnd,IRType(),{}); exception_fallback();
                start(ready);
                if (auto state = object_lifetime(e)) live = state;
            } else if ((linkage.presentation || linkage.host) && reference(t) && sem.types[sem.types[t].child].kind == TypeKind::Pointer) {
                // The Itanium runtime returns the adjusted pointer value for
                // pointer exceptions. A reference parameter needs an address
                // of pointer storage, not that pointee address.
                auto pointer = builder->add_slot(0,IRType::Ptr);
                emit(Opcode::Store,IRType::Ptr,{caught.operand,Operand::slot(pointer)});
                auto location = emit(Opcode::Addr,IRType(),{Operand::slot(pointer)});
                emit(Opcode::Store,IRType::Ptr,{location.operand,Operand::slot(objects[e])});
            } else if (reference(t) || sem.types[t].kind == TypeKind::Pointer) emit(Opcode::Store,IRType::Ptr,{caught.operand,Operand::slot(objects[e])});
            else store(load(Value(caught.operand,IRType::Ptr,t,true)),Value(Operand::slot(objects[e]),type(t),t,true));
        }
        c.parent = parent; c.handler = true; c.node = h; c.live = initial;
        c.has_catches = parent && exception_contexts[parent].has_catches;
        exception_context = exception_contexts.size(); exception_contexts.push_back(c);
        statement(ast[ast[h].first].next);
        if (lifecycle && !ended) {
            clean_inline(live,initial);
            emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(3))});
            exception_fallback();
        }
        if (!ended) {
            clean_inline(live,initial);
            emit(Opcode::EhEnd,IRType(),{});
            emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(2))}); jump(end);
        }
        exception_context = parent;
        start(cleanup); exception_clauses(parent);
        emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(2))});
        resume_exception(parent ? initial : 0,parent,true);
        start(next);
    }
    if (catches_all && (!parent || (!exception_contexts[parent].handler && !exception_contexts[parent].cleanup_dispatch))) {
        // The catch-all selector exhausts this dispatch. Its syntactic miss
        // edge cannot carry an exception or live cleanup state. Keep the O0
        // dispatch skeleton. A catch-only parent entry already defines its
        // retired region; cleanup-bearing joins still require balanced exits,
        // even on this impossible edge. Real misses retain full live state.
        if (parent) jump(exception_contexts[parent].entry);
        else emit(Opcode::Resume,IRType(),{});
    } else resume_exception(initial,parent,false);
    start(end); live = initial;
}
} }
