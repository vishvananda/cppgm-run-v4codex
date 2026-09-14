#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::initialize_constant_array(EntityId e, Value location)
{
    auto target = sem.entities[e].type;
    SymbolId source(constant_arrays.get(e));
    if (!source.index) {
        lowir_model::Global g; g.structured = true;
        g.symbol = fresh_symbol("@__constant_array_"+std::to_string(e));
        g.data.begin = p.data.size(); global_plan(sem.constant_array_plan(e));
        g.data.count = p.data.size()-g.data.begin; p.globals.push_back(g);
        auto& symbol = p.symbols[g.symbol.index-1];
        symbol.kind = lowir_model::Symbol::GlobalSymbol; symbol.entity = p.globals.size();
        symbol.metadata.binding = ir_model::SBM_INTERNAL;
        symbol.metadata.storage = ir_model::GSM_READONLY;
        source = g.symbol; constant_arrays.put(e,source.index);
    }
    Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(target);
    copy.alignment = sem.object_alignment(target);
    emit(copy,{Operand::symbol(source),address(location).operand});
}
} }
