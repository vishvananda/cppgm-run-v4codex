#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::finish_exception_boundary()
{
    if (!sem.function_nonthrowing(active_function)) return;
    auto blocks = p.functions[function.index-1].blocks;
    // O0 retains the source deallocation boundary even when the selected
    // allocation runtime refines the free call to unwind=no.
    bool throwing = deallocation_boundary;
    auto first = p.blocks[p.block_order[blocks.begin].index-1].instructions.begin;
    for (auto n = first; n < p.instructions.size(); ++n) {
        auto i = p.instructions[n];
        if (i.opcode != Opcode::Call) continue;
        auto signature = i.signature;
        auto callee = p.operands[i.operands.begin];
        if (callee.kind == Operand::Symbol) {
            auto symbol = p.symbols[callee.ref-1];
            signature = p.functions[symbol.entity-1].signature;
        }
        if (!signature || p.signatures[signature.index-1].boundary.unwind != CUM_NO) throwing = true;
    }
    if (!throwing) return;
    // Calls already carry the selected declaration's boundary facts. Inspect
    // those once, after all implicit cleanup/subobject calls have been emitted;
    // never repeat overload resolution or guess effects from source spellings.
    // Only this function's contiguous instruction slice is rebuilt. The scratch
    // vector dies here; work/storage are linear in its emitted instructions.
    auto handler = block();
    auto storage = builder->add_slot(0,IRType::Ptr);
    std::vector<Instruction> old(p.instructions.begin()+first,p.instructions.end());
    p.instructions.resize(first);
    for (unsigned b = 0; b < blocks.count; ++b) {
        auto& block = p.blocks[p.block_order[blocks.begin+b].index-1];
        auto range = block.instructions; block.instructions.begin = p.instructions.size();
        if (!b) {
            Instruction enter(Opcode::EhTry); enter.operands.begin = p.operands.size(); enter.operands.count = 1;
            p.operands.push_back(Operand::label(handler)); p.instructions.push_back(enter);
        }
        for (auto n = range.begin; n < range.end(); ++n) {
            auto i = old[n-first];
            if (i.opcode == Opcode::Return) p.instructions.push_back(Instruction(Opcode::EhEnd));
            if (i.destination && p.values[i.destination.index-1].definition == n+1)
                p.values[i.destination.index-1].definition = p.instructions.size()+1;
            p.instructions.push_back(i);
        }
        block.instructions.count = p.instructions.size()-block.instructions.begin;
    }
    start(handler); emit(Opcode::EhCatchAll,IRType(),{Operand::integer(1)});
    auto exception = emit(Opcode::Exception,IRType::Ptr,{});
    emit(Opcode::Store,IRType::Ptr,{exception.operand,Operand::slot(storage)});
    auto invoke = block(); jump(invoke); start(invoke);
    exception = emit(Opcode::Load,IRType::Ptr,{Operand::slot(storage)});
    if (!linkage.terminate_adapter) {
        Function f; f.symbol = fresh_symbol("@__terminate_exception");
        auto ptr = sem.types.compound(TypeKind::Pointer,sem.types.fundamental(FT_VOID));
        auto sig = sem.types.function(sem.types.fundamental(FT_VOID),{ptr},false);
        FunctionId owner(p.functions.size()+1); f.signature = signature(sig,owner);
        p.signatures[f.signature.index-1].boundary.unwind = CUM_NO;
        p.signatures[f.signature.index-1].boundary.returns = CRM_NORETURN;
        p.functions.push_back(f); linkage.terminate_adapter = f.symbol;
        auto& s = p.symbols[f.symbol.index-1]; s.kind = Symbol::FunctionSymbol; s.entity = owner.index;
        s.metadata.binding = SBM_INTERNAL;
    }
    emit(Opcode::Call,IRType::Void,{Operand::symbol(linkage.terminate_adapter),exception.operand});
    exception_fallback();
}
void Procedural::emit_terminate_adapter()
{
    if (!linkage.terminate_adapter) return;
    function = FunctionId(p.symbols[linkage.terminate_adapter.index-1].entity);
    if (p.functions[function.index-1].blocks.count) return;
    reset_lifetime(0); builder.reset(new FunctionBuilder(p,function)); start(block());
    auto signature = p.signatures[p.functions[function.index-1].signature.index-1];
    auto argument = p.parameters[signature.parameters.begin];
    begin_catch(Operand::value(argument.value));
    emit(Opcode::Call,IRType::Void,{Operand::symbol(exception_function(5))});
    emit(Opcode::Return,IRType(),{}); builder.reset();
}
} }
