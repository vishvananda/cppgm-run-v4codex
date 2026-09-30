#include "native/encoding.h"
namespace native {
// Object emission roots and edges are typed LowIR facts. A function body is
// visited at most once; an unreferenced local body never reaches selection.
std::vector<bool> object_demand(const lowir_model::Program& p)
{
    using namespace lowir_model;
    std::vector<bool> live(p.symbols.size()+1);
    std::vector<unsigned> work;
    auto demand = [&](unsigned id) {
        if (id && !live[id]) { live[id] = true; work.push_back(id); }
    };
    for (const auto& g : p.globals) if (!g.declaration) demand(g.symbol.index);
    for (const auto& f : p.functions) if (!f.declaration) {
        const auto& m = p.symbols[f.symbol.index-1].metadata;
        if (m.object_root || m.role != SR_NONE ||
            (m.binding != SBM_INTERNAL && !(m.binding == SBM_WEAK && m.inline_hint))) demand(f.symbol.index);
    }
    for (unsigned next = 0; next < work.size(); ++next) {
        const auto& s = p.symbols[work[next]-1];
        if (s.metadata.tls_for) demand(s.metadata.tls_for.index);
        if (s.kind == Symbol::GlobalSymbol) {
            const auto& g = p.globals[s.entity-1];
            for (unsigned n = g.data.begin; n < g.data.end(); ++n)
                if (p.data[n].kind == DataItem::Address) demand(p.data[n].symbol.index);
        } else if (s.kind == Symbol::FunctionSymbol) {
            const auto& f = p.functions[s.entity-1];
            for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
                const auto& block = p.blocks[p.block_order[b].index-1];
                for (unsigned n = block.instructions.begin; n < block.instructions.end(); ++n) {
                    const auto& i = p.instructions[n];
                    for (unsigned k = i.operands.begin; k < i.operands.end(); ++k)
                        if (p.operands[k].kind == lowir_model::Operand::Symbol) demand(p.operands[k].ref);
                }
            }
        }
    }
    return live;
}
} // namespace native
