#include "native/model.h"
#include "support/id_index.h"
namespace native {
void prepare_host_eh(Function& f)
{
    using lowir_model::require;
    cppgm::IdIndex blocks, selectors, stacks;
    f.host_outer.assign(f.blocks.size(),0);
    for (unsigned b = 0; b < f.blocks.size(); ++b) blocks.put(f.blocks[b].id,b+1);
    auto type_index = [&](SymbolId type, unsigned selector) {
        auto key = (std::uint64_t(type.index)<<32)+selector+1;
        auto index = selectors.get(key);
        if (!index) { index = f.host_types.size()+1; f.host_types.push_back(type); selectors.put(key,index); }
        return index;
    };
    auto filter_type = [&](unsigned index) {
        do { auto b = index&127; index >>= 7; f.host_filters.push_back(b|(index ? 128 : 0)); } while (index);
    };
    for (auto& c : f.exception_clauses) {
        if (!c.filtered) { c.host_selector = type_index(c.type,c.selector); continue; }
        // Negative actions index zero-terminated ULEB type-index lists after
        // TType; positive actions index backwards into its pointer table.
        require(f.host_filters.size() < INT32_MAX,"host exception filter table overflow");
        c.host_selector = -int(f.host_filters.size())-1;
        for (unsigned n = c.filter.begin; n < c.filter.end(); ++n) filter_type(type_index(f.exception_filter_types[n],0));
        filter_type(0);
    }
    // Persistent region stack: one node per push instruction, no copied stacks
    // at edges. Every reachable block is visited once; joins require identity.
    struct Region { unsigned parent = 0, landing = 0, active = 0; };
    std::vector<Region> regions(1);
    auto push = [&](unsigned parent, unsigned landing) {
        auto key = (std::uint64_t(parent)<<32)+landing+1;
        auto old = stacks.get(key); if (old) return old;
        Region r; r.parent = parent; r.landing = landing; r.active = landing ? landing : regions[parent].active;
        regions.push_back(r); unsigned id = regions.size()-1; stacks.put(key,id); return id;
    };
    std::vector<unsigned> incoming(f.blocks.size(),~0u), pending;
    auto edge = [&](unsigned label, unsigned state) {
        auto at = blocks.get(label); require(at,"missing host EH block"); --at;
        if (incoming[at] == ~0u) { incoming[at] = state; pending.push_back(at); }
        else require(incoming[at] == state,"inconsistent protected regions at native control join");
    };
    if (f.blocks.empty()) return;
    edge(f.blocks[0].id,0);
    for (unsigned cursor = 0; cursor < pending.size(); ++cursor) {
        auto b = pending[cursor]; unsigned state = incoming[b];
        const auto& range = f.blocks[b].instructions;
        bool terminal = false;
        for (unsigned n = range.begin; n < range.end(); ++n) {
            auto& i = f.instructions[n]; i.host_handler = regions[state].landing;
            if (i.op == Op::Resume) i.host_handler = regions[state].active;
            if (i.op == Op::EhPush) {
                f.host_outer[blocks.get(i.args[0].id)-1] = blocks.get(regions[state].active);
                // A cleanup landing retains a retired stack frame for eh_end,
                // but cannot protect its own cleanup again. Intern that inert
                // state so shared suffixes from sibling regions agree.
                edge(i.args[0].id,i.args[1].bits ? push(state,0) : state);
                state = push(state,i.args[0].id);
            } else if (i.op == Op::EhPop) {
                require(state,"unbalanced native protected region exit"); state = regions[state].parent;
            } else if (i.op == Op::Jump || i.op == Op::Jcc) {
                edge(i.args[0].id,state); terminal = i.op == Op::Jump;
            } else if (i.op == Op::Return || i.op == Op::Resume || i.op == Op::Trap) terminal = true;
        }
        if (!terminal && b+1 < f.blocks.size()) edge(f.blocks[b+1].id,state);
    }
    // Installing a region is not itself an exceptional edge. An otherwise
    // empty handler must not acquire LSDA coverage from its own dead cleanup.
    std::vector<bool> reachable(f.blocks.size()); pending.clear();
    auto visit = [&](unsigned label) {
        unsigned b = blocks.get(label)-1;
        if (!reachable[b]) { reachable[b] = true; pending.push_back(b); }
    };
    visit(f.blocks[0].id);
    for (unsigned cursor = 0; cursor < pending.size(); ++cursor) {
        auto b = pending[cursor]; bool terminal = false;
        const auto& range = f.blocks[b].instructions;
        for (unsigned n = range.begin; n < range.end(); ++n) {
            const auto& i = f.instructions[n];
            if (i.op == Op::Call && i.boundary.unwind != ir_model::CUM_NO && i.host_handler) visit(i.host_handler);
            if (i.op == Op::Resume && i.host_handler) visit(i.host_handler);
            if (i.op == Op::Jump || i.op == Op::Jcc) { visit(i.args[0].id); terminal = i.op == Op::Jump; }
            else if (i.op == Op::Return || i.op == Op::Resume || i.op == Op::Trap) terminal = true;
        }
        if (!terminal && b+1 < f.blocks.size()) visit(f.blocks[b+1].id);
    }
    for (unsigned b = 0; b < f.blocks.size(); ++b) if (!reachable[b])
        for (unsigned n = f.blocks[b].instructions.begin; n < f.blocks[b].instructions.end(); ++n) f.instructions[n].host_handler = ~0u;
}
} // namespace native
