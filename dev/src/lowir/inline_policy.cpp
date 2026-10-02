#include "lowir/inline_policy.h"
#include "lowir/ordinary_flow.h"
#include "lowir/call_effects.h"
namespace lowir_model {
bool inline_small_calls(Program& p, unsigned level, std::uint64_t& work)
{
    (void)level;
    InlinePolicy policy; policy.eligible.resize(p.functions.size()+1);
    policy.single.resize(p.functions.size()+1); policy.growth.resize(p.functions.size()+1);
    struct Edge { unsigned caller, next; };
    std::vector<Edge> edges(1);
    std::vector<unsigned> reverse(p.functions.size()+1), outstanding(p.functions.size()+1), ready;
    std::vector<bool> acyclic(p.functions.size()+1);
    for (unsigned fn = 1; fn <= p.functions.size(); ++fn) {
        const auto& f = p.functions[fn-1];
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                if (i.opcode != Opcode::Call) continue;
                auto a = p.operands[i.operands.begin]; if (a.kind != Operand::Symbol) continue;
                const auto& sym = p.symbols[a.ref-1]; if (sym.kind != Symbol::FunctionSymbol) continue;
                unsigned to = sym.entity; ++outstanding[fn];
                edges.push_back({fn,reverse[to]}); reverse[to] = edges.size()-1;
            }
        }
    }
    for (unsigned fn = 1; fn <= p.functions.size(); ++fn) if (!outstanding[fn]) ready.push_back(fn);
    for (unsigned next = 0; next < ready.size(); ++next) {
        auto fn = ready[next]; acyclic[fn] = true;
        for (auto e = reverse[fn]; e; e = edges[e].next) {
            ++work; auto caller = edges[e].caller; if (!--outstanding[caller]) ready.push_back(caller);
        }
    }
    std::vector<unsigned> calls(p.functions.size()+1);
    std::vector<bool> addressed(p.functions.size()+1);
    for (const auto& i : p.instructions) for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
        auto a = p.operands[n]; ++work;
        if (a.kind != Operand::Symbol) continue;
        const auto& s = p.symbols[a.ref-1]; if (s.kind != Symbol::FunctionSymbol) continue;
        if (i.opcode == Opcode::Call && n == i.operands.begin) ++calls[s.entity];
        else addressed[s.entity] = true;
    }
    for (const auto& d : p.data) if (d.kind == DataItem::Address) {
        const auto& s = p.symbols[d.symbol.index-1];
        if (s.kind == Symbol::FunctionSymbol) addressed[s.entity] = true;
    }
    bool any = false;
    for (unsigned fn = 1; fn <= p.functions.size(); ++fn) {
        const auto& f = p.functions[fn-1]; const auto& m = p.symbols[f.symbol.index-1].metadata;
        if (f.declaration || !calls[fn] || m.no_inline || !acyclic[fn]) continue;
        const auto& sig = p.signatures[f.signature.index-1];
        if (sig.boundary.arity != CAM_FIXED) continue;
        unsigned size = 0, hot_size = 0, call_count = 0; bool legal = true;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            bool cold = false;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work; ++size;
                cold |= i.opcode == Opcode::Call && call_boundary(p,i).returns == CRM_NORETURN;
                legal &= i.opcode != Opcode::StackAlloc && i.opcode != Opcode::VaStart && i.opcode != Opcode::VaArg &&
                    i.opcode != Opcode::Throw && i.opcode != Opcode::Resume &&
                    i.opcode != Opcode::EhTry && i.opcode != Opcode::EhCleanup;
                call_count += i.opcode == Opcode::Call;
            }
            hot_size += cold && r.count > 4 ? 4 : r.count;
        }
        bool single = calls[fn] == 1 && !addressed[fn] && !m.object_root && !m.keep_alias &&
            m.role == SR_NONE && (m.binding == SBM_INTERNAL || m.binding == SBM_WEAK);
        unsigned limit = single ? 512 : m.inline_hint || m.prefer_local ? (call_count ? 32 : 128) : 40;
        if (call_count && f.blocks.count == 1 && !single && !m.inline_hint && !m.prefer_local) limit = 6;
        OrdinaryFlow flow(p,f,work);
        std::vector<unsigned> degree = flow.incoming, queue;
        // Incoming is an edge-list head, so count degrees explicitly.
        degree.assign(flow.blocks.size(),0);
        for (unsigned e = 1; e < flow.edges.size(); ++e) ++degree[flow.edges[e].to];
        for (unsigned b = 1; b < degree.size(); ++b) if (!degree[b]) queue.push_back(b);
        for (unsigned next = 0; next < queue.size(); ++next)
            for (auto e = flow.outgoing[queue[next]]; e; e = flow.edges[e].next_out)
                if (!--degree[flow.edges[e].to]) queue.push_back(flow.edges[e].to);
        bool loop = queue.size()+1 != flow.blocks.size();
        legal &= !loop || single || m.inline_hint;
        policy.single[fn] = single; policy.growth[fn] = size ? size-1 : 0;
        policy.eligible[fn] = legal && (size <= limit || (size <= 128 && hot_size <= limit));
        any |= policy.eligible[fn];
    }
    // Admission bounds the whole cloned pool, including nested expansion;
    // no callee growth can make another caller's budget silently grow.
    policy.unit_work = 32ull*(p.instructions.size()+p.operands.size()+p.parameters.size()+p.slots.size()+p.functions.size()+1);
    if (any) expand_optional_calls(p,policy);
    return any;
}
void prune_support_functions(Program& p, std::uint64_t& work)
{
    std::vector<bool> live(p.symbols.size()+1); std::vector<unsigned> pending;
    auto demand = [&](unsigned id) { if (id && !live[id]) { live[id] = true; pending.push_back(id); } };
    for (const auto& g : p.globals) if (!g.declaration) demand(g.symbol.index);
    for (const auto& a : p.aliases) demand(a.target.index);
    for (const auto& f : p.functions) if (!f.declaration) {
        const auto& m = p.symbols[f.symbol.index-1].metadata;
        if (m.object_root || m.keep_alias || m.role != SR_NONE ||
            (m.binding != SBM_INTERNAL && m.binding != SBM_WEAK)) demand(f.symbol.index);
    }
    for (unsigned next = 0; next < pending.size(); ++next) {
        const auto& s = p.symbols[pending[next]-1]; ++work;
        demand(s.metadata.tls_for.index);
        if (s.kind == Symbol::GlobalSymbol) {
            const auto& g = p.globals[s.entity-1];
            for (unsigned n = g.data.begin; n < g.data.end(); ++n) {
                ++work; if (p.data[n].kind == DataItem::Address) demand(p.data[n].symbol.index);
            }
        } else if (s.kind == Symbol::FunctionSymbol) {
            const auto& f = p.functions[s.entity-1];
            for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
                auto r = p.blocks[p.block_order[b].index-1].instructions;
                for (unsigned n = r.begin; n < r.end(); ++n) for (unsigned k = p.instructions[n].operands.begin; k < p.instructions[n].operands.end(); ++k) {
                    ++work; if (p.operands[k].kind == Operand::Symbol) demand(p.operands[k].ref);
                }
            }
        }
    }
    // Preserve entity IDs/signatures for any adapter-side inspection. Dead
    // support bodies become declarations; scalar/CFG compaction releases IR.
    for (auto& f : p.functions) if (!live[f.symbol.index]) {
        f.declaration = true; f.blocks.count = 0; f.slots.count = 0;
    }
}
}
