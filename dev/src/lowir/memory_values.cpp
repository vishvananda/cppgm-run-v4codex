#include "lowir/memory_values.h"
#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include "lowir/call_effects.h"
#include <algorithm>
namespace lowir_model {
namespace {
// Identity and modular byte offsets are distinct from within-object evidence.
// Plain index supplies equality, never a disjoint-object/provenance promise.
struct Address {
    Operand::Kind kind = Operand::Integer;
    unsigned root = 0;
    std::uint64_t offset = 0;
    bool within = false, noalias = false, readonly = false, unique_global = false;
};
bool equal(Address a, Address b) { return a.root && a.kind == b.kind && a.root == b.root && a.offset == b.offset; }
bool disjoint(Address a, std::uint64_t an, Address b, std::uint64_t bn) {
    if (!a.root || !b.root) return false;
    if (a.kind == b.kind && a.root == b.root)
        return a.offset-b.offset >= bn && b.offset-a.offset >= an;
    if (!a.within || !b.within) return false;
    if (a.kind == Operand::Slot && b.kind == Operand::Slot) return true;
    if ((a.kind == Operand::Slot && b.kind == Operand::Symbol) ||
        (b.kind == Operand::Slot && a.kind == Operand::Symbol)) return true;
    if (a.kind == Operand::Symbol && b.kind == Operand::Symbol) return a.unique_global && b.unique_global;
    return a.noalias && b.noalias;
}
struct ValueFact { unsigned definitions = 0, producer = 0, uses = 0, epoch = 0; Address address; };
struct Cell { Address address; Type type; Operand value; };
struct State { Range cells; unsigned region = 0, epoch = 0; bool done = false; };
class Memory {
    Program& p;
    const Function& f;
    std::uint64_t& work;
    MemoryStats& stats;
    OrdinaryFlow flow;
    cppgm::IdIndex ids;
    std::vector<ValueFact> values = std::vector<ValueFact>(1);
    std::vector<State> states;
    std::vector<Cell> saved, cells;
    std::vector<bool> handlers;
    std::uint64_t budget;
    bool changed = false;
    unsigned phis = 0;
    unsigned local(unsigned v) {
        if (auto id = ids.get(v)) return id;
        unsigned id = values.size(); ids.put(v,id); values.emplace_back(); return id;
    }
    bool charge() { if (!budget) return false; --budget; ++work; return true; }
    bool stable(Operand a) const {
        return a.kind != Operand::Temporary || values[ids.get(a.ref)].definitions == 1;
    }
    Address address(Operand a) const {
        if (a.kind == Operand::Temporary) return values[ids.get(a.ref)].address;
        Address result;
        if (a.kind == Operand::Slot || (a.kind == Operand::Symbol && p.symbols[a.ref-1].kind == Symbol::GlobalSymbol)) {
            result.kind = a.kind; result.root = a.ref; result.within = true;
            result.readonly = a.kind == Operand::Symbol && p.symbols[a.ref-1].metadata.storage == GSM_READONLY;
            if (a.kind == Operand::Symbol) {
                const auto& s = p.symbols[a.ref-1];
                // Imported or explicitly object-named symbols can denote the
                // same ELF storage under different LowIR declaration IDs.
                result.unique_global = !p.globals[s.entity-1].declaration && !s.metadata.object && s.metadata.binding != SBM_WEAK;
            }
        }
        return result;
    }
    bool available(Operand a, const Instruction& use) const {
        return stable(a) && (a.kind != Operand::Temporary ||
            p.values[a.ref-1].definition < p.values[use.destination.index-1].definition);
    }
    void census() {
        auto params = p.signatures[f.signature.index-1].parameters;
        for (unsigned k = params.begin; k < params.end(); ++k) ++values[local(p.parameters[k].value.index)].definitions;
        for (unsigned b = 1; b < flow.blocks.size(); ++b) {
            auto r = p.blocks[flow.blocks[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                phis += i.opcode == Opcode::Phi;
                if (i.destination) { auto& v = values[local(i.destination.index)]; ++v.definitions; v.producer = n+1; }
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto a = p.operands[k]; ++work;
                    if (a.kind == Operand::Temporary) ++values[local(a.ref)].uses;
                    if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && a.kind == Operand::Label)
                        handlers[flow.local.get(a.ref)] = true;
                }
            }
        }
        // Unknown but immutable pointer values still have exact identity.
        for (unsigned k = params.begin; k < params.end(); ++k) {
            const auto& param = p.parameters[k]; auto& v = values[ids.get(param.value.index)];
            if (param.type != Type::Ptr || v.definitions != 1) continue;
            v.address.kind = Operand::Temporary; v.address.root = param.value.index;
            v.address.within = true; v.address.noalias = param.alias == PALM_NOALIAS;
        }
        for (unsigned b = 1; b < flow.blocks.size(); ++b) {
            auto r = p.blocks[flow.blocks[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                if (!i.destination || i.result_type() != Type::Ptr) continue;
                auto& v = values[ids.get(i.destination.index)]; if (v.definitions != 1) continue;
                v.address.kind = Operand::Temporary; v.address.root = i.destination.index;
                if (i.opcode != Opcode::Copy && i.opcode != Opcode::Addr && i.opcode != Opcode::Index) continue;
                auto a = address(p.operands[i.operands.begin]); if (!a.root) continue;
                if (i.opcode == Opcode::Index) {
                    auto offset = p.operands[i.operands.begin+1];
                    if (offset.kind != Operand::Integer) continue;
                    std::uint64_t bytes = offset.data.integer * i.type.bytes();
                    a.offset += bytes;
                    a.within &= !bytes || i.projection == IPK_FIELD;
                    a.readonly &= a.within;
                }
                v.address = a;
            }
        }
    }
    unsigned find(const std::vector<Cell>& pool, Range r, Address a, Type type) {
        for (unsigned k = r.begin; k < r.end(); ++k) {
            if (!charge()) return 0;
            if (pool[k].type == type && equal(pool[k].address,a)) return k+1;
        }
        return 0;
    }
    void remember(Address a, Type type, Operand v) {
        if (!a.root || !stable(v) || !type.scalar()) return;
        Range r; r.count = cells.size(); auto found = find(cells,r,a,type);
        if (found) cells[found-1].value = v;
        else if (budget && cells.size() < 32) cells.push_back({a,type,v});
    }
    void invalidate(Address a, std::uint64_t bytes) {
        unsigned out = 0;
        for (auto c : cells) {
            if (!charge()) { cells.clear(); return; }
            if (disjoint(a,bytes,c.address,c.type.bytes())) cells[out++] = c;
        }
        cells.resize(out);
    }
    void clear_mutable() {
        unsigned out = 0;
        for (auto c : cells) { ++work; if (c.address.readonly) cells[out++] = c; }
        cells.resize(out);
    }
    void coalesce() {
        struct Piece { unsigned load, store; };
        std::vector<Piece> pieces; pieces.reserve(16);
        for (unsigned b = 1; b < flow.blocks.size(); ++b) {
            auto r = p.blocks[flow.blocks[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end() && budget; ++n) {
                pieces.clear(); unsigned cursor = n; std::uint64_t bytes = 0;
                Address source, destination;
                while (cursor < r.end() && pieces.size() < 16 && charge()) {
                    const auto& load = p.instructions[cursor];
                    if (load.opcode != Opcode::Load || load.is_volatile || !load.type.scalar() ||
                        load.type == Type::I1 || load.type.floating() ||
                        values[ids.get(load.destination.index)].uses != 1 || !stable(Operand::value(load.destination))) break;
                    auto a = address(p.operands[load.operands.begin]);
                    unsigned store = cursor+1;
                    while (store < r.end() && charge()) {
                        auto op = p.instructions[store].opcode;
                        if (op != Opcode::Index && op != Opcode::Addr && op != Opcode::Copy && op != Opcode::Nop) break;
                        // A mutable pointer/value update changes the snapshot.
                        auto v = p.instructions[store].destination;
                        if (v && !stable(Operand::value(v))) break;
                        ++store;
                    }
                    if (store == r.end() || !budget) break;
                    const auto& put = p.instructions[store];
                    if (put.opcode != Opcode::Store || put.is_volatile || put.type != load.type ||
                        !same_scalar(p.operands[put.operands.begin],Operand::value(load.destination))) break;
                    auto d = address(p.operands[put.operands.begin+1]);
                    if (pieces.empty()) { source = a; destination = d; }
                    auto expected_source = source, expected_destination = destination;
                    expected_source.offset += bytes; expected_destination.offset += bytes;
                    if (!equal(a,expected_source) || !equal(d,expected_destination) ||
                        !a.within || !d.within || bytes+load.type.bytes() > 128 ||
                        !disjoint(source,bytes+load.type.bytes(),destination,bytes+load.type.bytes())) break;
                    bytes += load.type.bytes(); pieces.push_back({cursor,store}); cursor = store+1;
                    while (cursor < r.end() && charge()) {
                        const auto& i = p.instructions[cursor]; auto op = i.opcode;
                        if (op != Opcode::Index && op != Opcode::Addr && op != Opcode::Copy && op != Opcode::Nop) break;
                        if (i.destination && !stable(Operand::value(i.destination))) break;
                        ++cursor;
                    }
                }
                if (pieces.size() < 2 || !budget) continue;
                auto src = p.operands[p.instructions[pieces[0].load].operands.begin];
                auto& copy = p.instructions[pieces[0].store];
                copy.opcode = Opcode::CopyObject; copy.type = Type(); copy.bytes = bytes; copy.alignment = 1;
                p.operands[copy.operands.begin] = src;
                for (auto piece : pieces) {
                    auto& load = p.instructions[piece.load];
                    load.opcode = Opcode::Nop; load.destination = ValueId(); load.type = Type(); load.operands.count = 0;
                    if (piece.store == pieces[0].store) continue;
                    auto& put = p.instructions[piece.store]; put.opcode = Opcode::Nop; put.type = Type(); put.operands.count = 0;
                }
                n = pieces.back().store; changed = true; ++stats.copies;
            }
        }
    }
    void conditional(Instruction& load, unsigned b, bool clean) {
        if (!clean || handlers[b] || b == 1) return;
        auto a = p.operands[load.operands.begin];
        if (a.kind != Operand::Temporary) return;
        const auto& fact = values[ids.get(a.ref)];
        if (fact.definitions != 1 || fact.uses != 1 || !fact.producer) return;
        auto& phi = p.instructions[fact.producer-1];
        auto block = p.blocks[flow.blocks[b].index-1].instructions;
        if (fact.producer-1 < block.begin || fact.producer-1 >= block.end() || phi.opcode != Opcode::Phi) return;
        std::vector<Operand> incoming;
        for (unsigned k = phi.operands.begin; k < phi.operands.end(); k += 2) {
            if (!charge()) return;
            unsigned from = flow.local.get(p.operands[k].ref);
            if (!states[from].done) return;
            auto s = states[from];
            auto origin = address(p.operands[k+1]);
            // Replacing a private-home choice also removes its materialized
            // addresses and stores. For external fields the value phi instead
            // lengthens loaded-value live ranges; current native placement
            // spills them. Keep the late external load until a cheaper lowering
            // exists (measured policy, separate from the legality proof).
            if (origin.kind != Operand::Slot) return;
            unsigned found = find(saved,s.cells,origin,load.type);
            if (!found || !available(saved[found-1].value,load)) return;
            auto value = saved[found-1].value;
            // Incoming scalars/literals already have stable ABI carriers.
            // Extending an instruction result across the arms creates new
            // spill traffic in the current backend, even for private homes.
            if (value.kind == Operand::Temporary && values[ids.get(value.ref)].producer) return;
            incoming.push_back(saved[found-1].value);
        }
        // Change the address phi into a value phi in place: no inserted loads,
        // no speculative accesses and no extra phi/operand growth.
        for (unsigned k = 0; k < incoming.size(); ++k) p.operands[phi.operands.begin+2*k+1] = incoming[k];
        phi.type = load.type; phi.destination = load.destination; phi.debug = load.debug;
        load.opcode = Opcode::Nop; load.destination = ValueId(); load.type = Type(); load.operands.count = 0;
        changed = true; ++stats.conditional;
    }
    void visit(unsigned b) {
        cells.clear(); unsigned region = 0; bool initialized = false;
        unsigned epoch = b == 1 ? 1 : p.instructions.size()+b+2;
        if (b != 1 && !handlers[b]) for (unsigned e = flow.incoming[b]; e; e = flow.edges[e].next_in) {
            if (!charge()) { cells.clear(); break; }
            auto s = states[flow.edges[e].from];
            if (!s.done) { cells.clear(); initialized = true; region = 0; epoch = p.instructions.size()+b+2; break; }
            if (!initialized) {
                for (unsigned k = s.cells.begin; k < s.cells.end(); ++k) { if (!charge()) break; cells.push_back(saved[k]); }
                region = s.region; initialized = true;
                epoch = s.epoch;
            } else if (region != s.region) cells.clear();
            else {
                unsigned out = 0;
                for (auto c : cells) {
                    unsigned found = find(saved,s.cells,c.address,c.type);
                    if (found && same_scalar(saved[found-1].value,c.value)) cells[out++] = c;
                }
                cells.resize(out);
            }
            if (epoch != s.epoch) epoch = p.instructions.size()+b+2;
        }
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        bool clean = true;
        for (unsigned n = r.begin; n < r.end() && charge(); ++n) {
            auto& i = p.instructions[n];
            if (i.is_volatile || (i.opcode >= Opcode::AtomicLoad && i.opcode <= Opcode::AtomicSignalFence) ||
                (i.opcode >= Opcode::EhTry && i.opcode <= Opcode::Resume) || i.opcode == Opcode::X86 ||
                i.opcode == Opcode::VaStart || i.opcode == Opcode::VaArg) {
                cells.clear(); region = n+1; epoch = n+2; clean = false; continue;
            }
            if (i.opcode == Opcode::Call) {
                auto boundary = call_boundary(p,i);
                if (boundary.unwind != CUM_NO) { clear_mutable(); region = n+1; }
                else if (boundary.effects != CFXM_READNONE && boundary.effects != CFXM_READONLY) clear_mutable();
                if (boundary.unwind != CUM_NO || (boundary.effects != CFXM_READNONE && boundary.effects != CFXM_READONLY)) epoch = n+2;
                clean = false;
            } else if (i.opcode == Opcode::Store) {
                auto a = address(p.operands[i.operands.begin+1]); auto v = p.operands[i.operands.begin];
                invalidate(a,i.type.bytes()); clean = false; epoch = n+2;
                if (v.kind == Operand::Integer && i.type.integer()) v = normalize_integer(v,i.type);
                bool exact = (v.kind == Operand::Integer && i.type.integer()) || (v.kind == Operand::Null && i.type == Type::Ptr) ||
                    (v.kind == Operand::Symbol && i.type == Type::Ptr) ||
                    (v.kind == Operand::Temporary && p.values[v.ref-1].type == i.type);
                if (exact) remember(a,i.type,v);
            } else if (i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) {
                invalidate(address(p.operands[i.operands.begin+(i.opcode == Opcode::CopyObject)]),i.bytes); clean = false; epoch = n+2;
            } else if (i.opcode == Opcode::Load && i.type.scalar() && stable(Operand::value(i.destination))) {
                values[ids.get(i.destination.index)].epoch = epoch;
                auto a = address(p.operands[i.operands.begin]); Range all; all.count = cells.size();
                auto found = find(cells,all,a,i.type);
                if (found && available(cells[found-1].value,i)) {
                    i.opcode = Opcode::Copy; p.operands[i.operands.begin] = cells[found-1].value;
                    changed = true; ++stats.reused;
                } else {
                    conditional(i,b,clean);
                    if (i.opcode == Opcode::Load) remember(a,i.type,Operand::value(i.destination));
                }
            }
        }
        auto& s = states[b]; s.done = true; s.region = region; s.epoch = epoch; s.cells.begin = saved.size();
        // Snapshot storage and every candidate lookup share the function's
        // allowance. Exhaustion publishes no incomplete state or partial phi.
        if (budget < cells.size()) { budget = 0; cells.clear(); }
        for (auto c : cells) { charge(); saved.push_back(c); ++s.cells.count; }
    }
    bool equivalent(Operand a, Operand b, std::vector<unsigned>& redundant) {
        struct Pair { Operand first, second; bool dead; };
        std::vector<Pair> pending; pending.push_back({a,b,true});
        unsigned steps = 0;
        while (!pending.empty()) {
            if (++steps > 64 || !charge()) return false;
            auto pair = pending.back(); pending.pop_back(); a = pair.first; b = pair.second;
            if (!stable(a) || !stable(b)) return false;
            if (same_scalar(a,b)) continue;
            if (a.kind != Operand::Temporary || b.kind != Operand::Temporary || p.values[a.ref-1].type != p.values[b.ref-1].type) return false;
            auto av = values[ids.get(a.ref)], bv = values[ids.get(b.ref)];
            bool dead = pair.dead && bv.uses == 1;
            if (!av.producer || !bv.producer) return false;
            const auto& ai = p.instructions[av.producer-1]; const auto& bi = p.instructions[bv.producer-1];
            if (ai.destination.index != a.ref || bi.destination.index != b.ref) return false;
            auto copy = [&](const Instruction& i) {
                if (i.opcode != Opcode::Copy) return false;
                auto input = p.operands[i.operands.begin];
                return input.kind == Operand::Temporary && stable(input) && p.values[input.ref-1].type == i.type;
            };
            if (copy(ai)) { pending.push_back({p.operands[ai.operands.begin],b,pair.dead}); continue; }
            if (copy(bi)) { pending.push_back({a,p.operands[bi.operands.begin],dead}); continue; }
            if (ai.opcode != bi.opcode || ai.type != bi.type || ai.source_type != bi.source_type ||
                ai.operation != bi.operation || ai.operands.count != bi.operands.count || ai.projection != bi.projection) return false;
            if (ai.opcode == Opcode::Load) {
                if (ai.is_volatile || bi.is_volatile || !av.epoch || av.epoch != bv.epoch ||
                    !equal(address(p.operands[ai.operands.begin]),address(p.operands[bi.operands.begin]))) return false;
                if (dead) redundant.push_back(bv.producer-1);
                continue;
            }
            if (!discardable(ai) || ai.opcode == Opcode::Phi) return false;
            for (unsigned k = 0; k < ai.operands.count; ++k)
                pending.push_back({p.operands[ai.operands.begin+k],p.operands[bi.operands.begin+k],dead});
        }
        return true;
    }
    void diamonds() {
        // Number a bounded window of closed, pure diamonds by their condition
        // and arm values. A completed dominating phi is the representative;
        // values from only one arm are never exported speculatively.
        if (phis < 2 || flow.exceptional || !flow.dominance(work)) return;
        struct Diamond { unsigned block, phi; Operand condition, yes, no; };
        std::vector<Diamond> recent; recent.reserve(16);
        for (unsigned b : flow.rpo) {
            if (!charge()) break;
            auto r = p.blocks[flow.blocks[b].index-1].instructions;
            auto& phi = p.instructions[r.begin];
            if (phi.opcode != Opcode::Phi || phi.operands.count != 4 || !stable(Operand::value(phi.destination))) continue;
            unsigned left = flow.local.get(p.operands[phi.operands.begin].ref), right = flow.local.get(p.operands[phi.operands.begin+2].ref);
            if (left == right || left == b || right == b) continue;
            unsigned le = flow.incoming[left], re = flow.incoming[right];
            if (!le || !re || flow.edges[le].next_in || flow.edges[re].next_in || flow.edges[le].from != flow.edges[re].from) continue;
            unsigned from = flow.edges[le].from;
            auto& branch = p.instructions[p.blocks[flow.blocks[from].index-1].instructions.end()-1];
            if (branch.opcode != Opcode::Branch || from == b) continue;
            bool pure = true;
            for (unsigned arm : {left,right}) {
                auto body = p.blocks[flow.blocks[arm].index-1].instructions;
                for (unsigned n = body.begin; n+1 < body.end(); ++n) {
                    if (!charge()) { pure = false; break; }
                    const auto& i = p.instructions[n];
                    pure &= discardable(i) || i.opcode == Opcode::Nop || (i.opcode == Opcode::Load && !i.is_volatile);
                }
                const auto& jump = p.instructions[body.end()-1];
                pure &= jump.opcode == Opcode::Jump && p.operands[jump.operands.begin].ref == flow.blocks[b].index;
            }
            if (!pure) continue;
            bool left_true = p.operands[branch.operands.begin+1].ref == flow.blocks[left].index;
            Diamond d{b,r.begin,p.operands[branch.operands.begin],p.operands[phi.operands.begin+(left_true ? 1 : 3)],p.operands[phi.operands.begin+(left_true ? 3 : 1)]};
            bool replaced = false;
            for (const auto& old : recent) {
                if (!charge()) break;
                if (!flow.dominates(old.block,b)) continue;
                const auto& previous = p.instructions[old.phi];
                if (previous.type != phi.type || !available(Operand::value(previous.destination),phi)) continue;
                std::vector<unsigned> redundant;
                if (!equivalent(old.condition,d.condition,redundant) || !equivalent(old.yes,d.yes,redundant) || !equivalent(old.no,d.no,redundant)) continue;
                phi.opcode = Opcode::Copy; phi.operands.count = 1;
                p.operands[phi.operands.begin] = Operand::value(previous.destination);
                p.operands[branch.operands.begin] = old.condition;
                for (unsigned n : redundant) {
                    auto& i = p.instructions[n]; i.opcode = Opcode::Nop; i.type = Type(); i.destination = ValueId(); i.operands.count = 0;
                }
                changed = replaced = true; ++stats.diamonds; break;
            }
            if (!replaced) { if (recent.size() == 16) recent.erase(recent.begin()); recent.push_back(d); }
        }
    }
public:
    Memory(Program& p, const Function& f, std::uint64_t& work, MemoryStats& stats)
        : p(p), f(f), work(work), stats(stats), flow(p,f,work), states(flow.blocks.size()), handlers(flow.blocks.size()),
          budget(128*(flow.size+flow.edges.size()+1)) { cells.reserve(32); }
    bool run() {
        census();
        coalesce();
        // Reverse postorder over ordinary edges. Handler roots start empty;
        // a backedge not visited yet also resets facts. There is no fixed point.
        struct Visit { unsigned block, edge; };
        std::vector<Visit> stack; std::vector<bool> seen(flow.blocks.size()); std::vector<unsigned> order;
        auto walk = [&](unsigned root) {
            stack.push_back({root,flow.outgoing[root]}); seen[root] = true;
            unsigned begin = order.size();
            while (!stack.empty()) {
                auto& v = stack.back(); ++work;
                if (!v.edge) { order.push_back(v.block); stack.pop_back(); continue; }
                unsigned to = flow.edges[v.edge].to; v.edge = flow.edges[v.edge].next_out;
                if (!seen[to]) { seen[to] = true; stack.push_back({to,flow.outgoing[to]}); }
            }
            std::reverse(order.begin()+begin,order.end());
        };
        walk(1);
        for (unsigned b = 1; b < flow.blocks.size(); ++b) if (!seen[b]) walk(b);
        for (auto b : order) visit(b);
        diamonds();
        stats.snapshots += saved.size();
        stats.peak_snapshots = std::max(stats.peak_snapshots,std::uint64_t(saved.size()));
        if (!budget) ++stats.declined;
        return changed;
    }
};
}
bool simplify_memory_values(Program& p, std::uint64_t& work, MemoryStats& stats) {
    bool changed = false;
    for (const auto& f : p.functions) if (!f.declaration) {
        bool loads = false;
        for (unsigned b = f.blocks.begin; b < f.blocks.end() && !loads; ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) { ++work; if (p.instructions[n].opcode == Opcode::Load) { loads = true; break; } }
        }
        if (!loads) continue;
        Memory memory(p,f,work,stats); changed |= memory.run();
    }
    return changed;
}
}
