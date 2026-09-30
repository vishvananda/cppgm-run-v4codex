#include "native/selection.h"
#include <algorithm>
namespace native {
using namespace lowir_model;
void Selector::control_edges()
{
    std::vector<unsigned> targets;
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        unsigned id = p.block_order[b].index;
        if (b+1 != source.blocks.end()) workspace.next_block[id] = p.block_order[b+1].index;
        const auto& block = p.blocks[id-1];
        const auto& term = p.instructions[block.instructions.end()-1];
        targets.clear();
        for (unsigned k = 0; k < term.operands.count; ++k)
            if (arg(term,k).kind == lowir_model::Operand::Label) targets.push_back(arg(term,k).ref);
        std::sort(targets.begin(),targets.end());
        targets.erase(std::unique(targets.begin(),targets.end()),targets.end());
        for (unsigned target : targets) ++workspace.predecessor_count[target];
        if (targets.size() == 1) workspace.successor[id] = targets[0];
    }
}
void Selector::promote_parameters()
{
    // Only a single entry-block initialization from an incoming parameter,
    // followed exclusively by same-type nonvolatile loads, can share its value.
    for (unsigned b = source.blocks.begin; b != source.blocks.end(); ++b) {
        const auto& block = p.blocks[p.block_order[b].index-1];
        for (unsigned n = block.instructions.begin; n != block.instructions.end(); ++n) {
            const auto& i = p.instructions[n];
            for (unsigned k = 0; k < i.operands.count; ++k) {
                auto a = arg(i,k);
                if (a.kind != lowir_model::Operand::Slot) continue;
                auto& fact = workspace.slot_facts[a.ref];
                if (i.opcode != Opcode::Store || k != 1 || i.is_volatile) fact.observed = true;
                if (i.is_volatile || i.type != p.slots[a.ref-1].type) { fact.escape = true; continue; }
                if (i.opcode == Opcode::Load && k == 0) { if (!fact.stored) fact.escape = true; continue; }
                if (i.opcode == Opcode::Store && k == 1 && b == source.blocks.begin && !fact.stored) {
                    auto from = arg(i,0);
                    if (from.kind == lowir_model::Operand::Temporary && !p.values[from.ref-1].definition) {
                        fact.stored = from.ref; fact.position = n+1; continue;
                    }
                }
                fact.escape = true;
            }
        }
    }
}
void Selector::aliases()
{
    for (unsigned id : definitions) {
        auto& v = state(id);
        const auto& i = p.instructions[v.definition-1];
        if (i.opcode == Opcode::Load && arg(i,0).kind == lowir_model::Operand::Slot) {
            const auto& slot = workspace.slot_facts[arg(i,0).ref];
            if (slot.stored && !slot.escape) v.alias = slot.stored;
        }
        if (i.opcode != Opcode::Copy || arg(i,0).kind != lowir_model::Operand::Temporary) continue;
        Type from = value_type(arg(i,0),i.type);
        bool same = from == i.type || (from.width() == 64 && i.type.width() == 64 && scalar_integer(from) && scalar_integer(i.type));
        if (same) v.alias = root(arg(i,0).ref);
    }
}
void Selector::folds()
{
    // These are selection windows, never memory propagation: only the next
    // selected instruction may consume a nonvolatile load/address directly.
    // Carrier intervals are extended through that consumer before placement.
    for (auto at = definitions.rbegin(); at != definitions.rend(); ++at) {
        auto& v = state(*at);
        if (v.alias || !v.uses) continue;
        const auto& definition = p.instructions[v.definition-1];
        if (definition.opcode == Opcode::Index && arg(definition,1).literal() && v.address_only) {
            v.folded_index = true;
            if (arg(definition,0).kind == lowir_model::Operand::Temporary) {
                auto& carrier = state(root(arg(definition,0).ref));
                carrier.last = std::max(carrier.last,v.last);
                carrier.crosses_block |= v.crosses_block;
                carrier.crosses_call |= v.crosses_call;
            }
        }
        if (v.uses != 1 || v.crosses_block || v.last != v.definition+1) continue;
        const auto& i = p.instructions[v.definition-1];
        const auto& next = p.instructions[v.last-1];
        if (i.opcode == Opcode::Load && !i.is_volatile && next.type == i.type && arg(next,next.operands.count-1).kind == lowir_model::Operand::Temporary) {
            bool binary = next.opcode == Opcode::Binary && (next.operation == Operation::Add || next.operation == Operation::Sub ||
                next.operation == Operation::And || next.operation == Operation::Or || next.operation == Operation::Xor ||
                (next.operation == Operation::Mul && i.type.width() >= 16));
            v.folded_load = (binary || next.opcode == Opcode::Compare) && arg(next,1).ref == *at;
        }
        if (i.opcode == Opcode::Index) {
            bool memory_use = (next.opcode == Opcode::Load && arg(next,0).kind == lowir_model::Operand::Temporary && arg(next,0).ref == *at) ||
                (next.opcode == Opcode::Store && arg(next,1).kind == lowir_model::Operand::Temporary && arg(next,1).ref == *at);
            bool index_use = next.opcode == Opcode::Index && arg(next,0).kind == lowir_model::Operand::Temporary && arg(next,0).ref == *at && arg(next,1).literal();
            v.folded_index = memory_use || index_use;
        }
        if (!v.folded_load && !v.folded_index) continue;
        for (unsigned k = 0; k < i.operands.count; ++k) if (arg(i,k).kind == lowir_model::Operand::Temporary) {
            auto& carrier = state(root(arg(i,k).ref));
            carrier.last = std::max(carrier.last,v.last);
        }
    }
}
} // namespace native
