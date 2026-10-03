#include "native/selection.h"
#include <algorithm>
namespace native {
void Selector::retain_global_values(bool handlers)
{
    // EH restoration is a separate placement boundary; retain its stack form.
    if (handlers) return;
    std::vector<unsigned> candidates;
    unsigned effects = 0;
    for (unsigned b = source.blocks.begin; b < source.blocks.end(); ++b) {
        const auto& block = p.blocks[p.block_order[b].index-1];
        for (unsigned n = block.instructions.begin; n < block.instructions.end(); ++n)
            effects |= clobbers(p.instructions[n]);
    }
    for (unsigned id : definitions) {
        const auto& v = state(id);
        const auto& i = p.instructions[v.definition-1];
        if (v.writes != 1 || v.alias || !v.uses || v.folded_load || v.folded_index || v.compare_branch ||
            v.location.kind != Operand::None || !scalar_integer(p.values[id-1].type)) continue;
        if (i.opcode == lowir_model::Opcode::Addr || i.opcode == lowir_model::Opcode::Const) continue;
        if (v.crosses_block || i.opcode == lowir_model::Opcode::Phi) candidates.push_back(id);
    }
    stats.global_candidates += candidates.size();
    if (candidates.empty()) return;
    // At most seven registers, no eviction/retry or IR growth. Prefer repeated
    // consumers; the stable local value identity resolves equal scores.
    std::stable_sort(candidates.begin(),candidates.end(),[&](unsigned a,unsigned b) {
        return state(a).uses > state(b).uses;
    });
    struct Interval { unsigned begin, end; };
    std::vector<Interval> backward;
    std::array<std::vector<Interval>,16> occupied;
    unsigned reserved = 0;
    for (unsigned r = 0; r < 16; ++r) if (live_until[r]) reserved |= 1u<<r;
    std::uint64_t budget = 32*(std::uint64_t(candidates.size())+source.blocks.count+1);
    for (unsigned b = source.blocks.begin; b < source.blocks.end(); ++b) {
        auto range = p.blocks[p.block_order[b].index-1].instructions;
        budget += 32*std::uint64_t(range.count);
        const auto& terminal = p.instructions[range.end()-1];
        for (unsigned n = terminal.operands.begin; n < terminal.operands.end(); ++n) {
            const auto& a = p.operands[n];
            if (a.kind != lowir_model::Operand::Label) continue;
            auto begin = p.blocks[a.ref-1].instructions.begin+1;
            if (begin <= range.end()) backward.push_back({begin,range.end()});
        }
    }
    auto interval = [&](unsigned id) {
        if (!budget) return Interval{0,~0u};
        const auto& v = state(id);
        Interval result{v.definition,std::max(v.definition,v.last)};
        const auto& definition = p.instructions[v.definition-1];
        // Phi destinations are written on predecessor edges, before the
        // destination block. Include those simultaneous transfer positions.
        if (definition.opcode == lowir_model::Opcode::Phi)
            for (unsigned n = definition.operands.begin; n < definition.operands.end(); n += 2) {
                auto edge = p.blocks[p.operands[n].ref-1].instructions.end();
                result.begin = std::min(result.begin,edge);
                result.end = std::max(result.end,edge);
            }
        bool changed = true;
        while (changed) {
            changed = false;
            for (auto edge : backward) {
                if (!budget) return Interval{0,~0u};
                --budget;
                if (edge.end < result.begin || result.end < edge.begin) continue;
                auto begin = std::min(result.begin,edge.begin);
                auto end = std::max(result.end,edge.end);
                changed |= begin != result.begin || end != result.end;
                result = {begin,end};
            }
        }
        return result;
    };
    unsigned parameters = p.signatures[source.signature.index-1].parameters.count;
    for (unsigned id : candidates) {
        auto lifetime = interval(id);
        for (int r : {XR_R8,XR_R9,XR_RBX,XR_R12,XR_R13,XR_R14,XR_R15}) {
            if ((reserved | effects) & (1u<<r)) continue;
            // Incoming parameters have not been moved yet. Even unused ABI
            // carriers are excluded here, including aggregate/hidden inputs.
            if (r < XR_R12 && r != XR_RBX && (parameters || f.result.kind() == Type::Object)) continue;
            bool overlaps = false;
            for (auto prior : occupied[r]) {
                if (!budget) { overlaps = true; break; }
                --budget;
                if (prior.begin <= lifetime.end && lifetime.begin <= prior.end) { overlaps = true; break; }
            }
            if (overlaps) continue;
            occupied[r].push_back(lifetime);
            // Ordinary temporaries keep using the existing reservation rule;
            // only the proven disjoint global candidates share this carrier.
            live_until[r] = ~0u;
            state(id).location = Operand::r(r); ++stats.global_retained;
            if (r == XR_RBX || r >= XR_R12) f.preserved |= 1u<<r;
            break;
        }
    }
}
void Selector::cleanup_control()
{
    // One compaction, preserving all surviving instruction/debug identities.
    // Inverting a branch when its true edge falls through removes its paired
    // jump without moving operations across exception or effect boundaries.
    unsigned out = 0;
    for (unsigned b = 0; b < f.blocks.size(); ++b) {
        auto& block = f.blocks[b];
        auto range = block.instructions;
        unsigned next = b+1 < f.blocks.size() ? f.blocks[b+1].id : 0;
        block.instructions.begin = out;
        for (unsigned n = range.begin; n < range.end(); ++n) {
            auto i = f.instructions[n];
            if (i.op == Op::Jump && i.args[0].id == next) continue;
            if (i.op == Op::Jcc && i.args[0].id == next && n+2 == range.end() &&
                f.instructions[n+1].op == Op::Jump) {
                i.condition = X86Condition(unsigned(i.condition)^1);
                i.args[0] = f.instructions[++n].args[0];
            }
            f.instructions[out++] = i;
        }
        block.instructions.count = out-block.instructions.begin;
    }
    stats.control_removed += f.instructions.size()-out;
    f.instructions.resize(out);
}
} // namespace native
