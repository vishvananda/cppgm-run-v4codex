#include "lowir/folding.h"
#include "support/id_index.h"
#include <algorithm>
namespace lowir_model {
namespace {
struct SlotFact {
    bool eligible = true;
    unsigned stores = 0, first_store = 0;
};
struct Definition {
    unsigned slot, block, store;
    Range inputs;
    ValueId value;
    bool unknown = false, reaches_store = false;
    Definition(unsigned s, unsigned b, unsigned i) : slot(s), block(b), store(i) {}
};
struct Edge { unsigned block, next; };
// A sparse, load-demanded SSA construction. Each (block, slot) entry is
// published before its predecessor requests, so loops do not recurse. Store
// copies capture mutable operands and perform exactly the original conversion.
class Promotion {
    Program& p;
    const Function& f;
    std::uint64_t& work;
    std::uint64_t budget;
    cppgm::IdIndex slots, blocks, tails, entries;
    std::vector<SlotFact> facts;
    std::vector<unsigned> predecessors, handlers, phi_heads, next_phi;
    std::vector<Edge> edges = std::vector<Edge>(1);
    std::vector<Definition> definitions = std::vector<Definition>(1,Definition(0,0,0));
    std::vector<unsigned> inputs;
    std::vector<std::pair<unsigned,unsigned>> loads;
    std::vector<unsigned> store_nodes;
    unsigned entry_end = 0, first_registration = 0;
    std::uint64_t input_instructions = 0, input_operands = 0;
    bool exhausted = false;
    std::uint64_t key(unsigned block, unsigned slot) const { return (std::uint64_t(block)<<32)|slot; }
    bool charge(unsigned n = 1) {
        if (n > budget) { exhausted = true; return false; }
        budget -= n; work += n; return true;
    }
    unsigned incoming(unsigned block, unsigned slot) {
        auto k = key(block,slot);
        if (auto old = entries.get(k)) return old;
        if (!charge()) return 0;
        unsigned n = definitions.size(); definitions.emplace_back(slot,block,0);
        entries.put(k,n); return n;
    }
    void census() {
        for (unsigned n = f.slots.begin; n < f.slots.end(); ++n) slots.put(p.slot_order[n].index,n-f.slots.begin+1);
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) blocks.put(p.block_order[n].index,n-f.blocks.begin+1);
        cppgm::IdIndex distinct;
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
            auto id = p.block_order[n].index; unsigned b = blocks.get(id);
            auto r = p.blocks[id-1].instructions;
            if (b == 1) { entry_end = r.end(); first_registration = r.end(); }
            for (unsigned k = r.begin; k < r.end(); ++k) {
                const auto& i = p.instructions[k]; ++work;
                if ((i.opcode == Opcode::EhTry || i.opcode == Opcode::EhCleanup) && i.operands.count) {
                    handlers[blocks.get(p.operands[i.operands.begin].ref)] = 1;
                    if (b == 1) first_registration = std::min(first_registration,k);
                }
                for (unsigned j = 0; j < i.operands.count; ++j) {
                    auto a = p.operands[i.operands.begin+j]; ++work;
                    if (a.kind != Operand::Slot) continue;
                    auto s = slots.get(a.ref); auto& fact = facts[s];
                    bool load = i.opcode == Opcode::Load && j == 0;
                    bool store = i.opcode == Opcode::Store && j == 1;
                    Type t = p.slots[a.ref-1].type;
                    // Phi's representable scalar types are the transport limit.
                    bool scalar = (t.integer() && t != Type::I128) || t == Type::Ptr || t == Type::F32 || t == Type::F64;
                    fact.eligible &= (load || store) && !i.is_volatile && i.type == t && scalar;
                    if (store) { ++fact.stores; fact.first_store = k+1; }
                }
            }
            const auto& terminal = p.instructions[r.end()-1];
            for (unsigned j = terminal.operands.begin; j < terminal.operands.end(); ++j) {
                auto a = p.operands[j]; if (a.kind != Operand::Label) continue;
                unsigned to = blocks.get(a.ref);
                if (distinct.get(key(b,to))) continue;
                distinct.put(key(b,to),1); edges.push_back({b,predecessors[to]}); predecessors[to] = edges.size()-1;
            }
        }
    }
    void scan() {
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
            unsigned b = n-f.blocks.begin+1;
            auto r = p.blocks[p.block_order[n].index-1].instructions;
            for (unsigned k = r.begin; k < r.end() && !exhausted; ++k) {
                const auto& i = p.instructions[k];
                bool store = i.opcode == Opcode::Store;
                if (!store && i.opcode != Opcode::Load) continue;
                auto a = p.operands[i.operands.begin+unsigned(store)];
                if (a.kind != Operand::Slot) continue;
                unsigned s = slots.get(a.ref); if (!facts[s].eligible) continue;
                if (!charge()) break;
                auto bs = key(b,s);
                if (store) {
                    unsigned d = definitions.size(); definitions.emplace_back(s,b,k+1);
                    tails.put(bs,d); store_nodes.push_back(d);
                } else {
                    unsigned d = tails.get(bs); if (!d) d = incoming(b,s);
                    loads.emplace_back(k,d);
                }
            }
        }
    }
    void resolve() {
        for (unsigned n = 1; n < definitions.size() && !exhausted; ++n) {
            unsigned s = definitions[n].slot, b = definitions[n].block;
            if (definitions[n].store || !facts[s].eligible) continue;
            // No phi may be introduced on an exceptional edge. A single
            // entry store preceding every registration is available there;
            // otherwise keep this slot addressable, without guessing EH state.
            if (handlers[b]) {
                auto& fact = facts[s];
                bool entry_store = fact.stores == 1 && fact.first_store && fact.first_store <= first_registration && !predecessors[1];
                unsigned d = entry_store ? tails.get(key(1,s)) : 0;
                if (d && definitions[d].store <= entry_end) {
                    definitions[n].inputs.begin = inputs.size(); definitions[n].inputs.count = 1; inputs.push_back(d);
                } else fact.eligible = false;
                continue;
            }
            if (!predecessors[b] || b == 1) { definitions[n].unknown = true; continue; }
            unsigned start = inputs.size();
            for (unsigned e = predecessors[b]; e && !exhausted; e = edges[e].next) {
                if (!charge(2)) break;
                unsigned pred = edges[e].block, d = tails.get(key(pred,s));
                if (!d) d = incoming(pred,s);
                inputs.push_back(pred); inputs.push_back(d);
            }
            definitions[n].inputs.begin = start; definitions[n].inputs.count = inputs.size()-start;
            // The adjacency lists are prepended. Publish phi inputs in the
            // stable source order of their predecessor blocks.
            for (unsigned a = start, b = inputs.size(); a+2 < b; a += 2) {
                if (!charge(2)) return;
                b -= 2; std::swap(inputs[a],inputs[b]); std::swap(inputs[a+1],inputs[b+1]);
            }
        }
    }
    void partial_facts() {
        // Missing entry facts invalidate their consumers, not every later
        // definition of the same home. Keep an unresolved load in memory. If
        // such a load may also observe a store, retain the whole home rather
        // than remove a store that still feeds a memory read.
        std::vector<unsigned> heads(definitions.size()), pending;
        std::vector<Edge> users(1);
        for (unsigned n = 1; n < definitions.size(); ++n) {
            auto& d = definitions[n];
            if (!facts[d.slot].eligible) continue;
            d.reaches_store = d.store != 0;
            if (d.unknown || d.reaches_store) pending.push_back(n);
            unsigned stride = handlers[d.block] ? 1 : 2;
            for (unsigned k = d.inputs.begin; k < d.inputs.end(); k += stride) {
                if (!charge()) return;
                unsigned input = inputs[k+stride-1];
                users.push_back({n,heads[input]}); heads[input] = users.size()-1;
            }
        }
        while (!pending.empty()) {
            unsigned n = pending.back(); pending.pop_back();
            for (unsigned e = heads[n]; e; e = users[e].next) {
                if (!charge()) return;
                auto& to = definitions[users[e].block]; const auto& from = definitions[n];
                bool changed = (from.unknown && !to.unknown) || (from.reaches_store && !to.reaches_store);
                to.unknown |= from.unknown; to.reaches_store |= from.reaches_store;
                if (changed) pending.push_back(users[e].block);
            }
        }
        for (auto load : loads) {
            const auto& d = definitions[load.second];
            if (d.unknown && d.reaches_store) facts[d.slot].eligible = false;
        }
    }
    Operand operand(unsigned d) const {
        // Exceptional aliases point directly to the single entry snapshot.
        if (!definitions[d].store && handlers[definitions[d].block]) d = inputs[definitions[d].inputs.begin];
        return Operand::value(definitions[d].value);
    }
