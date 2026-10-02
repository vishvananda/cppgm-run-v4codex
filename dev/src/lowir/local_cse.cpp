#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include <algorithm>
namespace lowir_model {
namespace {
bool less(const Program&, Operand, Operand);
bool commutative_binary(const Instruction& i) {
    return i.opcode == Opcode::Binary && (i.operation == Operation::Add || i.operation == Operation::Mul ||
        i.operation == Operation::And || i.operation == Operation::Or || i.operation == Operation::Xor);
}
std::uint64_t mix(std::uint64_t h, std::uint64_t n) {
    n ^= n>>30; n *= 0xbf58476d1ce4e5b9ULL; n ^= n>>27; n *= 0x94d049bb133111ebULL;
    return (h^(n^(n>>31)))*1099511628211ULL;
}
std::uint64_t key(const Program& p, const Instruction& i) {
    std::uint64_t h = mix(unsigned(i.opcode),unsigned(i.operation));
    h = mix(h,i.type.kind()); h = mix(h,i.type.width()); h = mix(h,i.type.alignment());
    h = mix(h,unsigned(i.projection));
    bool reversed = commutative_binary(i) && i.operands.count == 2 &&
        less(p,p.operands[i.operands.begin+1],p.operands[i.operands.begin]);
    for (unsigned n = 0; n < i.operands.count; ++n) {
        auto a = p.operands[i.operands.begin+(reversed ? 1-n : n)]; h = mix(h,a.kind);
        if (a.kind == Operand::Integer) { h = mix(h,a.data.integer); h = mix(h,a.integer_high()); }
        else h = mix(h,a.ref);
    }
    return h;
}
bool equal(const Program& p, const Instruction& a, const Instruction& b) {
    if (a.opcode != b.opcode || a.operation != b.operation || a.type != b.type ||
        a.source_type != b.source_type || a.projection != b.projection || a.operands.count != b.operands.count) return false;
    if (commutative_binary(a) && a.operands.count == 2 &&
        same_scalar(p.operands[a.operands.begin],p.operands[b.operands.begin+1]) &&
        same_scalar(p.operands[a.operands.begin+1],p.operands[b.operands.begin])) return true;
    for (unsigned n = 0; n < a.operands.count; ++n)
        if (!same_scalar(p.operands[a.operands.begin+n],p.operands[b.operands.begin+n])) return false;
    return true;
}
bool less(const Program& p, Operand a, Operand b) {
    if (a.literal() != b.literal()) return !a.literal();
    if (a.kind == Operand::Temporary && b.kind == Operand::Temporary &&
        p.values[a.ref-1].definition != p.values[b.ref-1].definition)
        return p.values[a.ref-1].definition < p.values[b.ref-1].definition;
    if (a.kind != b.kind) return a.kind < b.kind;
    if (a.kind != Operand::Integer) return a.ref < b.ref;
    return a.integer_high() != b.integer_high() ? a.integer_high() < b.integer_high() : a.data.integer < b.data.integer;
}
void canonicalize(Program& p, Instruction& i, const std::vector<unsigned>& uses) {
    if (i.operands.count != 2) return;
    // Number commutative arithmetic canonically without changing its useful
    // destructive operand order. Repeated load reuse otherwise moves a live
    // invariant ahead of the dying accumulator and adds a move at every add.
    auto left = p.operands[i.operands.begin], right = p.operands[i.operands.begin+1];
    bool dying_accumulator = commutative_binary(i) && left.kind == Operand::Temporary && right.kind == Operand::Temporary &&
        uses[left.ref] == 1 && uses[right.ref] > 1;
    bool commute = (!dying_accumulator && commutative_binary(i)) || i.operation == Operation::Eq || i.operation == Operation::Ne;
    bool swap = false;
    if (i.operation == Operation::Gt) { i.operation = Operation::Lt; swap = true; }
    if (i.operation == Operation::Ge) { i.operation = Operation::Le; swap = true; }
    if (i.operation == Operation::Ugt) { i.operation = Operation::Ult; swap = true; }
    if (i.operation == Operation::Uge) { i.operation = Operation::Ule; swap = true; }
    auto& a = p.operands[i.operands.begin]; auto& b = p.operands[i.operands.begin+1];
    if (swap || (commute && less(p,b,a))) std::swap(a,b);
}
}
namespace {
struct SavedBucket { unsigned bucket, value; };
void expressions(Program& p, BlockId block, const std::vector<unsigned>& definitions,
    const std::vector<unsigned>& uses,
    std::vector<unsigned>& table, std::vector<SavedBucket>& undo, std::uint64_t& budget, std::uint64_t& work)
{
    auto r = p.blocks[block.index-1].instructions;
    for (unsigned n = r.begin; n < r.end() && budget; ++n) {
        auto& i = p.instructions[n]; ++work; --budget;
        bool eligible = i.opcode == Opcode::Addr || i.opcode == Opcode::Index ||
            ((i.opcode == Opcode::Binary || i.opcode == Opcode::Unary || i.opcode == Opcode::Compare || i.opcode == Opcode::Convert) && discardable(i));
        if (!eligible || definitions[i.destination.index] != 1) continue;
        for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) {
            auto a = p.operands[j];
            eligible &= a.kind != Operand::Floating && (a.kind != Operand::Temporary || definitions[a.ref] == 1);
        }
        if (!eligible) continue;
        canonicalize(p,i,uses); unsigned bucket = key(p,i)&(table.size()-1);
        while (table[bucket] && budget) {
            --budget; ++work;
            const auto& previous = p.instructions[table[bucket]-1];
            if (equal(p,i,previous) && p.values[previous.destination.index-1].definition < p.values[i.destination.index-1].definition) {
                p.operands[i.operands.begin] = Operand::value(previous.destination);
                i.type = i.result_type(); i.opcode = Opcode::Copy; i.source_type = Type();
                i.debug = DebugLocation();
                i.operation = Operation::None; i.operands.count = 1; break;
            }
            bucket = (bucket+1)&(table.size()-1);
        }
        if (!table[bucket]) { undo.push_back({bucket,0}); table[bucket] = n+1; }
    }
}
void order_definitions(Program& p, Function& f, const OrdinaryFlow& flow,
    const std::vector<unsigned>& definitions, std::uint64_t& work)
{
    // RPO serializes each dominating definition before its ordinary uses.
    // LowIR permits mutable and merely source-ordered temporaries too: keep
    // their original schedule unless this SSA dominance proof succeeds.
    cppgm::IdIndex owners;
    for (unsigned b = 1; b < flow.blocks.size(); ++b) {
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            auto v = p.instructions[n].destination; ++work;
            if (!v) continue;
            if (definitions[v.index] != 1) return;
            owners.put(v.index,b);
        }
    }
    for (unsigned b : flow.rpo) {
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            const auto& i = p.instructions[n]; if (i.opcode == Opcode::Phi) continue;
            for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                auto a = p.operands[k]; ++work;
                if (a.kind != Operand::Temporary) continue;
                unsigned owner = owners.get(a.ref);
                if (definitions[a.ref] != 1 || (owner && !flow.dominates(owner,b))) return;
            }
        }
    }
    unsigned next = f.blocks.begin;
    for (unsigned b : flow.rpo) p.block_order[next++] = flow.blocks[b];
    for (unsigned b = 1; b < flow.blocks.size(); ++b) if (!flow.enter[b]) p.block_order[next++] = flow.blocks[b];
    unsigned ordinal = 0;
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
        auto r = p.blocks[p.block_order[n].index-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) {
            auto v = p.instructions[k].destination; ++ordinal;
            if (v) p.values[v.index-1].definition = ordinal;
        }
    }
}
}
void eliminate_local_expressions(Program& p, const std::vector<bool>& call_cycles, std::uint64_t& work,
    bool edges, const std::vector<bool>* selected)
{
    std::vector<unsigned> definitions(p.values.size()+1);
    std::vector<unsigned> uses(p.values.size()+1);
    for (const auto& a : p.parameters) ++definitions[a.value.index];
    for (const auto& i : p.instructions) if (i.destination) ++definitions[i.destination.index];
    for (const auto& a : p.operands) { if (a.kind == Operand::Temporary) ++uses[a.ref]; ++work; }
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        auto& f = p.functions[fn]; if (f.declaration) continue;
        if (selected && !(*selected)[fn]) continue;
        bool call_cycle = call_cycles[fn];
        if (edges && call_cycle) continue;
        if (f.blocks.count == 1) {
            auto b = p.block_order[f.blocks.begin]; auto r = p.blocks[b.index-1].instructions;
            unsigned size = 8; while (size < 2*r.count) size *= 2;
            std::vector<unsigned> table(size); std::vector<SavedBucket> undo;
            std::uint64_t budget = 16*(std::uint64_t(r.count)+1);
            expressions(p,b,definitions,uses,table,undo,budget,work); continue;
        }
        OrdinaryFlow flow(p,f,work);
        if (!call_cycle && flow.dominance(work)) {
            order_definitions(p,f,flow,definitions,work);
            unsigned size = 8; while (size < 2*flow.size) size *= 2;
            std::vector<unsigned> table(size); std::vector<SavedBucket> undo;
            std::uint64_t budget = 16*(flow.size+1);
            struct Scope { unsigned child, mark; };
            std::vector<Scope> scopes;
            expressions(p,flow.blocks[1],definitions,uses,table,undo,budget,work);
            scopes.push_back({flow.first_child[1],0});
            while (!scopes.empty()) {
                auto& scope = scopes.back();
                if (scope.child) {
                    unsigned child = scope.child; scope.child = flow.next_child[child];
                    unsigned mark = undo.size();
                    expressions(p,flow.blocks[child],definitions,uses,table,undo,budget,work);
                    scopes.push_back({flow.first_child[child],mark});
                } else {
                    while (undo.size() > scope.mark) { auto saved = undo.back(); undo.pop_back(); table[saved.bucket] = saved.value; }
                    scopes.pop_back();
                }
            }
            if (edges) propagate_edge_facts(p,flow,definitions,work);
        } else {
            for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
                auto b = p.block_order[n]; auto r = p.blocks[b.index-1].instructions;
                unsigned size = 8; while (size < 2*r.count) size *= 2;
                std::vector<unsigned> table(size); std::vector<SavedBucket> undo;
                std::uint64_t budget = 16*(std::uint64_t(r.count)+1);
                expressions(p,b,definitions,uses,table,undo,budget,work);
            }
        }
    }
}
}
