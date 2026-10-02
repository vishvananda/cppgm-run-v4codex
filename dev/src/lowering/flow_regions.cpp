#include "lowering/flow_proof.h"
#include "support/id_index.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using namespace lowir_model;
std::vector<BlockId> flow_exception_targets(const Program& p, FunctionId function, unsigned first_instruction, std::size_t& work, std::size_t& edges)
{
    auto blocks = p.functions[function.index-1].blocks;
    auto first = p.block_order[blocks.begin].index;
    auto count = p.blocks.size()-first+1;
    std::vector<BlockId> handlers(p.instructions.size()-first_instruction);
    std::vector<unsigned char> cleanup(count);
    for (auto b = blocks.begin; b < blocks.end(); ++b) {
        auto id = p.block_order[b]; auto instructions = p.blocks[id.index-1].instructions;
        for (auto n = instructions.begin; n < instructions.end(); ++n) {
            ++work;
            const auto& i = p.instructions[n];
            if (i.opcode == Opcode::EhCleanup && !i.operands.count) cleanup[id.index-first] = 1;
            else if (i.opcode != Opcode::EhCatch && i.opcode != Opcode::EhCatchAll && i.opcode != Opcode::EhFilter) break;
        }
    }
    struct Region { unsigned parent, landing, active; };
    std::vector<Region> regions(1,Region{0,0,0}); IdIndex stacks;
    auto push = [&](unsigned parent, unsigned landing) {
        auto key = (std::uint64_t(parent)<<32)+landing+1;
        if (auto old = stacks.get(key)) return old;
        auto id = unsigned(regions.size());
        regions.push_back({parent,landing,landing ? landing : regions[parent].active}); stacks.put(key,id); return id;
    };
    std::vector<unsigned> incoming(count,~0u), pending;
    auto edge = [&](unsigned block, unsigned state) {
        ++edges;
        if (block < first || p.blocks[block-1].owner.index != function.index) throw std::logic_error("foreign flow region");
        auto& old = incoming[block-first];
        if (old == ~0u) { old = state; pending.push_back(block); }
        else if (old != state) throw std::logic_error("inconsistent flow region");
    };
    edge(first,0);
    for (unsigned cursor = 0; cursor < pending.size(); ++cursor) {
        auto block = pending[cursor], state = incoming[block-first];
        auto instructions = p.blocks[block-1].instructions;
        for (auto n = instructions.begin; n < instructions.end(); ++n) {
            ++work;
            const auto& i = p.instructions[n]; auto begin = i.operands.begin;
            handlers[n-first_instruction] = BlockId(i.opcode == Opcode::Resume ? regions[state].active : regions[state].landing);
            if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) {
                auto landing = p.operands[begin].ref;
                edge(landing,i.opcode == Opcode::EhCleanup || cleanup[landing-first] ? push(state,0) : state);
                state = push(state,landing);
            } else if (i.opcode == Opcode::EhEnd) {
                if (!state) throw std::logic_error("unbalanced flow region");
                state = regions[state].parent;
            } else if (i.opcode == Opcode::Jump || i.opcode == Opcode::Branch || i.opcode == Opcode::Switch)
                for (auto j = begin; j < i.operands.end(); ++j)
                    if (p.operands[j].kind == Operand::Label) edge(p.operands[j].ref,state);
        }
    }
    return handlers;
}
} }
