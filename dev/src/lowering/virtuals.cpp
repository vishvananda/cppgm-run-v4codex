#include "lowering/procedural.h"
#include <stdexcept>
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
    abi_mangle::Target target; target.kind = kind; target.type = abi_type(sem.entities[cls].type);
    bool internal = internal_scope(sem.entities[cls].owner);
    auto key = (std::uint64_t(16+unsigned(kind)) << 32) | target.type;
    if (!internal) if (auto prior = linkage.external.get(key)) return SymbolId(prior);
    SymbolId sym = fresh_symbol(kind == abi_mangle::TargetKind::Vtable ? "@vtable" : kind == abi_mangle::TargetKind::Typeinfo ? "@typeinfo" : "@typeinfo_name");
    auto& meta = p.symbols[sym.index-1].metadata; meta.binding = internal ? SBM_INTERNAL : SBM_WEAK;
    std::string object = abi_mangle::mangle(abi,target);
    if (internal && linkage.merge) object += "." + std::to_string(sym.index);
    meta.object = p.intern(object);
    if (!internal) linkage.external.put(key,sym.index);
    return sym;
}
SymbolId Procedural::typeinfo(EntityId cls)
{
    if (typeinfos[cls]) return typeinfos[cls];
    auto base = sem.direct_base(cls);
    SymbolId base_info = base ? typeinfo(base) : SymbolId();
    SymbolId info = typeinfos[cls] = abi_global(cls,abi_mangle::TargetKind::Typeinfo);
    if (p.symbols[info.index-1].kind != Symbol::Unknown) return info;
    SymbolId name = abi_global(cls,abi_mangle::TargetKind::TypeinfoName);
    abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Type; target.type = abi_type(sem.entities[cls].type);
    std::string encoded = abi_mangle::mangle(abi,target);
    std::vector<DataItem> data;
    for (unsigned char c : encoded) data.push_back(scalar(IRType::I8,c));
    data.push_back(scalar(IRType::I8,0)); publish(p,name,data);
    unsigned role = !base ? 0 : sem.base_offset(sem.entities[cls].type) ? 2 : 1;
    if (!linkage.rtti_roles[role]) {
        const char* names[] = {"_ZTVN10__cxxabiv117__class_type_infoE", "_ZTVN10__cxxabiv120__si_class_type_infoE", "_ZTVN10__cxxabiv121__vmi_class_type_infoE"};
        SymbolRole roles[] = {SR_RTTI_CLASS,SR_RTTI_SI,SR_RTTI_VMI};
        auto sym = fresh_symbol("@rtti_runtime"); linkage.rtti_roles[role] = sym;
        Global g; g.symbol = sym; g.declaration = true; p.globals.push_back(g);
        auto& s = p.symbols[sym.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
        s.metadata.binding = SBM_STRONG; s.metadata.role = roles[role]; s.metadata.object = p.intern(names[role]);
    }
    data = {relocation(linkage.rtti_roles[role],16),relocation(name)};
    if (role == 1) data.push_back(relocation(base_info));
    if (role == 2) { data.push_back(scalar(IRType::I32,0)); data.push_back(scalar(IRType::I32,1)); data.push_back(relocation(base_info)); data.push_back(scalar(IRType::I64,(sem.base_offset(sem.entities[cls].type)<<8)|2)); }
    publish(p,info,data); return info;
}
void Procedural::emit_vtables()
{
    for (EntityId cls = 1; cls < sem.entities.size(); ++cls) {
        if (!sem.polymorphic(cls) || !sem.virtual_class(cls).demanded) continue;
        SymbolId table = vtables[cls] = abi_global(cls,abi_mangle::TargetKind::Vtable);
        if (p.symbols[table.index-1].kind != Symbol::Unknown) continue;
        std::vector<DataItem> data = {scalar(IRType::I64,0),relocation(typeinfo(cls))};
        auto slots = sem.virtual_class(cls).slots;
        for (unsigned j = 0; j < slots.size(); ++j) {
            EntityId e = slots[j]; auto m = sem.member_fact(e);
            if (m.pure) {
                if (!pure_virtual) {
                    pure_virtual = fresh_symbol("@pure_virtual");
                    Function f; f.symbol = pure_virtual; f.declaration = true;
                    f.signature = signature(sem.call_type(e)); p.functions.push_back(f);
                    auto& s = p.symbols[pure_virtual.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = p.functions.size();
                    s.metadata.binding = SBM_STRONG; s.metadata.object = p.intern("__cxa_pure_virtual");
                }
                data.push_back(relocation(pure_virtual));
            } else data.push_back(relocation(symbol(e,false,m.destructor && j && slots[j-1] == e)));
        }
        publish(p,table,data);
    }
}
void Procedural::vpointer_store(EntityId cls)
{
    if (!sem.polymorphic(cls)) return;
    if (!vtables[cls]) throw std::logic_error("undemanded vtable in lifecycle entry");
    Value object = emit(Opcode::Load,IRType::Ptr,{Operand::slot(this_slot)});
    Value table = emit(Opcode::Addr,IRType(),{Operand::symbol(vtables[cls])});
    Value point = emit(Opcode::Index,IRType::I8,{table.operand,Operand::integer(16)});
    emit(Opcode::Store,IRType::Ptr,{point.operand,object.operand});
}
Value Procedural::virtual_function(Value object, unsigned slot)
{
    auto table = emit(Opcode::Load,IRType::Ptr,{object.operand});
    if (slot > 1) table = emit(Opcode::Index,IRType::I8,{table.operand,Operand::integer((slot-1)*8)});
    return emit(Opcode::Load,IRType::Ptr,{table.operand});
}
Value Procedural::pointer_projection(Value base, unsigned adjustment)
{
    if (adjustment <= 1) return base_projection(base,adjustment);
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
    if (deleting_symbols[e]) return deleting_symbols[e];
    auto sym = fresh_symbol("@deleting_destructor"); deleting_symbols[e] = sym;
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
    for (EntityId e = 1; e < sem.entities.size(); ++e) {
        if (!deleting_symbols[e]) continue;
        function = FunctionId(p.symbols[deleting_symbols[e].index-1].entity);
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
    if (virtual_signatures[e]) return virtual_signatures[e];
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
    p.signatures.push_back(sig); return virtual_signatures[e] = SignatureId(p.signatures.size());
}
} }
