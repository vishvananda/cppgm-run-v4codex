#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
namespace {
void combine_lifecycle(lowir_model::Program& p, Linkage& links,
    const std::vector<FunctionId>& units, bool finalization)
{
    using namespace lowir_model;
    if (units.size() < 2) return;
    // TU semantic graphs have already been released. These typed function IDs
    // are the complete cross-unit scheduling facts: no semantic lookup/replay.
    const std::string preferred = finalization ? "@__cppgm_finish" : "@__cppgm_start";
    auto name = p.intern(preferred);
    while (p.symbol_names.find(name) || links.native_names.get(p.intern(p.name(name).substr(1))))
        name = p.intern(preferred+"__"+std::to_string(++links.disambiguator));
    links.native_names.put(p.intern(p.name(name).substr(1)),1);
    Function f; f.symbol = p.symbol(name);
    Signature signature; signature.result = IRType::Void;
    p.signatures.push_back(signature); f.signature = SignatureId(p.signatures.size());
    FunctionId id(p.functions.size()+1); p.functions.push_back(f);
    auto& symbol = p.symbols[f.symbol.index-1];
    symbol.kind = Symbol::FunctionSymbol; symbol.entity = id.index;
    symbol.metadata.binding = ir_model::SBM_INTERNAL;
    symbol.metadata.role = finalization ? ir_model::SR_FINI : ir_model::SR_INIT;
    FunctionBuilder builder(p,id); builder.start_block(Name(0));
    for (std::size_t i = 0; i < units.size(); ++i) {
        auto unit = units[finalization ? units.size()-1-i : i];
        auto callee = p.functions[unit.index-1].symbol;
        p.symbols[callee.index-1].metadata.role = ir_model::SR_NONE;
        builder.append(Instruction(Opcode::Call,IRType::Void),{Operand::symbol(callee)});
    }
    builder.append(Instruction(Opcode::Return),{});
    if (!p.function_order.empty()) p.function_order.push_back(id);
}
}
void Linkage::finish_lifecycle(lowir_model::Program& program)
{
    // Source order is a deterministic permitted order between TUs. Each TU
    // already records its ordered initialization and reverse destruction.
    combine_lifecycle(program,*this,initializers,false);
    combine_lifecycle(program,*this,finalizers,true);
}
} }
