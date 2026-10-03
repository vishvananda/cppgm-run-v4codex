#include "lowir/emission_demand.h"
namespace lowir_model {
std::vector<bool> emission_demand(const Program& p, const std::vector<EmissionDependency>& dependencies)
{
    std::vector<bool> live(p.symbols.size()+1);
    std::vector<unsigned> work;
    std::vector<unsigned> first, next;
    if (!dependencies.empty()) {
        first.resize(p.symbols.size()+1); next.resize(dependencies.size());
        for (unsigned n = 0; n < dependencies.size(); ++n) {
            auto owner = dependencies[n].owner.index;
            next[n] = first[owner]; first[owner] = n+1;
        }
    }
    auto demand = [&](unsigned id) {
        if (id && !live[id]) { live[id] = true; work.push_back(id); }
    };
    auto root = [&](const SymbolMetadata& m) {
        return m.object_root || m.keep_alias || m.role != SR_NONE || m.section ||
            (m.binding != SBM_INTERNAL && m.binding != SBM_WEAK);
    };
    for (const auto& g : p.globals) if (!g.declaration && root(p.symbols[g.symbol.index-1].metadata))
        demand(g.symbol.index);
    for (const auto& alias : p.aliases)
        if (p.symbols[alias.target.index-1].metadata.binding != SBM_WEAK) demand(alias.target.index);
    for (const auto& f : p.functions) if (!f.declaration) {
        const auto& m = p.symbols[f.symbol.index-1].metadata;
        if (root(m)) demand(f.symbol.index);
    }
    for (unsigned cursor = 0; cursor < work.size(); ++cursor) {
        auto id = work[cursor];
        if (!first.empty()) for (auto edge = first[id]; edge; edge = next[edge-1])
            demand(dependencies[edge-1].target.index);
        const auto& s = p.symbols[id-1];
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
                        if (p.operands[k].kind == Operand::Symbol) demand(p.operands[k].ref);
                }
            }
        }
    }
    return live;
}
}
