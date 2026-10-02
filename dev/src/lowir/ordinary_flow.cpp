#include "lowir/ordinary_flow.h"
#include <algorithm>
namespace lowir_model {
bool has_call_cycle(const Program& p, const Function& f, std::uint64_t& work)
{
    if (f.blocks.count == 1) {
        auto r = p.blocks[p.block_order[f.blocks.begin].index-1].instructions;
        const auto& t = p.instructions[r.end()-1];
        if (t.opcode != Opcode::Jump && t.opcode != Opcode::Branch && t.opcode != Opcode::Switch) return false;
    }
    bool any = false;
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
        auto r = p.blocks[p.block_order[n].index-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) { ++work; any |= p.instructions[k].opcode == Opcode::Call; }
    }
    if (!any) return false;
    OrdinaryFlow flow(p,f,work);
    std::vector<bool> calls(flow.blocks.size()), active(flow.blocks.size());
    for (unsigned b = 1; b < flow.blocks.size(); ++b) {
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) calls[b] = calls[b] || p.instructions[n].opcode == Opcode::Call;
    }
    // Iterative Tarjan: a call outside a cycle does not make its caller's
    // loops expensive. Each block and ordinary edge is visited once.
    std::vector<unsigned> number(flow.blocks.size()), low(flow.blocks.size()), members;
    struct Visit { unsigned block, edge; };
    std::vector<Visit> stack; unsigned serial = 0;
    auto discover = [&](unsigned b) {
        number[b] = low[b] = ++serial; active[b] = true;
        members.push_back(b); stack.push_back({b,flow.outgoing[b]});
    };
    for (unsigned root = 1; root < flow.blocks.size(); ++root) if (!number[root]) {
        discover(root);
        while (!stack.empty()) {
            ++work;
            unsigned b = stack.back().block, edge = stack.back().edge;
            if (edge) {
                unsigned to = flow.edges[edge].to; stack.back().edge = flow.edges[edge].next_out;
                if (!number[to]) { discover(to); continue; }
                if (active[to]) low[b] = std::min(low[b],number[to]);
                continue;
            }
            if (low[b] == number[b]) {
                unsigned count = 0, member; bool call = false, self = false;
                for (unsigned e = flow.outgoing[b]; e; e = flow.edges[e].next_out) self |= flow.edges[e].to == b;
                do { member = members.back(); members.pop_back(); active[member] = false; ++count; call |= calls[member]; } while (member != b);
                if (call && (count > 1 || self)) return true;
            }
            stack.pop_back();
            if (!stack.empty()) { auto parent = stack.back().block; low[parent] = std::min(low[parent],low[b]); }
        }
    }
    return false;
}
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
