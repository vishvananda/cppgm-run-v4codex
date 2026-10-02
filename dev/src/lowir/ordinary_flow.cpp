#include "lowir/ordinary_flow.h"
#include <algorithm>
namespace lowir_model {
OrdinaryFlow::OrdinaryFlow(const Program& p, const Function& f, std::uint64_t& work)
    : blocks(1), edges(1), incoming(f.blocks.count+1), outgoing(f.blocks.count+1),
      rank(f.blocks.count+1), parent(f.blocks.count+1), first_child(f.blocks.count+1),
      next_child(f.blocks.count+1), enter(f.blocks.count+1), leave(f.blocks.count+1)
{
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) { local.put(p.block_order[n].index,blocks.size()); blocks.push_back(p.block_order[n]); }
    cppgm::IdIndex seen;
    for (unsigned b = 1; b < blocks.size(); ++b) {
        auto r = p.blocks[blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            const auto& i = p.instructions[n]; ++work; size += 1+i.operands.count;
            exceptional |= (i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count;
        }
        const auto& i = p.instructions[r.end()-1];
        for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
            auto a = p.operands[n]; if (a.kind != Operand::Label) continue;
            unsigned to = local.get(a.ref); auto key = (std::uint64_t(b)<<32)|to; ++work;
            if (seen.get(key)) continue;
            seen.put(key,1); edges.push_back({b,to,incoming[to],outgoing[b]}); incoming[to] = outgoing[b] = edges.size()-1;
        }
    }
}
bool OrdinaryFlow::dominance(std::uint64_t& work)
{
    if (exceptional) return false;
    // Iterative DFS avoids call-stack growth on generated long chains.
    struct Visit { unsigned block, edge; };
    std::vector<Visit> stack; std::vector<bool> seen(blocks.size());
    stack.push_back({1,outgoing[1]}); seen[1] = true;
    while (!stack.empty()) {
        auto& v = stack.back(); ++work;
        if (!v.edge) { rpo.push_back(v.block); stack.pop_back(); continue; }
        unsigned to = edges[v.edge].to; v.edge = edges[v.edge].next_out;
        if (!seen[to]) { seen[to] = true; stack.push_back({to,outgoing[to]}); }
    }
    std::reverse(rpo.begin(),rpo.end());
    for (unsigned n = 0; n < rpo.size(); ++n) rank[rpo[n]] = n;
    parent[1] = 1;
    std::vector<unsigned> pending; std::vector<bool> queued(blocks.size());
    auto enqueue = [&](unsigned b) { if (b != 1 && seen[b] && !queued[b]) { queued[b] = true; pending.push_back(b); } };
    for (unsigned n = rpo.size(); n > 1; --n) enqueue(rpo[n-1]);
    std::uint64_t budget = 32*(size+edges.size()+1);
    auto charge = [&]() { if (!budget) return false; --budget; ++work; return true; };
    // Parent facts move toward the entry. Only successors of changed facts
    // are dirtied. Exhaustion rejects the analysis, never a partial proof.
    while (!pending.empty()) {
        unsigned b = pending.back(); pending.pop_back(); queued[b] = false;
        unsigned candidate = 0;
        for (unsigned e = incoming[b]; e; e = edges[e].next_in) {
            if (!charge()) return false;
            unsigned pred = edges[e].from; if (!parent[pred]) continue;
            if (!candidate) { candidate = pred; continue; }
            while (candidate != pred) {
                if (!charge()) return false;
                if (rank[candidate] > rank[pred]) candidate = parent[candidate];
                else pred = parent[pred];
            }
        }
        if (candidate && candidate != parent[b]) {
            parent[b] = candidate;
            for (unsigned e = outgoing[b]; e; e = edges[e].next_out) { if (!charge()) return false; enqueue(edges[e].to); }
        }
    }
    for (unsigned b : rpo) if (b != 1) { next_child[b] = first_child[parent[b]]; first_child[parent[b]] = b; }
    unsigned stamp = 0; stack.push_back({1,first_child[1]}); enter[1] = ++stamp;
    while (!stack.empty()) {
        auto& v = stack.back(); ++work;
        if (!v.edge) { leave[v.block] = ++stamp; stack.pop_back(); continue; }
        unsigned child = v.edge; v.edge = next_child[child]; enter[child] = ++stamp;
        stack.push_back({child,first_child[child]});
    }
    complete = true; return true;
}
}
