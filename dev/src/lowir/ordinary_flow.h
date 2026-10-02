#pragma once
#include "lowir/model.h"
#include "support/id_index.h"
namespace lowir_model {
// Function-local ordinary CFG. EH registrations explicitly invalidate this
// dominance model; clients then use their conservative local path.
struct OrdinaryFlow {
    struct Edge { unsigned from, to, next_in, next_out; };
    cppgm::IdIndex local;
    std::vector<BlockId> blocks;
    std::vector<Edge> edges;
    std::vector<unsigned> incoming, outgoing, rpo, rank, parent, first_child, next_child;
    std::vector<unsigned> enter, leave;
    bool exceptional = false, complete = false;
    std::uint64_t size = 0;
    OrdinaryFlow(const Program&, const Function&, std::uint64_t& work);
    bool dominance(std::uint64_t& work);
    bool dominates(unsigned a, unsigned b) const { return enter[a] && enter[a] <= enter[b] && leave[b] <= leave[a]; }
};
// Conservative profitability admission: without call/loop cost summaries,
// cyclic functions containing calls retain local optimization. This is not a
// legality restriction and can be relaxed by a later measured policy.
bool has_call_cycle(const Program&, const Function&, std::uint64_t& work);
void propagate_edge_facts(Program&, const OrdinaryFlow&, const std::vector<unsigned>& definitions, std::uint64_t& work);
}
