#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
SymbolId Procedural::view_symbol(EntityId cls, unsigned view)
{
    auto key = (std::uint64_t(vtable_symbol(cls).index)<<32)|view;
    if (auto prior = linkage.view_symbols.get(key)) return SymbolId(prior);
    auto symbol = fresh_symbol("@vtable_view"); linkage.view_symbols.put(key,symbol.index);
    auto& meta = p.symbols[symbol.index-1].metadata;
    meta.binding = SBM_INTERNAL;
    if (sem.virtual_class(cls).key_function && !sem.entities[cls].specialization && !internal_entity(cls)) {
        // PA23 represents virtual-base segments as separate LowIR globals.
        // A key-owned segment therefore needs the same support-symbol identity
        // in a referencing TU and in its defining TU, just like its main table.
        abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Vtable;
        target.type = abi_type(sem.entities[cls].type);
        meta.binding = SBM_WEAK;
        meta.object = p.intern(abi_mangle::mangle(abi,target)+".cppgm.view."+std::to_string(view));
        return symbol;
    }
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
    auto callee = symbol(e,false,deleting);
    if (!slot.this_adjustment && !slot.result_adjustment && !slot.result_virtual_row) return callee;
    // A thunk is a program ABI entry, even when several TU-local table facts
    // request it. The target symbol includes internal linkage and D0/D1 identity;
    // intern all adjustment components in the same program-owned namespace.
    auto offset_id = [&](std::int64_t value) {
        auto id = linkage.thunk_adjustments.get(value);
        if (!id) { id = ++linkage.next_thunk_adjustment; linkage.thunk_adjustments.put(value,id); }
        return id;
    };
    auto result_pair = (std::uint64_t(offset_id(slot.result_adjustment))<<32)|offset_id(slot.result_virtual_row);
    auto result_id = linkage.thunk_result_pairs.get(result_pair);
    if (!result_id) { result_id = ++linkage.next_thunk_result; linkage.thunk_result_pairs.put(result_pair,result_id); }
    auto pair = (std::uint64_t(offset_id(slot.this_adjustment))<<32)|result_id;
    auto adjustment = linkage.thunk_pairs.get(pair);
    if (!adjustment) { adjustment = ++linkage.next_thunk_pair; linkage.thunk_pairs.put(pair,adjustment); }
    auto key = (std::uint64_t(callee.index)<<32)|adjustment;
    if (auto prior = linkage.thunk_symbols.get(key)) return SymbolId(prior);
    auto output = fresh_symbol("@adjustor_thunk"); linkage.thunk_symbols.put(key,output.index);
    abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Thunk;
    target.function = abi_mangle::entity_function(abi,abi_function_context(e));
    if (deleting) target.function.terminal = abi_mangle::ABI_TERMINAL_DESTRUCTOR_DELETING;
    target.this_adjust = slot.this_adjustment;
    target.result_adjust = slot.result_adjustment; target.has_result_adjust = slot.result_adjustment != 0 || slot.result_virtual_row != 0;
    target.virtual_result = slot.result_virtual_row != 0;
    target.result_vcall_offset = slot.result_virtual_row;
    auto& meta = p.symbols[output.index-1].metadata;
    meta.binding = internal_entity(e) ? SBM_INTERNAL : SBM_WEAK;
    auto object = abi_mangle::mangle(abi,target);
    if (meta.binding == SBM_INTERNAL && linkage.merge) object += "."+std::to_string(output.index);
    meta.object = p.intern(object);
    Function f; f.symbol = output;
    FunctionId owner(p.functions.size()+1); f.signature = signature(sem.call_type(e),owner);
    p.functions.push_back(f);
    auto& s = p.symbols[output.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
    adjustor_thunks.push_back({e,slot.this_adjustment,slot.result_adjustment,slot.result_virtual_row,output,deleting});
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
        auto result = emit_raw(Instruction(Opcode::Call,sig.result),args.data(),args.size());
        if (thunk.result_adjustment || thunk.result_virtual_row) {
            auto project = [&](Value value) {
                if (thunk.result_virtual_row) {
                    auto table = emit(Opcode::Load,IRType::Ptr,{value.operand});
                    auto row = Operand::integer(thunk.result_virtual_row); row.negative_integer = true;
                    auto location = emit(Opcode::Index,IRType::I8,{table.operand,row});
                    auto offset = emit(Opcode::Load,IRType::I64,{location.operand});
                    value = emit(Opcode::Index,IRType::I8,{value.operand,offset.operand});
                }
                if (thunk.result_adjustment)
                    value = emit(Opcode::Index,IRType::I8,{value.operand,Operand::integer(thunk.result_adjustment)});
                return value;
            };
            if (reference(returned)) result = project(result);
            else {
                auto storage = builder->add_slot(0,IRType::Ptr);
                emit(Opcode::Store,IRType::Ptr,{Operand::integer(0),Operand::slot(storage)});
                auto test = emit(Opcode::Compare,IRType::Ptr,{result.operand,Operand::integer(0)},Operation::Eq);
                auto adjust = block(), end = block();
                emit(Opcode::Branch,IRType(),{test.operand,Operand::label(end),Operand::label(adjust)});
                start(adjust); result = project(result);
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
