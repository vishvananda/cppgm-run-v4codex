#include "lowering/procedural.h"
#include <stdexcept>
#include <algorithm>
namespace cppgm { namespace lowering {
using namespace lowir_model;
namespace {
DataItem scalar(IRType t, std::uint64_t value) { DataItem d; d.kind = DataItem::Scalar; d.type = t; d.value = Operand::integer(value); return d; }
DataItem relocation(SymbolId symbol, std::int64_t addend = 0) { DataItem d; d.kind = DataItem::Address; d.type = IRType::Ptr; d.symbol = symbol; d.addend = addend; return d; }
void publish(Program& p, SymbolId symbol, const std::vector<DataItem>& data)
{
    Global g; g.symbol = symbol; g.structured = true; g.data.begin = p.data.size(); g.data.count = data.size();
    for (const auto& item : data) p.data.push_back(item); p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
}
}
SymbolId Procedural::abi_global(EntityId cls, abi_mangle::TargetKind kind)
{
    return abi_type_global(sem.entities[cls].type,kind);
}
SymbolId Procedural::abi_type_global(TypeId type, abi_mangle::TargetKind kind)
{
    abi_mangle::Target target; target.kind = kind; target.type = abi_type(type);
    bool internal = internal_rtti_type(type);
    // Incomplete placeholders must not preempt another translation unit's
    // complete RTTI. Their name objects still share the canonical ABI identity.
    internal |= kind == abi_mangle::TargetKind::Typeinfo && rtti_incomplete_flags(type);
    auto key = (std::uint64_t(16+unsigned(kind)) << 32) | target.type;
    if (!internal) if (auto prior = linkage.external.get(key)) return SymbolId(prior);
    std::string name = kind == abi_mangle::TargetKind::Vtable ? "@vtable" : kind == abi_mangle::TargetKind::Typeinfo ? "@typeinfo" : "@typeinfo_name";
    if (sem.types[type].kind == TypeKind::Fundamental)
        name = (kind == abi_mangle::TargetKind::Typeinfo ? "@__rtti_" : "@__typeinfo_name__")+support_type_name(type);
    SymbolId sym = fresh_symbol(name);
    auto& meta = p.symbols[sym.index-1].metadata; meta.binding = internal ? SBM_INTERNAL : SBM_WEAK;
    std::string object = abi_mangle::mangle(abi,target);
    if (internal && linkage.merge) object += "." + std::to_string(sym.index);
    meta.object = p.intern(object);
    if (!internal) linkage.external.put(key,sym.index);
    return sym;
}
SymbolId Procedural::typeinfo(EntityId cls)
{
    return rtti_type(sem.entities[cls].type);
}
SymbolId Procedural::vtable_symbol(EntityId cls)
{
    if (sem.virtual_class(cls).demand != semantic::FactState::Success)
        throw std::logic_error("unpublished vtable demand");
    auto id = sem.virtual_class_id(cls);
    if (!vtables[id]) vtables[id] = abi_global(cls,abi_mangle::TargetKind::Vtable);
    return vtables[id];
}
void Procedural::emit_vtables()
{
    for (EntityId cls : sem.demanded_vtables()) {
        SymbolId table = vtable_symbol(cls);
        if (p.symbols[table.index-1].kind != Symbol::Unknown) continue;
        auto emit_view = [&](SymbolId output, const std::vector<semantic::VirtualSlot>& slots, std::uint64_t offset) {
            std::vector<DataItem> data = {scalar(IRType::I64,0-offset),relocation(typeinfo(cls))};
            data[0].value.negative_integer = offset != 0;
            for (unsigned j = 0; j < slots.size(); ++j) {
                auto m = sem.member_fact(slots[j].function);
                bool deleting = m.destructor && j && slots[j-1].function == slots[j].function;
                data.push_back(relocation(virtual_target(slots[j],deleting)));
            }
            publish(p,output,data);
        };
        emit_view(table,sem.virtual_class(cls).slots,0);
        const auto& views = sem.virtual_class(cls).views;
        for (unsigned j = 0; j < views.size(); ++j)
            if (views[j].offset) emit_view(view_symbol(cls,j+1),views[j].slots,views[j].offset);
    }
}
void Procedural::vpointer_store(EntityId cls)
{
    if (!sem.polymorphic(cls)) return;
    auto symbol = vtables[sem.virtual_class_id(cls)];
    if (!symbol) throw std::logic_error("undemanded vtable in lifecycle entry");
    auto store = [&](SymbolId symbol, std::uint64_t offset) {
        Value object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
        if (offset) object = emit(Opcode::Index,IRType::I8,{object.operand,Operand::integer(offset)});
        Value table = emit(Opcode::Addr,IRType(),{Operand::symbol(symbol)});
        Value point = emit(Opcode::Index,IRType::I8,{table.operand,Operand::integer(16)});
        emit(Opcode::Store,IRType::Ptr,{point.operand,object.operand});
    };
    store(symbol,0);
    const auto& views = sem.virtual_class(cls).views;
    for (unsigned j = 0; j < views.size(); ++j)
        if (views[j].store) store(view_symbol(cls,j+1),views[j].offset);
}
Value Procedural::virtual_function(Value object, unsigned slot)
{
    auto table = emit(Opcode::Load,IRType::Ptr,{object.operand});
    if (slot > 1) table = emit(Opcode::Index,IRType::I8,{table.operand,Operand::integer((slot-1)*8)});
    return emit(Opcode::Load,IRType::Ptr,{table.operand});
}
Value Procedural::pointer_projection(Value base, unsigned adjustment)
{
    if (base.nonnull || !sem.base_adjustments[adjustment].total) return base_projection(base,adjustment);
    auto slot = builder->add_slot(0,IRType::Ptr);
    auto test = emit(Opcode::Compare,IRType::Ptr,{base.operand,Operand::integer(0)},Operation::Eq);
    auto null = block(), adjust = block(), end = block();
    emit(Opcode::Branch,IRType(),{test.operand,Operand::label(null),Operand::label(adjust)});
    start(null); emit(Opcode::Store,IRType::Ptr,{Operand::integer(0),Operand::slot(slot)}); jump(end);
    start(adjust); auto projected = base_projection(base,adjustment); emit(Opcode::Store,IRType::Ptr,{projected.operand,Operand::slot(slot)}); jump(end);
    start(end); return emit(Opcode::Load,IRType::Ptr,{Operand::slot(slot)});
}
SymbolId Procedural::deleting_symbol(EntityId e)
{
    auto id = sem.entities[e].member_info;
    if (deleting_symbols[id]) return deleting_symbols[id];
    auto sym = fresh_symbol("@deleting_destructor"); deleting_symbols[id] = sym;
    deleting_entries.push_back(e);
    abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Function;
    target.function.name = abi.name(abi_scope(sem.entities[e].owner),spelling(sem.entities[e].name));
    target.function.category = abi_mangle::FunctionCategory::Member;
    target.function.terminal = abi_mangle::ABI_TERMINAL_DESTRUCTOR_DELETING;
    local_member_abi(e,target.function);
    bool internal = internal_scope(sem.entities[e].owner);
    auto& s = p.symbols[sym.index-1]; s.metadata.binding = internal ? SBM_INTERNAL : sem.entities[e].inline_function ? SBM_WEAK : SBM_STRONG;
    std::string object = abi_mangle::mangle(abi,target);
    if (internal && linkage.merge) object += "." + std::to_string(sym.index);
    s.metadata.object = p.intern(object);
    Function f; f.symbol = sym; f.declaration = !sem.entities[e].body && !sem.synthetic_member(e);
    f.signature = signature(sem.call_type(e),FunctionId(p.functions.size()+1));
    p.functions.push_back(f); s.kind = Symbol::FunctionSymbol; s.entity = p.functions.size();
    return sym;
}
void Procedural::emit_deleting_entries()
{
    std::sort(deleting_entries.begin(), deleting_entries.end());
    for (EntityId e : deleting_entries) {
        function = FunctionId(p.symbols[deleting_symbols[sem.entities[e].member_info].index-1].entity);
        if (p.functions[function.index-1].declaration) continue;
        builder.reset(new FunctionBuilder(p,function)); reset_lifetime(e); returned = sem.types.fundamental(FT_VOID);
        start(block()); this_slot = builder->add_slot(0,IRType::Ptr);
        auto sig = p.signatures[p.functions[function.index-1].signature.index-1];
        emit(Opcode::Store,IRType::Ptr,{Operand::value(p.parameters[sig.parameters.begin].value),Operand::slot(this_slot)});
        auto cleanup = block(), end = block(); emit(Opcode::EhCleanup,IRType(),{Operand::label(cleanup)});
        auto m = sem.member_fact(e); EntityId cls = sem.scopes[sem.entities[e].owner].entity;
        bool complete = m.deleting_complete;
        auto release = [&]() {
            auto object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
            Operand args[] = {Operand::symbol(symbol(m.deleting_deallocation)),object.operand,Operand::integer(sem.object_size(sem.entities[cls].type))};
            emit(Instruction(Opcode::Call,IRType::Void),args,sem.types[sem.entities[m.deleting_deallocation].type].count+1);
        };
        if (complete) {
            auto object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
            emit(Opcode::Call,IRType::Void,{Operand::symbol(symbol(e)),object.operand});
        } else vpointer_store(cls);
        emit(Opcode::EhEnd,IRType(),{});
        if (!complete) {
            for (unsigned j = 0; j < m.destruction_count; ++j) {
                auto action = sem.destruction_actions[m.destruction_begin+j];
                if (!sem.destructor_needed(action.destructor)) continue;
                auto suffix = block(), next = block(); emit(Opcode::EhCleanup,IRType(),{Operand::label(suffix)});
                destroy_subobject(action); emit(Opcode::EhEnd,IRType(),{}); jump(next);
                start(suffix); emitting_cleanup = true;
                for (unsigned k = j+1; k < m.destruction_count; ++k) {
                    auto remaining = sem.destruction_actions[m.destruction_begin+k];
                    if (sem.destructor_needed(remaining.destructor)) destroy_subobject(remaining);
                }
                release(); emit(Opcode::EhEnd,IRType(),{}); emit(Opcode::Resume,IRType(),{}); emitting_cleanup = false; start(next);
            }
        }
        release(); jump(end); start(cleanup); emitting_cleanup = true;
        if (!complete) destroy_subobjects(e);
        release(); emit(Opcode::EhEnd,IRType(),{}); emit(Opcode::Resume,IRType(),{});
        emitting_cleanup = false; start(end); emit(Opcode::Return,IRType(),{}); builder.reset();
    }
}
} }

namespace cppgm { namespace lowering {
SignatureId Procedural::virtual_signature(EntityId e)
{
    auto id = sem.entities[e].member_info;
    if (virtual_signatures[id]) return virtual_signatures[id];
    auto source = signature(sem.call_type(e));
    auto sig = p.signatures[source.index-1];
    // Refinements belong to this member-call signature, not the canonical
    // function-pointer signature's shared parameter slice.
    auto parameters = sig.parameters;
    sig.parameters.begin = p.parameters.size();
    for (unsigned j = 0; j < parameters.count; ++j) p.parameters.push_back(p.parameters[parameters.begin+j]);
    auto cls = sem.scopes[sem.entities[e].owner].entity;
    p.parameters[sig.parameters.begin + sem.indirect_value(sem.types[sem.entities[e].type].child)].object_bytes = sem.object_size(sem.entities[cls].type);
    if (sem.function_nonthrowing(e)) sig.boundary.unwind = ir_model::CUM_NO;
    p.signatures.push_back(sig); return virtual_signatures[id] = SignatureId(p.signatures.size());
}
} }
