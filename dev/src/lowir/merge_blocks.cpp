#include "lowir/folding.h"
namespace lowir_model {
bool merge_forward_blocks(Program& p, std::uint64_t& work)
{
    // Joining one forward ordinary edge preserves the exact EH operation
    // sequence. Handler entries are roots and cannot be absorbed. Source
    // ordering of every operand definition is proved before any movement.
    std::vector<unsigned> incoming(p.blocks.size()+1), pred(p.blocks.size()+1),
        ordinal(p.blocks.size()+1), root(p.blocks.size()+1), tail(p.blocks.size()+1), next(p.blocks.size()+1);
    std::vector<bool> handler(p.blocks.size()+1);
    for (unsigned n = 0; n < p.block_order.size(); ++n) {
        auto b = p.block_order[n].index; ordinal[b] = n+1; root[b] = tail[b] = b;
        auto r = p.blocks[b-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) {
            const auto& i = p.instructions[k]; ++work;
            if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count)
                handler[p.operands[i.operands.begin].ref] = true;
        }
        const auto& t = p.instructions[r.end()-1];
        for (unsigned k = t.operands.begin; k < t.operands.end(); ++k) {
            auto a = p.operands[k]; ++work;
            if (a.kind == Operand::Label) { ++incoming[a.ref]; pred[a.ref] = b; }
        }
    }
    bool changed = false;
    for (const auto& f : p.functions) for (unsigned n = f.blocks.begin+1; n < f.blocks.end(); ++n) {
        auto b = p.block_order[n].index, from = pred[b];
        if (handler[b] || incoming[b] != 1 || ordinal[from] >= ordinal[b]) continue;
        auto r = p.blocks[b-1].instructions, fr = p.blocks[from-1].instructions;
        if (p.instructions[fr.end()-1].opcode != Opcode::Jump || p.instructions[r.begin].opcode == Opcode::Phi) continue;
        auto destination = root[from];
        if (tail[destination] != from) continue;
        // Use the root's original end: crossing an unrelated intervening
        // definition is not legal even if its address could be rematerialized.
        unsigned earlier = p.blocks[destination-1].instructions.end(); bool safe = true;
        for (unsigned k = r.begin; k < r.end() && safe; ++k) {
            const auto& i = p.instructions[k];
            for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) {
                auto a = p.operands[j]; ++work;
                if (a.kind != Operand::Temporary) continue;
                auto definition = p.values[a.ref-1].definition;
                if (definition > earlier && definition <= r.begin) { safe = false; break; }
            }
        }
        if (!safe) continue;
        next[from] = b; tail[destination] = b; root[b] = destination; changed = true;
    }
    if (!changed) return false;
    Pool<Instruction> instructions; Pool<Operand> operands; Pool<BlockId> order;
    for (auto& v : p.values) { v.definition = 0; v.defined = false; }
    for (const auto& a : p.parameters) p.values[a.value.index-1].defined = true;
    for (auto& f : p.functions) {
        auto old = f.blocks; f.blocks.begin = order.size();
        for (unsigned n = old.begin; n < old.end(); ++n) {
            auto b = p.block_order[n].index; if (root[b] != b) continue;
            auto begin = instructions.size(); order.push_back(BlockId(b));
            for (unsigned member = b; member; member = next[member]) {
                auto r = p.blocks[member-1].instructions;
                for (unsigned k = r.begin; k < r.end()-unsigned(next[member] != 0); ++k) {
                    auto i = p.instructions[k]; auto args = i.operands; i.operands.begin = operands.size();
                    for (unsigned j = args.begin; j < args.end(); ++j) {
                        auto a = p.operands[j]; if (a.kind == Operand::Label) a.ref = root[a.ref];
                        operands.push_back(a); ++work;
                    }
                    if (i.destination) {
                        auto& v = p.values[i.destination.index-1];
                        if (!v.defined) v.definition = instructions.size()+1;
                        v.defined = true;
                    }
                    instructions.push_back(i);
                }
            }
            p.blocks[b-1].instructions.begin = begin;
            p.blocks[b-1].instructions.count = instructions.size()-begin;
        }
        f.blocks.count = order.size()-f.blocks.begin;
    }
    p.instructions.swap(instructions); p.operands.swap(operands); p.block_order.swap(order);
    return true;
}
}
