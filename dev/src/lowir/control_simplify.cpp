#include "lowir/folding.h"
#include "support/id_index.h"
namespace lowir_model {
namespace {
void fold_terminal(Program& p, Instruction& i) {
    auto* a = p.operands.data()+i.operands.begin;
    Operand target;
    if (i.opcode == Opcode::Branch) {
        if (a[0].kind == Operand::Integer) target = a[a[0].data.integer || a[0].integer_high() ? 1 : 2];
        else if (a[1].ref == a[2].ref) target = a[1];
        else return;
    } else if (i.opcode == Opcode::Switch && a[0].kind == Operand::Integer) {
        target = a[1];
        for (unsigned n = 2; n < i.operands.count; n += 2)
            if (a[0].data.integer == a[n].data.integer) { target = a[n+1]; break; }
    } else return;
    i.opcode = Opcode::Jump; i.type = Type(); a[0] = target; i.operands.count = 1;
}
}
void simplify_control(Program& p, std::uint64_t& work)
{
    std::vector<bool> live(p.blocks.size()+1), permitted(p.functions.size());
    std::vector<unsigned> pending, address_block(p.values.size()+1);
    // LowIR permits a slot/global address definition in an ordinarily dead
    // block to be rematerialized by a landing pad. Retain that definition's
    // block (and its edges), rather than erasing an exceptional dependency.
    for (auto id : p.block_order) {
        auto r = p.blocks[id.index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            const auto& i = p.instructions[n];
            if (i.destination && (i.opcode == Opcode::Addr || i.opcode == Opcode::Index || i.opcode == Opcode::Copy))
                address_block[i.destination.index] = id.index;
        }
    }
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        const auto& f = p.functions[fn]; if (f.declaration) continue;
        bool handlers = false;
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
            auto range = p.blocks[p.block_order[n].index-1].instructions;
            for (unsigned k = range.begin; k < range.end(); ++k) {
                auto op = p.instructions[k].opcode; ++work;
                handlers |= op == Opcode::EhTry || op == Opcode::EhCleanup;
            }
        }
        permitted[fn] = !handlers;
        auto enqueue = [&](unsigned id) { if (!live[id]) { live[id] = true; pending.push_back(id); } };
        enqueue(p.block_order[f.blocks.begin].index);
        while (!pending.empty()) {
            unsigned id = pending.back(); pending.pop_back(); ++work;
            auto r = p.blocks[id-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                if (handlers) for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto a = p.operands[k]; ++work;
                    if (a.kind == Operand::Temporary && address_block[a.ref]) enqueue(address_block[a.ref]);
                }
                if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count)
                    enqueue(p.operands[i.operands.begin].ref);
            }
            auto& t = p.instructions[r.end()-1];
            fold_terminal(p,t);
            for (unsigned n = t.operands.begin; n < t.operands.end(); ++n)
                if (p.operands[n].kind == Operand::Label) enqueue(p.operands[n].ref);
        }
    }
    // One indexed edge census repairs phis after dropping ordinary edges.
    cppgm::IdIndex edges;
    std::vector<unsigned> predecessors(p.blocks.size()+1);
    for (unsigned b = 1; b <= p.blocks.size(); ++b) if (live[b]) {
        const auto& t = p.instructions[p.blocks[b-1].instructions.end()-1];
        for (unsigned n = t.operands.begin; n < t.operands.end(); ++n) if (p.operands[n].kind == Operand::Label) {
            unsigned target = p.operands[n].ref; auto key = (std::uint64_t(b)<<32)|target;
            if (!edges.get(key)) { edges.put(key,1); ++predecessors[target]; }
            ++work;
        }
    }
    // Merge only adjacent source-order blocks, with one ordinary predecessor
    // and no phi in the successor. This cannot move definitions ahead of uses.
    std::vector<unsigned> mapping(p.blocks.size()+1), merged(p.blocks.size()+1);
    std::vector<bool> omit_jump(p.instructions.size());
    Pool<Block> blocks; Pool<BlockId> order;
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        auto& f = p.functions[fn]; auto old = f.blocks;
        f.blocks.begin = order.size(); f.blocks.count = 0; unsigned previous = 0;
        for (unsigned n = old.begin; n < old.end(); ++n) {
            auto id = p.block_order[n].index; if (!live[id]) continue;
            bool join = false;
            if (previous && permitted[fn] && predecessors[id] == 1 &&
                p.instructions[p.blocks[id-1].instructions.begin].opcode != Opcode::Phi) {
                auto pos = p.blocks[previous-1].instructions.end()-1;
                const auto& t = p.instructions[pos];
                join = t.opcode == Opcode::Jump && p.operands[t.operands.begin].ref == id;
                if (join) omit_jump[pos] = true;
            }
            if (join) { mapping[id] = mapping[previous]; merged[id] = previous; }
            else {
                blocks.push_back(p.blocks[id-1]); mapping[id] = blocks.size();
                order.push_back(BlockId(blocks.size())); ++f.blocks.count;
            }
            previous = id;
        }
    }
    Pool<Instruction> instructions; Pool<Operand> operands;
    for (auto& v : p.values) { v.definition = 0; v.defined = false; }
    for (const auto& a : p.parameters) p.values[a.value.index-1].defined = true;
    for (auto id : p.block_order) if (live[id.index]) {
        auto& b = blocks[mapping[id.index]-1];
        if (!merged[id.index]) { b.instructions.begin = instructions.size(); b.instructions.count = 0; }
        auto range = p.blocks[id.index-1].instructions;
        for (unsigned n = range.begin; n < range.end(); ++n) if (!omit_jump[n]) {
            auto i = p.instructions[n]; auto args = i.operands;
            i.operands.begin = operands.size(); i.operands.count = 0;
            for (unsigned j = args.begin; j < args.end(); ++j) {
                auto a = p.operands[j];
                if (i.opcode == Opcode::Phi && a.kind == Operand::Label &&
                    !edges.get((std::uint64_t(a.ref)<<32)|id.index)) { ++j; continue; }
                if (a.kind == Operand::Label) { require(mapping[a.ref],"missing live CFG target"); a.ref = mapping[a.ref]; }
                operands.push_back(a); ++i.operands.count; ++work;
            }
            if (i.destination) {
                auto& v = p.values[i.destination.index-1];
                if (!v.defined) v.definition = instructions.size()+1;
                v.defined = true;
            }
            instructions.push_back(i); ++b.instructions.count;
        }
    }
    p.instructions.swap(instructions); p.operands.swap(operands);
    p.blocks.swap(blocks); p.block_order.swap(order);
}
}
