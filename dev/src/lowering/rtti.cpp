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
    for (const auto& item : data) p.data.push_back(item);
    p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
}
}
bool Procedural::internal_rtti_type(TypeId id)
{
    if (!id) return false;
    if (auto old = rtti_internal_types.get(id)) return old == 2;
    auto t = sem.types[id];
    bool internal = internal_rtti_type(t.child);
    if (t.kind == TypeKind::Named) {
        internal |= internal_scope(sem.entities[t.entity].owner);
        auto args = sem.specialization_arguments(t.entity);
        for (unsigned i = 0; i < args.count; ++i) {
            auto arg = sem.template_argument(args.offset+i);
            internal |= semantic::value_argument(arg) ? local_abi_argument(arg) : internal_rtti_type(arg);
        }
    } else if (t.kind == TypeKind::ArgumentPack) {
        auto args = sem.pack_arguments(id);
        for (unsigned i = 0; i < args.count; ++i) {
            auto arg = sem.template_argument(args.offset+i);
            internal |= semantic::value_argument(arg) ? local_abi_argument(arg) : internal_rtti_type(arg);
        }
    } else if (t.kind == TypeKind::MemberPointer) internal |= internal_rtti_type(sem.entities[t.entity].type);
    else if (t.kind == TypeKind::Function)
        for (unsigned i = 0; i < t.count; ++i) internal |= internal_rtti_type(sem.types.parameters[t.offset+i]);
    rtti_internal_types.put(id,internal ? 2 : 1); return internal;
}
unsigned Procedural::rtti_incomplete_flags(TypeId id)
{
    if (auto old = rtti_incomplete_types.get(id)) return old-1;
    auto t = sem.types[id];
    unsigned flags = t.kind == TypeKind::Pointer || t.kind == TypeKind::MemberPointer ? rtti_incomplete_flags(t.child) :
        sem.class_value(id) && !sem.entities[t.entity].complete ? 8 : 0;
    if (t.kind == TypeKind::MemberPointer && !sem.entities[t.entity].complete) flags |= 16;
    rtti_incomplete_types.put(id,flags+1); return flags;
}
SymbolId Procedural::rtti_runtime(unsigned role)
{
    if (linkage.rtti_roles[role]) return linkage.rtti_roles[role];
    static const char* names[] = {
        "_ZTVN10__cxxabiv117__class_type_infoE", "_ZTVN10__cxxabiv120__si_class_type_infoE", "_ZTVN10__cxxabiv121__vmi_class_type_infoE",
        "_ZTVN10__cxxabiv123__fundamental_type_infoE", "_ZTVN10__cxxabiv119__pointer_type_infoE",
        "_ZTVN10__cxxabiv117__array_type_infoE", "_ZTVN10__cxxabiv120__function_type_infoE", "_ZTVN10__cxxabiv116__enum_type_infoE",
        "_ZTVN10__cxxabiv129__pointer_to_member_type_infoE"};
    static const SymbolRole roles[] = {SR_RTTI_CLASS,SR_RTTI_SI,SR_RTTI_VMI,SR_RTTI_DATA,SR_RTTI_DATA,SR_RTTI_DATA,SR_RTTI_DATA,SR_RTTI_DATA,SR_RTTI_DATA};
    auto symbol = fresh_symbol("@rtti_runtime"); linkage.rtti_roles[role] = symbol;
    Global g; g.symbol = symbol; g.declaration = true; p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
    s.metadata.binding = SBM_STRONG; s.metadata.role = roles[role]; s.metadata.object = p.intern(names[role]);
    return symbol;
}
SymbolId Procedural::rtti_type(TypeId id)
{
    id = sem.types.unqualified(id);
    if (auto old = rtti_symbols.get(id)) { ++rtti_hits; return SymbolId(old); }
    ++rtti_work;
    auto t = sem.types[id];
    auto info = abi_type_global(id,abi_mangle::TargetKind::Typeinfo);
    rtti_symbols.put(id,info.index);
    if (p.symbols[info.index-1].kind != Symbol::Unknown) return info;
    if (t.kind == TypeKind::Fundamental && t.fundamental == FT_VOID) {
        Global g; g.symbol = info; g.declaration = true; p.globals.push_back(g);
        auto& symbol = p.symbols[info.index-1]; symbol.kind = Symbol::GlobalSymbol; symbol.entity = p.globals.size();
        symbol.metadata.binding = SBM_STRONG; symbol.metadata.role = SR_RTTI_DATA; return info;
    }
    unsigned role = 3, flags = 0;
    SymbolId dependency, owner;
    std::uint64_t offset = 0;
    if (t.kind == TypeKind::Named) {
        role = sem.entities[t.entity].class_info ? 0 : 7;
        if (auto b = sem.first_base_edge(t.entity)) {
            sem.object_size(id);
            auto edge = sem.base_edge(b);
            dependency = typeinfo(edge.base); offset = edge.offset;
            flags = edge.access == semantic::Access::Public ? 2 : 0;
            role = edge.next || edge.virtual_base || offset || !flags ? 2 : 1;
        }
    } else if (t.kind == TypeKind::Pointer || t.kind == TypeKind::MemberPointer) {
        role = t.kind == TypeKind::Pointer ? 4 : 8;
        auto qualified = t.child;
        while (sem.types[qualified].kind == TypeKind::Array) qualified = sem.types[qualified].child;
        flags = sem.types[qualified].kind == TypeKind::Function ? 0 : sem.types[qualified].cv;
        flags |= rtti_incomplete_flags(id);
        dependency = rtti_type(t.child);
        if (role == 8) {
            if (!sem.entities[t.entity].complete) flags |= 16;
            owner = rtti_type(sem.entities[t.entity].type);
        }
    } else if (t.kind == TypeKind::Array) role = 5;
    else if (t.kind == TypeKind::Function) role = 6;
    else if (t.kind != TypeKind::Fundamental) throw std::runtime_error("unsupported RTTI type category");
    auto name = abi_type_global(id,abi_mangle::TargetKind::TypeinfoName);
    std::vector<DataItem> data;
    if (p.symbols[name.index-1].kind == Symbol::Unknown) {
        abi_mangle::Target target; target.kind = abi_mangle::TargetKind::Type; target.type = abi_type(id);
        auto encoded = abi_mangle::mangle(abi,target);
        for (unsigned char c : encoded) data.push_back(scalar(IRType::I8,c));
        data.push_back(scalar(IRType::I8,0)); publish(p,name,data);
    }
    data = {relocation(rtti_runtime(role),16),relocation(name)};
    if (role == 1) data.push_back(relocation(dependency));
    if (role == 2) {
        unsigned count = 0;
        for (auto b = sem.first_base_edge(t.entity); b; b = sem.base_edge(b).next) ++count;
        data.push_back(scalar(IRType::I32,sem.rtti_class_flags(t.entity))); data.push_back(scalar(IRType::I32,count));
        for (auto b = sem.first_base_edge(t.entity); b; b = sem.base_edge(b).next) {
            auto edge = sem.base_edge(b);
            data.push_back(relocation(typeinfo(edge.base)));
            data.push_back(scalar(IRType::I64,(edge.offset<<8)|(edge.access == semantic::Access::Public ? 2 : 0)));
        }
    }
    if (role == 4 || role == 8) { data.push_back(scalar(IRType::I32,flags)); data.push_back(relocation(dependency)); }
    if (role == 8) data.push_back(relocation(owner));
    publish(p,info,data); return info;
}
SymbolId Procedural::rtti_function(unsigned role)
{
    if (linkage.rtti_functions[role]) return linkage.rtti_functions[role];
    static const char* names[] = {"__dynamic_cast","__cxa_bad_typeid","__cxa_bad_cast"};
    static const SymbolRole roles[] = {SR_DYNAMIC_CAST,SR_BAD_TYPEID,SR_BAD_CAST};
    auto ptr = sem.types.compound(TypeKind::Pointer,sem.types.fundamental(FT_VOID));
    auto sig = role ? sem.types.function(sem.types.fundamental(FT_VOID),{},false) :
        sem.types.function(ptr,{ptr,ptr,ptr,sem.types.fundamental(FT_LONG_INT)},false);
    Function f; f.symbol = fresh_symbol("@rtti_function"); f.declaration = true;
    FunctionId owner(p.functions.size()+1); f.signature = signature(sig,owner);
    if (role) p.signatures[f.signature.index-1].boundary.returns = ir_model::CRM_NORETURN;
    p.functions.push_back(f); linkage.rtti_functions[role] = f.symbol;
    auto& s = p.symbols[f.symbol.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
    s.metadata.binding = SBM_STRONG; s.metadata.linkage = LLM_C;
    s.metadata.role = roles[role]; s.metadata.object = p.intern(names[role]);
    return f.symbol;
}
void Procedural::rtti_failure(unsigned role)
{
    emit(Opcode::Call,IRType::Void,{Operand::symbol(rtti_function(role))});
    if (result_type() == IRType::Void) emit(Opcode::Return,IRType(),{});
    else emit(Opcode::Return,result_type(),{result_type().floating() ? Operand::floating(0) : Operand::integer(0)});
}
Value Procedural::rtti_expression(NodeId n)
{
    auto use = sem.rtti_expression(n); auto fact = sem.expression_fact(n);
    if (fact.form == semantic::ExpressionForm::Typeid) {
        Value value;
        if (!use.dynamic) value = emit(Opcode::Addr,IRType(),{Operand::symbol(rtti_type(use.type))});
        else {
            auto object = address(expression(ast[n].first,true));
            auto null = emit(Opcode::Compare,IRType::Ptr,{object.operand,Operand::integer(0)},Operation::Eq);
            auto fail = block(), scan = block();
            emit(Opcode::Branch,IRType(),{null.operand,Operand::label(fail),Operand::label(scan)});
            start(fail); rtti_failure(1); start(scan);
            auto table = emit(Opcode::Load,IRType::Ptr,{object.operand});
            auto offset = Operand::integer(-8); offset.negative_integer = true;
            auto slot = emit(Opcode::Index,IRType::I8,{table.operand,offset});
            value = emit(Opcode::Load,IRType::Ptr,{slot.operand});
        }
        value.type = fact.type; value.address = true; return value;
    }
    auto operand = ast[ast[n].first].next;
    auto object = use.reference ? address(expression(operand,true)) : load(expression(operand));
    // A -2 hint excludes a public source-to-target base path, not a
    // crosscast through a more-derived complete object. Let RTTI decide.
    bool to_void = sem.types[use.type].kind == TypeKind::Fundamental && sem.types[use.type].fundamental == FT_VOID;
    auto slot = builder->add_slot(0,IRType::Ptr);
    emit(Opcode::Store,IRType::Ptr,{Operand::integer(0),Operand::slot(slot)});
    auto null = emit(Opcode::Compare,IRType::Ptr,{object.operand,Operand::integer(0)},Operation::Eq);
    auto scan = block(), end = block();
    emit(Opcode::Branch,IRType(),{null.operand,Operand::label(end),Operand::label(scan)});
    start(scan);
    if (to_void) {
        auto table = emit(Opcode::Load,IRType::Ptr,{object.operand});
        auto offset = Operand::integer(-16); offset.negative_integer = true;
        auto top = emit(Opcode::Index,IRType::I8,{table.operand,offset});
        auto displacement = emit(Opcode::Load,IRType::I64,{top.operand});
        auto complete = emit(Opcode::Index,IRType::I8,{object.operand,displacement.operand});
        emit(Opcode::Store,IRType::Ptr,{complete.operand,Operand::slot(slot)}); jump(end);
        // Retain the O0 cast continuation in the explicit course LowIR view.
        start(block());
    }
    auto from = emit(Opcode::Addr,IRType(),{Operand::symbol(rtti_type(use.source))});
    auto to = emit(Opcode::Addr,IRType(),{Operand::symbol(rtti_type(use.type))});
    auto hint = Operand::integer(use.hint); hint.negative_integer = use.hint < 0;
    auto value = emit(Opcode::Call,IRType::Ptr,{Operand::symbol(rtti_function(0)),object.operand,from.operand,to.operand,hint});
    emit(Opcode::Store,IRType::Ptr,{value.operand,Operand::slot(slot)});
    if (use.reference) {
        auto failed = emit(Opcode::Compare,IRType::Ptr,{value.operand,Operand::integer(0)},Operation::Eq);
        auto fail = block(), found = block();
        emit(Opcode::Branch,IRType(),{failed.operand,Operand::label(fail),Operand::label(found)});
        start(fail); rtti_failure(2); start(found); jump(end);
        start(block());
    }
    jump(end); start(end);
    value = emit(Opcode::Load,IRType::Ptr,{Operand::slot(slot)});
    value.type = fact.type; value.address = use.reference; return value;
}
} }
