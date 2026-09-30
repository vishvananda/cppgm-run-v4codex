#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
Value Procedural::emit(Instruction instruction, const Operand* args, std::size_t count)
{
    if (instruction.opcode != Opcode::Call) return emit_raw(instruction,args,count);
    auto signature = instruction.signature;
    if (count && args[0].kind == Operand::Symbol) {
        auto symbol = p.symbols[args[0].ref-1];
        if (symbol.kind == lowir_model::Symbol::FunctionSymbol)
            signature = p.functions[symbol.entity-1].signature;
    }
    auto id = linkage.signature_parameter_abis.get(signature.index);
    if (!id) return emit_raw(instruction,args,count);
    auto plan = linkage.parameter_abis[id];
    if (count < plan.visible+1) throw std::logic_error("missing visible value arguments");
    // The ABI plan belongs to the program signature, including declarations
    // published in an earlier TU. Every source call path consumes this same
    // layout; reference/pointer parameters never produce hidden arguments.
    std::vector<Operand> expanded(args,args+plan.visible+1);
    expanded.reserve(count+plan.hidden.count);
    for (unsigned j = 0; j < plan.hidden.count; ++j) {
        auto fact = linkage.value_base_arguments[plan.hidden.begin+j];
        auto object = expanded[fact.parameter+1];
        if (object.kind == Operand::Slot || object.kind == Operand::Symbol)
            object = emit(Opcode::Addr,IRType(),{object}).operand;
        expanded.push_back(emit(Opcode::Index,IRType::I8,{object,Operand::integer(fact.offset)}).operand);
    }
    expanded.insert(expanded.end(),args+plan.visible+1,args+count);
    return emit_raw(instruction,expanded.data(),expanded.size());
}
} }
