#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
SymbolId Procedural::view_symbol(EntityId cls, unsigned view)
{
    auto key = (std::uint64_t(cls)<<32)|view;
    if (auto prior = view_symbols.get(key)) return SymbolId(prior);
    auto symbol = fresh_symbol("@vtable_view"); view_symbols.put(key,symbol.index);
    auto& meta = p.symbols[symbol.index-1].metadata;
    meta.binding = SBM_INTERNAL;
    // Keep the backend object identity distinct from its optional LowIR alias.
    // These private views have no externally mandated Itanium symbol name.
    meta.object = p.intern("__cppgm_vtable_view_"+std::to_string(symbol.index));
    return symbol;
}
SymbolId Procedural::virtual_target(const semantic::VirtualSlot& slot, bool deleting)
{
    auto e = slot.function;
    if (sem.member_fact(e).pure) {
        if (!pure_virtual) {
            pure_virtual = fresh_symbol("@pure_virtual");
            Function f; f.symbol = pure_virtual; f.declaration = true;
            f.signature = signature(sem.call_type(e)); p.functions.push_back(f);
            auto& s = p.symbols[pure_virtual.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = p.functions.size();
            s.metadata.binding = SBM_STRONG; s.metadata.object = p.intern("__cxa_pure_virtual");
            s.metadata.role = SR_PURE_VIRTUAL;
        }
        return pure_virtual;
    }
    if (!slot.this_adjustment && !slot.result_adjustment) return symbol(e,false,deleting);
    auto offset_id = [&](std::int64_t value) {
        auto id = thunk_adjustments.get(value);
        if (!id) { id = ++next_thunk_adjustment; thunk_adjustments.put(value,id); }
        return id;
    };
    auto pair = (std::uint64_t(offset_id(slot.this_adjustment))<<32)|offset_id(slot.result_adjustment);
    auto adjustment = thunk_pairs.get(pair);
    if (!adjustment) { adjustment = ++next_thunk_pair; thunk_pairs.put(pair,adjustment); }
    auto key = (std::uint64_t(e)<<32)|adjustment;
    auto& index = deleting ? deleting_thunk_symbols : thunk_symbols;
    if (auto prior = index.get(key)) return SymbolId(prior);
    auto output = fresh_symbol("@adjustor_thunk"); index.put(key,output.index);
    abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Thunk;
    target.function = abi_mangle::entity_function(abi,abi_function_context(e));
    if (deleting) target.function.terminal = abi_mangle::ABI_TERMINAL_DESTRUCTOR_DELETING;
    target.this_adjust = slot.this_adjustment;
    target.result_adjust = slot.result_adjustment; target.has_result_adjust = slot.result_adjustment != 0;
    auto& meta = p.symbols[output.index-1].metadata;
    meta.binding = internal_entity(e) ? SBM_INTERNAL : SBM_WEAK;
    auto object = abi_mangle::mangle(abi,target);
    if (meta.binding == SBM_INTERNAL && linkage.merge) object += "."+std::to_string(output.index);
    meta.object = p.intern(object);
    Function f; f.symbol = output;
    FunctionId owner(p.functions.size()+1); f.signature = signature(sem.call_type(e),owner);
    p.functions.push_back(f);
    auto& s = p.symbols[output.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
    adjustor_thunks.push_back({e,slot.this_adjustment,slot.result_adjustment,output,deleting});
    return output;
}
void Procedural::emit_adjustor_thunks()
{
    for (const auto& thunk : adjustor_thunks) {
        function = FunctionId(p.symbols[thunk.symbol.index-1].entity);
        builder.reset(new FunctionBuilder(p,function)); reset_lifetime(thunk.target);
        returned = sem.types[sem.entities[thunk.target].type].child;
        start(block());
        auto sig = p.signatures[p.functions[function.index-1].signature.index-1];
        std::vector<Operand> args = {Operand::symbol(symbol(thunk.target,false,thunk.deleting))};
        for (unsigned i = 0; i < sig.parameters.count; ++i)
            args.push_back(Operand::value(p.parameters[sig.parameters.begin+i].value));
        auto receiver = 1+unsigned(sem.indirect_value(returned));
        auto delta = Operand::integer(thunk.adjustment); delta.negative_integer = thunk.adjustment < 0;
        if (thunk.adjustment) args[receiver] = emit(Opcode::Index,IRType::I8,{args[receiver],delta}).operand;
        auto result = emit(Instruction(Opcode::Call,sig.result),args.data(),args.size());
        if (thunk.result_adjustment) {
            auto offset = Operand::integer(thunk.result_adjustment);
            if (reference(returned)) result = emit(Opcode::Index,IRType::I8,{result.operand,offset});
            else {
                auto storage = builder->add_slot(0,IRType::Ptr);
                emit(Opcode::Store,IRType::Ptr,{Operand::integer(0),Operand::slot(storage)});
                auto test = emit(Opcode::Compare,IRType::Ptr,{result.operand,Operand::integer(0)},Operation::Eq);
                auto adjust = block(), end = block();
                emit(Opcode::Branch,IRType(),{test.operand,Operand::label(end),Operand::label(adjust)});
                start(adjust); result = emit(Opcode::Index,IRType::I8,{result.operand,offset});
                emit(Opcode::Store,IRType::Ptr,{result.operand,Operand::slot(storage)}); jump(end);
                start(end); result = emit(Opcode::Load,IRType::Ptr,{Operand::slot(storage)});
            }
        }
        if (sig.result == IRType::Void) emit(Opcode::Return,IRType(),{});
        else emit(Opcode::Return,sig.result,{result.operand});
        builder.reset();
    }
}
} }
