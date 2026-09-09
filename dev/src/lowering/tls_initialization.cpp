#include "lowering/procedural.h"

namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::prepare_tls(EntityId e)
{
    TlsInitializer tls; tls.object = e;
    tls.guard = fresh_symbol("@__cppgm_tls_guard_" + spelling(sem.entities[e].name));
    Global guard; guard.symbol = tls.guard; guard.type = IRType::I64;
    guard.data.begin = p.data.size(); guard.data.count = 1;
    DataItem zero; zero.zero_bytes = 8; p.data.push_back(zero); p.globals.push_back(guard);
    auto& g = p.symbols[tls.guard.index-1]; g.kind = Symbol::GlobalSymbol; g.entity = p.globals.size();
    g.metadata.binding = SBM_INTERNAL; g.metadata.storage = GSM_THREAD_LOCAL;
    g.metadata.object = p.intern(".Lcppgm.tls." + std::to_string(tls.guard.index));
    auto add = [&](const std::string& name, bool pointer, bool declaration) {
        Function f; f.symbol = fresh_symbol(name); f.declaration = declaration;
        FunctionId id(p.functions.size()+1);
        TypeId result = sem.types.fundamental(FT_VOID);
        if (pointer) result = sem.types.compound(TypeKind::Pointer, result);
        f.signature = signature(sem.types.function(result, {}, false), id); p.functions.push_back(f);
        auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = Symbol::FunctionSymbol; symbol.entity = id.index; symbol.metadata.binding = SBM_INTERNAL;
        symbol.metadata.object = p.intern(".Lcppgm.tls." + std::to_string(f.symbol.index));
        return f.symbol;
    };
    SymbolId guard_wrapper = add("@__cppgm_tls_guard_wrapper", true, true);
    p.symbols[guard_wrapper.index-1].metadata.tls_for = tls.guard;
    tls.initializer = add("@__cppgm_tls_init_" + spelling(sem.entities[e].name), false, false);
    tls.wrapper = add("@__cppgm_tls_wrapper_" + spelling(sem.entities[e].name), true, false);
    auto& wrapper = p.symbols[tls.wrapper.index-1]; wrapper.metadata.tls_for = symbols[e];
    if (p.symbols[symbols[e].index-1].metadata.binding != SBM_INTERNAL) {
        abi_mangle::Target target; target.kind = abi_mangle::TargetKind::TlsWrapper;
        target.type = abi.name(abi_scope(sem.entities[e].owner), spelling(sem.entities[e].name));
        wrapper.metadata.object = p.intern(abi_mangle::mangle(abi, target)); wrapper.metadata.binding = SBM_WEAK;
    }
    tls_wrappers.put(e, tls.wrapper.index); tls_initializers.push_back(tls);
}
void Procedural::emit_tls_initializers()
{
    for (auto tls : tls_initializers) {
        reset_lifetime(0); active_tls = tls.object;
        function = FunctionId(p.symbols[tls.initializer.index-1].entity);
        builder.reset(new FunctionBuilder(p, function)); this_slot = SlotId(); start(block());
        Value guard = emit(Opcode::Load, IRType::I64, {Operand::symbol(tls.guard)});
        Value done = emit(Opcode::Compare, IRType::I64, {guard.operand, Operand::integer(0)}, Operation::Ne);
        BlockId run = block(), end = block();
        emit(Opcode::Branch, IRType(), {done.operand, Operand::label(end), Operand::label(run)});
        start(run);
        auto entity = sem.entities[tls.object];
        Value object(Operand::symbol(symbols[tls.object]), type(entity.type), entity.type, true);
        if (entity.initializer) initialize(entity.initializer, entity.type, object);
        else if (sem.types[entity.type].kind == TypeKind::Array) array_construct(sem.object_constructor(tls.object), entity.type, object, false, {});
        else construct(sem.object_constructor(tls.object), 0, address(object));
        clean_inline(live, 0);
        emit(Opcode::Store, IRType::I64, {Operand::integer(1), Operand::symbol(tls.guard)}); jump(end);
        start(end); emit(Opcode::Return, IRType(), {}); builder.reset();
        function = FunctionId(p.symbols[tls.wrapper.index-1].entity);
        builder.reset(new FunctionBuilder(p, function)); start(block());
        emit(Opcode::Call, IRType::Void, {Operand::symbol(tls.initializer)});
        Value pointer = emit(Opcode::Addr, IRType(), {Operand::symbol(symbols[tls.object])});
        emit(Opcode::Return, IRType::Ptr, {pointer.operand}); builder.reset(); active_tls = 0;
    }
}
} }
