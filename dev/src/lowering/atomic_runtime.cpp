#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
bool Procedural::inline_atomic(TypeId t)
{
    auto bytes = sem.object_size(t);
    return bytes && bytes <= 16 && !(bytes & (bytes-1)) && (bytes < 16 || sem.object_alignment(t) >= 16);
}
Operand Procedural::atomic_buffer(Value value)
{
    if (value.address) return address(value).operand;
    if (value.ir.kind() == IRType::Object) return value.operand;
    auto slot = Operand::slot(builder->add_slot(0,value.ir));
    emit(Opcode::Store,value.ir,{value.operand,slot}); return slot;
}
Value Procedural::atomic_runtime(AtomicOp op, std::uint64_t bytes, Operand object, Operand value, Operand result)
{
    // libatomic's generic ABI is used for representations without a safe
    // inline primitive. These are runtime library calls, never compiler work.
    using namespace lowir_model;
    unsigned index = op == AtomicOp::Load ? 0 : op == AtomicOp::Store ? 1 : op == AtomicOp::Exchange ? 2 : 3;
    if (!linkage.atomic_runtime_symbols[index]) {
        static const char* names[] = {"__atomic_load","__atomic_store","__atomic_exchange","__atomic_compare_exchange"};
        auto ptr = sem.types.compound(TypeKind::Pointer,sem.types.fundamental(FT_VOID));
        auto size = sem.types.fundamental(FT_UNSIGNED_LONG_INT), order = sem.types.fundamental(FT_INT);
        std::vector<TypeId> params{size,ptr,ptr};
        if (index >= 2) params.push_back(ptr);
        params.push_back(order);
        if (index == 3) params.push_back(order);
        auto sig = sem.types.function(sem.types.fundamental(index == 3 ? FT_BOOL : FT_VOID),params,false);
        Function f; f.symbol = fresh_symbol("@atomic_runtime"); f.declaration = true;
        FunctionId owner(p.functions.size()+1); f.signature = signature(sig,owner);
        p.signatures[f.signature.index-1].boundary.unwind = ir_model::CUM_NO;
        p.functions.push_back(f); linkage.atomic_runtime_symbols[index] = f.symbol;
        auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = Symbol::FunctionSymbol; symbol.entity = owner.index;
        symbol.metadata.binding = SBM_STRONG; symbol.metadata.linkage = LLM_C;
        symbol.metadata.object = p.intern(names[index]);
    }
    auto pointer = [&](Operand operand) {
        return operand.kind == Operand::Slot || operand.kind == Operand::Symbol ? emit(Opcode::Addr,IRType(),{operand}).operand : operand;
    };
    std::vector<Operand> args{Operand::symbol(linkage.atomic_runtime_symbols[index]),Operand::integer(bytes),pointer(object)};
    if (index == 3) { args.push_back(pointer(result)); args.push_back(pointer(value)); }
    else { args.push_back(pointer(value)); if (index == 2) args.push_back(pointer(result)); }
    args.push_back(Operand::integer(5)); if (index == 3) args.push_back(Operand::integer(5));
    return emit(Instruction(Opcode::Call,index == 3 ? IRType::U8 : IRType::Void),args);
}
} }
