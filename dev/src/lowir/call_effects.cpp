#include "lowir/call_effects.h"
#include "support/id_index.h"
namespace lowir_model {
FunctionBoundaryMetadata call_boundary(const Program& p, const Instruction& i)
{
    if (i.signature) return p.signatures[i.signature.index-1].boundary;
    auto a = p.operands[i.operands.begin];
    if (a.kind == Operand::Symbol) {
        const auto& s = p.symbols[a.ref-1];
        if (s.kind == Symbol::FunctionSymbol) return p.signatures[p.functions[s.entity-1].signature.index-1].boundary;
    }
    return FunctionBoundaryMetadata();
}
namespace {
bool may_unwind(const Program& p, const Instruction& i) {
    return i.opcode == Opcode::Throw || i.opcode == Opcode::Resume ||
        (i.opcode == Opcode::Call && call_boundary(p,i).unwind != CUM_NO);
}
// A persistent registration stack; one node per reachable registration and
// one incoming state per block. Conflicting merges decline transactionally.
bool retire(Program& p, Function& f, std::uint64_t& budget, std::uint64_t& work)
{
    struct Region { unsigned parent, instruction, handler; bool keep; };
    std::vector<Region> regions(1);
    cppgm::IdIndex incoming, registration;
    std::vector<unsigned> pending;
    std::vector<std::pair<unsigned,unsigned>> ends;
    bool valid = true, throws = false;
    auto edge = [&](unsigned block, unsigned state) {
        auto old = incoming.get(block);
        if (!old) { incoming.put(block,state+1); pending.push_back(block); }
        else if (old != state+1) valid = false;
    };
    edge(p.block_order[f.blocks.begin].index,0);
    for (unsigned next = 0; next < pending.size() && valid; ++next) {
        unsigned b = pending[next], state = incoming.get(b)-1;
        auto range = p.blocks[b-1].instructions;
        for (unsigned n = range.begin; n < range.end() && valid; ++n) {
            const auto& i = p.instructions[n];
            if (!budget) return false;
            --budget; ++work;
            if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) {
                auto id = registration.get(n+1);
                if (!id) {
                    id = regions.size(); registration.put(n+1,id);
                    regions.push_back({state,n,p.operands[i.operands.begin].ref,false});
                } else if (regions[id].parent != state) { valid = false; break; }
                state = id;
            } else if (i.opcode == Opcode::EhEnd) {
                if (!state) { valid = false; break; }
                ends.push_back({n,state}); state = regions[state].parent;
            } else if (may_unwind(p,i)) {
                throws = true;
                // Conservative outward propagation includes nested regions.
                // Each registration is charged/visited at most once here.
                for (unsigned r = state; r && !regions[r].keep; r = regions[r].parent) {
                    regions[r].keep = true;
                    // Cleanup landings retain the registration until eh_end;
                    // catch dispatch removes it before entering its handler.
                    auto& region = regions[r];
                    bool cleanup = p.instructions[region.instruction].opcode == Opcode::EhCleanup;
                    auto handler = p.blocks[region.handler-1].instructions;
                    for (unsigned k = handler.begin; k < handler.end(); ++k) {
                        if (!budget) return false;
                        --budget; ++work;
                        const auto& h = p.instructions[k];
                        if (h.opcode == Opcode::EhCleanup && !h.operands.count) cleanup = true;
                        if (h.opcode != Opcode::EhCatch && h.opcode != Opcode::EhCatchAll && h.opcode != Opcode::EhFilter &&
                            !(h.opcode == Opcode::EhCleanup && !h.operands.count)) break;
                    }
                    edge(region.handler,cleanup ? r : region.parent);
                }
            }
            if (terminator(i.opcode)) for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                if (!budget) return false;
                --budget; ++work;
                if (p.operands[k].kind == Operand::Label) edge(p.operands[k].ref,state);
            }
        }
    }
    if (!valid) return false;
    auto erase = [&](unsigned n) { auto& i = p.instructions[n]; i.opcode = Opcode::Nop; i.type = Type(); i.operands.count = 0; };
    for (unsigned r = 1; r < regions.size(); ++r) if (!regions[r].keep) erase(regions[r].instruction);
    for (auto e : ends) if (!regions[e.second].keep) erase(e.first);
    return !throws;
}
}
void simplify_call_regions(Program& p, std::uint64_t& work)
{
    struct Edge { unsigned caller, next; };
    std::vector<Edge> edges(1);
    std::vector<unsigned> reverse(p.functions.size());
    std::vector<bool> queued(p.functions.size()); std::vector<unsigned> pending;
    std::uint64_t budget = 32ull*(p.instructions.size()+p.operands.size()+p.blocks.size()+1);
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        const auto& f = p.functions[fn]; if (f.declaration) continue;
        pending.push_back(fn); queued[fn] = true;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                if (i.opcode != Opcode::Call) continue;
                auto a = p.operands[i.operands.begin]; if (a.kind != Operand::Symbol) continue;
                const auto& s = p.symbols[a.ref-1]; if (s.kind != Symbol::FunctionSymbol) continue;
                auto to = s.entity-1; edges.push_back({fn,reverse[to]}); reverse[to] = edges.size()-1;
            }
        }
    }
    // A published no-unwind fact can only become stronger as regions/calls
    // disappear. Only its indexed callers are requeued; total work is capped.
    for (unsigned next = 0; next < pending.size() && budget; ++next) {
        auto fn = pending[next]; queued[fn] = false;
        auto& f = p.functions[fn]; auto& boundary = p.signatures[f.signature.index-1].boundary;
        bool safe = retire(p,f,budget,work);
        if (!safe || boundary.unwind == CUM_NO) continue;
        boundary.unwind = CUM_NO;
        for (auto e = reverse[fn]; e; e = edges[e].next) {
            auto caller = edges[e].caller;
            if (!queued[caller]) { pending.push_back(caller); queued[caller] = true; }
        }
    }
}
}
