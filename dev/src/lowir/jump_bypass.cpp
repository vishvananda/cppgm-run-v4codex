#include "lowir/folding.h"
#include "support/id_index.h"
namespace lowir_model {
namespace {
struct BackEdge { unsigned from, next; };
struct PhiRewrite { unsigned instruction; Range operands; };
void bypass(Program& p, const Function& f, std::uint64_t& work)
{
    cppgm::IdIndex local;
    unsigned count = f.blocks.count;
    std::vector<unsigned> target(count+1), mapped(count+1), visiting(count+1), pred(count+1), path;
    std::vector<BackEdge> edges(1);
    std::uint64_t budget = 0;
    for (unsigned n = 1; n <= count; ++n) local.put(p.block_order[f.blocks.begin+n-1].index,n);
    cppgm::IdIndex distinct;
    for (unsigned b = 1; b <= count; ++b) {
        auto r = p.blocks[p.block_order[f.blocks.begin+b-1].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            const auto& i = p.instructions[n]; ++work; budget += 16*(1+std::uint64_t(i.operands.count));
            if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) return;
        }
        const auto& i = p.instructions[r.end()-1];
        if (b != 1 && r.count == 1 && i.opcode == Opcode::Jump) target[b] = local.get(p.operands[i.operands.begin].ref);
        for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
            auto a = p.operands[n]; if (a.kind != Operand::Label) continue;
            unsigned to = local.get(a.ref); auto key = (std::uint64_t(b)<<32)|to;
            if (distinct.get(key)) continue;
            distinct.put(key,1); edges.push_back({b,pred[to]}); pred[to] = edges.size()-1;
        }
    }
    // Memoized functional-graph traversal. Keep empty cycles (nontermination)
    // intact, including the entry block's identity.
    for (unsigned b = 1; b <= count; ++b) if (!mapped[b]) {
        unsigned n = b; path.clear();
        while (!mapped[n] && target[n] && !visiting[n]) { visiting[n] = 1; path.push_back(n); n = target[n]; ++work; }
        bool cycle = visiting[n];
        if (!mapped[n]) mapped[n] = n;
        for (unsigned k = path.size(); k; --k) {
            unsigned id = path[k-1]; mapped[id] = cycle ? id : mapped[n]; visiting[id] = 0;
        }
    }
    std::vector<PhiRewrite> rewrites;
    std::vector<Operand> args;
    auto charge = [&]() { if (!budget) return false; --budget; ++work; return true; };
    for (unsigned b = 1; b <= count; ++b) if (mapped[b] == b) {
        auto r = p.blocks[p.block_order[f.blocks.begin+b-1].index-1].instructions;
        for (unsigned n = r.begin; n < r.end() && p.instructions[n].opcode == Opcode::Phi; ++n) {
            PhiRewrite rewrite; rewrite.instruction = n; rewrite.operands.begin = args.size();
            cppgm::IdIndex values;
            const auto& i = p.instructions[n];
            for (unsigned k = i.operands.begin; k < i.operands.end(); k += 2) {
                auto value = p.operands[k+1]; path.clear(); path.push_back(local.get(p.operands[k].ref));
                while (!path.empty()) {
                    if (!charge()) return;
                    unsigned from = path.back(); path.pop_back();
                    if (mapped[from] != from) {
                        for (unsigned e = pred[from]; e; e = edges[e].next) {
                            if (!charge()) return;
                            path.push_back(edges[e].from);
                        }
                    } else if (auto old = values.get(from)) {
                        // Two choices become one edge only if their values
                        // agree. Otherwise keep the original diamond.
                        if (!same_scalar(args[old-1],value)) return;
                    } else {
                        args.push_back(Operand::label(p.block_order[f.blocks.begin+from-1]));
                        args.push_back(value); values.put(from,args.size());
                    }
                }
            }
            rewrite.operands.count = args.size()-rewrite.operands.begin; rewrites.push_back(rewrite);
        }
    }
    // Commit only after every affected phi has a complete, conflict-free edge
    // map within budget. The following reachability sweep removes old blocks.
    for (unsigned b = 1; b <= count; ++b) {
        auto r = p.blocks[p.block_order[f.blocks.begin+b-1].index-1].instructions;
        auto& i = p.instructions[r.end()-1];
        for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
            auto& a = p.operands[n];
            if (a.kind == Operand::Label) a.ref = p.block_order[f.blocks.begin+mapped[local.get(a.ref)]-1].index;
        }
    }
    for (const auto& rewrite : rewrites) {
        auto& i = p.instructions[rewrite.instruction]; i.operands.begin = p.operands.size(); i.operands.count = rewrite.operands.count;
        for (unsigned n = rewrite.operands.begin; n < rewrite.operands.end(); ++n) p.operands.push_back(args[n]);
    }
}
}
void bypass_empty_jumps(Program& p, std::uint64_t& work)
{
    for (const auto& f : p.functions) if (!f.declaration) bypass(p,f,work);
}
}