public:
    Promotion(Program& p, const Function& f, std::uint64_t& work)
        : p(p), f(f), work(work), budget(0), facts(f.slots.count+1),
          predecessors(f.blocks.count+1), handlers(f.blocks.count+1), phi_heads(f.blocks.count+1) {
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
            auto r = p.blocks[p.block_order[n].index-1].instructions;
            input_instructions += r.count;
            budget += 16*(std::uint64_t(r.count)+1);
            for (unsigned k = r.begin; k < r.end(); ++k) input_operands += p.instructions[k].operands.count;
        }
        budget += 16*input_operands;
    }
    bool run(std::vector<unsigned>& additions, NameIndex& names, unsigned& serial) {
        census(); scan(); resolve(); if (exhausted) return false;
        partial_facts(); if (exhausted) return false;
        std::uint64_t phis = 0, phi_operands = 0;
        for (const auto& d : definitions) if (d.slot && facts[d.slot].eligible && !d.unknown && !d.store && !handlers[d.block]) {
            ++phis; phi_operands += d.inputs.count;
        }
        // Admission limits representation growth independently of analysis
        // work. Stores become copies; only phis can add instructions.
        if (phis > input_instructions || phi_operands > 2*input_operands) return false;
        bool changed = false;
        next_phi.resize(definitions.size());
        for (unsigned n = 1; n < definitions.size(); ++n) {
            auto& d = definitions[n]; if (!facts[d.slot].eligible || d.unknown || (!d.store && handlers[d.block])) continue;
            changed = true;
            Value v; v.owner = p.slots[p.slot_order[f.slots.begin+d.slot-1].index-1].owner;
            v.type = p.slots[p.slot_order[f.slots.begin+d.slot-1].index-1].type;
            do { v.name = p.intern("%opt_slot_"+std::to_string(serial++)); } while (!names.insert(v.name,1));
            p.values.push_back(v); d.value = ValueId(p.values.size());
            if (!d.store) { next_phi[n] = phi_heads[d.block]; phi_heads[d.block] = n; }
        }
        for (unsigned n : store_nodes) {
            auto& d = definitions[n]; if (!facts[d.slot].eligible) continue;
            auto& i = p.instructions[d.store-1];
            i.opcode = Opcode::Copy; i.destination = d.value; i.operands.count = 1;
            auto location = i.debug; i.debug = DebugLocation();
            auto input = p.operands[i.operands.begin];
            Type actual = input.kind == Operand::Temporary ? p.values[input.ref-1].type :
                input.kind == Operand::Slot ? p.slots[input.ref-1].type : i.type;
            // Stores perform floating format conversion; Copy only accepts
            // the same format. Preserve the store's rounding at this snapshot.
            if (i.type.floating() && actual != i.type) {
                i.opcode = Opcode::Convert; i.source_type = actual;
                i.debug = location;
                i.operation = actual.width() < i.type.width() ? Operation::Fpext : Operation::Fptrunc;
            }
        }
        for (auto load : loads) {
            auto& d = definitions[load.second]; if (!facts[d.slot].eligible || d.unknown) continue;
            auto& i = p.instructions[load.first]; i.opcode = Opcode::Copy;
            i.debug = DebugLocation();
            p.operands[i.operands.begin] = operand(load.second);
        }
        for (unsigned b = 1; b < phi_heads.size(); ++b) {
            auto id = p.block_order[f.blocks.begin+b-1];
            for (unsigned n = phi_heads[b]; n; n = next_phi[n]) {
                auto& d = definitions[n]; Instruction i(Opcode::Phi,p.values[d.value.index-1].type);
                i.destination = d.value; i.operands.begin = p.operands.size(); i.operands.count = d.inputs.count;
                i.debug = p.instructions[p.blocks[id.index-1].instructions.begin].debug;
                for (unsigned k = d.inputs.begin; k < d.inputs.end(); k += 2) {
                    p.operands.push_back(Operand::label(p.block_order[f.blocks.begin+inputs[k]-1]));
                    p.operands.push_back(operand(inputs[k+1]));
                }
                // Linked additions index a separate instruction suffix.
                additions.push_back(id.index); p.instructions.push_back(i);
            }
        }
        return changed;
    }
};
}
bool promote_scalar_slots(Program& p, const std::vector<bool>& call_cycles, std::uint64_t& work)
{
    if (p.slots.empty()) return false;
    auto original_size = p.instructions.size();
    NameIndex names;
    for (const auto& v : p.values) if (v.name) names.insert(v.name,1);
    std::vector<unsigned> additions;
    bool changed = false;
    unsigned serial = p.values.size()+1;
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        const auto& f = p.functions[fn];
        if (!f.declaration && f.slots.count && f.blocks.count > 1 && !call_cycles[fn]) changed |= Promotion(p,f,work).run(additions,names,serial);
    }
    if (!changed) return false;
    // Rebuild block slices once, preserving block identity/source order. All
    // transient demand records have already died at their function boundary.
    std::vector<unsigned> heads(p.blocks.size()+1), next(additions.size());
    for (unsigned n = additions.size(); n; --n) { next[n-1] = heads[additions[n-1]]; heads[additions[n-1]] = n; }
    Pool<Instruction> instructions; instructions.reserve(p.instructions.size());
    for (auto id : p.block_order) {
        auto& b = p.blocks[id.index-1]; auto old = b.instructions; b.instructions.begin = instructions.size();
        for (unsigned n = heads[id.index]; n; n = next[n-1]) instructions.push_back(p.instructions[original_size+n-1]);
        for (unsigned n = old.begin; n < old.end(); ++n) instructions.push_back(p.instructions[n]);
        b.instructions.count = instructions.size()-b.instructions.begin;
    }
    p.instructions.swap(instructions);
    for (auto& v : p.values) { v.definition = 0; v.defined = false; }
    for (auto a : p.parameters) p.values[a.value.index-1].defined = true;
    for (unsigned n = 0; n < p.instructions.size(); ++n) if (p.instructions[n].destination) {
        auto& v = p.values[p.instructions[n].destination.index-1];
        if (!v.defined) v.definition = n+1;
        v.defined = true;
    }
    return true;
}
}
