#include "lowir/loop_simplify.h"
#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include <algorithm>
namespace lowir_model {
namespace {
struct Fact { unsigned definitions = 0, instruction = 0, block = 0, users = 0; };
struct Use { unsigned block, point, next; };
struct Loop {
    unsigned header = 0, entry = 0, latch = 0, exit = 0, compare = 0, induction = 0, update = 0;
    bool body_true = false;
    std::uint64_t trips = 0;
    Operand final;
    bool pointer = false, finite_pointer = false, range = false, guarded = false;
    Operand initial, count;
    std::uint64_t stride = 0;
    std::vector<unsigned> blocks, phis, body, exports;
};
Operation swapped(Operation op) {
    switch (op) {
    case Operation::Lt: return Operation::Gt; case Operation::Le: return Operation::Ge;
    case Operation::Gt: return Operation::Lt; case Operation::Ge: return Operation::Le;
    case Operation::Ult: return Operation::Ugt; case Operation::Ule: return Operation::Uge;
    case Operation::Ugt: return Operation::Ult; case Operation::Uge: return Operation::Ule;
    default: return op;
    }
}
Operation inverted(Operation op) {
    switch (op) {
    case Operation::Lt: return Operation::Ge; case Operation::Le: return Operation::Gt;
    case Operation::Gt: return Operation::Le; case Operation::Ge: return Operation::Lt;
    case Operation::Ult: return Operation::Uge; case Operation::Ule: return Operation::Ugt;
    case Operation::Ugt: return Operation::Ule; case Operation::Uge: return Operation::Ult;
    case Operation::Eq: return Operation::Ne; case Operation::Ne: return Operation::Eq;
    default: return Operation::None;
    }
}
// All analysis is function-owned. Candidate walks share a linear work budget;
// a failed candidate does not leave partial rewrites or invalidate other facts.
class Loops {
    Program& p;
    const Function f;
    OrdinaryFlow flow;
    std::uint64_t& work;
    LoopStats& stats;
    unsigned level;
    std::uint64_t budget, growth = 0;
    std::uint64_t& unit_growth;
    SymbolId& fill_bytes;
    cppgm::IdIndex ids;
    std::vector<Fact> facts = std::vector<Fact>(1);
    std::vector<Use> uses = std::vector<Use>(1);
    std::vector<unsigned> member;
    std::vector<bool> committed;
    std::vector<Range> replacements;
    Pool<Instruction> planned;
    NameIndex names;
    bool names_ready = false;
    unsigned serial = 0;
    unsigned local(unsigned id) {
        if (auto n = ids.get(id)) return n;
        unsigned n = facts.size(); ids.put(id,n); facts.emplace_back(); return n;
    }
    bool charge(std::uint64_t n = 1) {
        if (n > budget) { budget = 0; return false; }
        budget -= n; work += n; return true;
    }
    Range range(unsigned b) const { return p.blocks[flow.blocks[b].index-1].instructions; }
    const Instruction* definition(Operand a) const {
        if (a.kind != Operand::Temporary) return nullptr;
        const auto& v = facts[ids.get(a.ref)];
        return v.definitions == 1 && v.instruction ? &p.instructions[v.instruction-1] : nullptr;
    }
    Operand incoming(const Instruction& phi, unsigned block) const {
        for (unsigned n = phi.operands.begin; n < phi.operands.end(); n += 2)
            if (p.operands[n].ref == flow.blocks[block].index) return p.operands[n+1];
        return Operand::label(BlockId()); // not a value; proof will decline
    }
    void index() {
        auto params = p.signatures[f.signature.index-1].parameters;
        for (unsigned n = params.begin; n < params.end(); ++n) ++facts[local(p.parameters[n].value.index)].definitions;
        for (unsigned b = 1; b < flow.blocks.size(); ++b) {
            auto r = range(b);
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n]; ++work;
                if (i.destination) {
                    auto& v = facts[local(i.destination.index)]; ++v.definitions; v.instruction = n+1; v.block = b;
                }
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto a = p.operands[k]; ++work;
                    if (a.kind != Operand::Temporary) continue;
                    unsigned point = i.opcode == Opcode::Phi ? flow.local.get(p.operands[k-1].ref) : b;
                    auto& v = facts[local(a.ref)]; uses.push_back({b,point,v.users}); v.users = uses.size()-1;
                }
            }
        }
    }
    bool shape(Loop& l, unsigned first, unsigned exit) {
        if (exit == l.header || first == exit || first == l.header) return false;
        l.blocks.clear(); l.blocks.push_back(l.header);
        unsigned previous = l.header, b = first;
        while (b != l.header) {
            if (!charge() || member[b] == l.header || committed[b] || b == exit) return false;
            auto e = flow.incoming[b];
            if (!e || flow.edges[e].next_in || flow.edges[e].from != previous) return false;
            member[b] = l.header; l.blocks.push_back(b);
            const auto& t = p.instructions[range(b).end()-1];
            if (t.opcode != Opcode::Jump) return false;
            previous = b; b = flow.local.get(p.operands[t.operands.begin].ref);
        }
        l.latch = previous; l.exit = exit;
        auto e = flow.incoming[l.header];
        if (!e || !flow.edges[e].next_in || flow.edges[flow.edges[e].next_in].next_in) return false;
        unsigned a = flow.edges[e].from, c = flow.edges[flow.edges[e].next_in].from;
        if (a != l.latch && c != l.latch) return false;
        l.entry = a == l.latch ? c : a;
        return l.entry != l.header && member[l.entry] != l.header &&
            flow.dominates(l.header,l.latch) && flow.dominates(l.entry,l.header);
    }
    bool analyze(Loop& l) {
        auto hr = range(l.header);
        const auto& terminal = p.instructions[hr.end()-1];
        if (terminal.opcode != Opcode::Branch) return false;
        auto cmp = definition(p.operands[terminal.operands.begin]);
        if (!cmp || cmp->opcode != Opcode::Compare ||
            (!cmp->type.integer() && cmp->type != Type::Ptr) || cmp->type.width() > 64) return false;
        l.pointer = cmp->type == Type::Ptr;
        l.compare = cmp-p.instructions.data();
        if (l.compare != hr.end()-2) return false;
        unsigned yes = flow.local.get(p.operands[terminal.operands.begin+1].ref);
        unsigned no = flow.local.get(p.operands[terminal.operands.begin+2].ref);
        // Only a successor dominated by the header and returning on a linear
        // chain can be the body. Reset membership after each attempted arm.
        bool found = shape(l,yes,no); l.body_true = true;
        if (!found) {
            for (auto b : l.blocks) member[b] = 0;
            found = shape(l,no,yes); l.body_true = false;
        }
        if (!found) return false;
        member[l.header] = l.header;
        for (unsigned n = hr.begin; n < l.compare; ++n) {
            if (!charge() || p.instructions[n].opcode != Opcode::Phi || p.instructions[n].operands.count != 4) return false;
            l.phis.push_back(n);
            // A phi may legally name a source-later entry definition. Copies
            // replacing that phi cannot; retain such source ordering intact.
            auto initial = incoming(p.instructions[n],l.entry);
            if (initial.kind == Operand::Temporary && facts[ids.get(initial.ref)].instruction > hr.begin) return false;
        }
        auto a = p.operands[cmp->operands.begin], b = p.operands[cmp->operands.begin+1];
        Operation op = cmp->operation;
        if (a.kind == Operand::Integer || (l.pointer && (!definition(a) || definition(a)->opcode != Opcode::Phi))) { std::swap(a,b); op = swapped(op); }
        auto phi = definition(a);
        if (!phi || phi->opcode != Opcode::Phi || phi->type != cmp->type ||
            facts[ids.get(a.ref)].block != l.header || (!l.pointer && b.kind != Operand::Integer)) return false;
        l.induction = phi-p.instructions.data();
        if (l.pointer) {
            if (!pointer_induction(l,*phi,b,l.body_true ? op : inverted(op))) return false;
        } else {
            auto next = incoming(*phi,l.latch); auto update = definition(next);
            if (!update || update->opcode != Opcode::Binary || update->type != phi->type ||
                member[facts[ids.get(next.ref)].block] != l.header ||
                (update->operation != Operation::Add && update->operation != Operation::Sub)) return false;
            auto x = p.operands[update->operands.begin], y = p.operands[update->operands.begin+1];
            if (update->operation == Operation::Add && same_scalar(y,a)) std::swap(x,y);
            if (!same_scalar(x,a) || y.kind != Operand::Integer) return false;
            l.update = update-p.instructions.data();
            if (!l.body_true) op = inverted(op);
            if (!constant_trip(phi->type,op,incoming(*phi,l.entry),b,y,update->operation == Operation::Sub,l.trips,l.final)) return false;
        }
        for (auto block : l.blocks) {
            auto r = range(block);
            for (unsigned n = r.begin; n < r.end(); ++n) {
                const auto& i = p.instructions[n];
                if (!charge(1+i.operands.count)) return false;
                if (block != l.header && !terminator(i.opcode)) {
                    if (i.opcode == Opcode::Phi) return false;
                    l.body.push_back(n);
                }
                if (i.destination) {
                    auto& fact = facts[ids.get(i.destination.index)];
                    if (fact.definitions != 1) return false;
                    bool exported = false;
                    for (unsigned u = fact.users; u; u = uses[u].next) {
                        if (!charge()) return false;
                        if (member[uses[u].block] != l.header) {
                            if (!flow.dominates(l.header,uses[u].point)) return false;
                            exported = true;
                        }
                    }
                    if (exported) l.exports.push_back(n);
                }
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) {
                    auto arg = p.operands[k];
                    if (arg.kind != Operand::Temporary) continue;
                    const auto& v = facts[ids.get(arg.ref)];
                    if (v.definitions != 1) return false;
                    if (v.block && member[v.block] != l.header && !flow.dominates(v.block,l.entry)) return false;
                }
            }
        }
        return true;
    }
    bool pointer_induction(Loop& l, const Instruction& phi, Operand limit, Operation op) {
        if (op != Operation::Ne || !charge(12+2*l.phis.size())) return false;
        l.initial = incoming(phi,l.entry); l.final = limit;
        if (l.initial.kind == Operand::Slot) return false;
        auto next = incoming(phi,l.latch);
        auto update = definition(next);
        if (!update || update->opcode != Opcode::Index || member[facts[ids.get(next.ref)].block] != l.header) return false;
        l.update = update-p.instructions.data();
        auto step = p.operands[update->operands.begin+1];
        if (step.kind != Operand::Integer || !update->type.bytes()) return false;
        l.stride = step.data.integer*update->type.bytes();
        auto carrier = definition(p.operands[update->operands.begin]);
        if (!carrier || carrier->opcode != Opcode::Phi) return false;
        for (auto n : l.phis) {
            const auto& twin = p.instructions[n];
            if (twin.type != Type::Ptr || !same_scalar(incoming(twin,l.entry),l.initial) ||
                !same_scalar(incoming(twin,l.latch),next)) return false;
        }
        bool found = false;
        for (auto n : l.phis) found |= &p.instructions[n] == carrier;
        if (!found) return false;
        if (limit.kind == Operand::Slot) return false;
        if (limit.kind == Operand::Temporary) {
            const auto& fact = facts[ids.get(limit.ref)];
            if (fact.definitions != 1 || (fact.block && !flow.dominates(fact.block,l.entry))) return false;
        }
        // A 64-bit pointer induction with an odd byte stride visits every
        // representable address before repeating: gcd(stride, 2^64) = 1.
        // Hence equality with any invariant pointer terminates. This uses no
        // object bounds or alignment promise. Even strides need a separate
        // congruence proof and must keep a possibly nonterminating loop.
        l.finite_pointer = l.stride & 1;
        l.range = pointer_range(l,limit);
        return l.finite_pointer || l.range;
    }
    bool pointer_range(Loop& l, Operand limit) {
        auto bound = definition(limit);
        if (!bound || bound->opcode != Opcode::Index || bound->type.bytes() != 1 ||
            !same_scalar(p.operands[bound->operands.begin],l.initial)) return false;
        // Contiguous forward scalar stores require an exact byte count.
        if (!l.stride || l.stride > 8 || (l.stride & (l.stride-1))) return false;
        l.count = p.operands[bound->operands.begin+1];
        if (l.count.kind == Operand::Temporary) {
            const auto& fact = facts[ids.get(l.count.ref)];
            if (p.values[l.count.ref-1].type != Type::I64 || fact.definitions != 1 ||
                (fact.block && !flow.dominates(fact.block,l.entry))) return false;
        }
        if (l.count.kind != Operand::Temporary && l.count.kind != Operand::Integer) return false;
        if (l.stride == 1) return true;
        if (l.count.kind == Operand::Integer) return !(l.count.data.integer & (l.stride-1));
        // A widened byte-count expression must be an exact multiple of the
        // stride, including modular shifts/multiplication in the LowIR domain.
        auto bytes = definition(l.count);
        if (!bytes || bytes->opcode != Opcode::Binary || bytes->type != Type::I64) return false;
        auto factor = p.operands[bytes->operands.begin+1];
        if (factor.kind != Operand::Integer) return false;
        if (bytes->operation == Operation::Mul) return !(factor.data.integer & (l.stride-1));
        return bytes->operation == Operation::Shl && factor.data.integer < 64 &&
            (std::uint64_t(1)<<factor.data.integer) >= l.stride;
    }
    void emit(Pool<Instruction>& out, Instruction i, const std::vector<Operand>& args) {
        i.operands.begin = p.operands.size(); i.operands.count = args.size();
        for (auto a : args) p.operands.push_back(a);
        out.push_back(i); work += 1+args.size();
    }
    void assign(Pool<Instruction>& out, const Instruction& original, Operand value) {
        Instruction i(value.literal() ? Opcode::Const : Opcode::Copy,original.result_type());
        i.destination = original.destination; i.debug = original.debug;
        emit(out,i,{value});
    }
    bool erase(const Loop& l) {
        // An arbitrarily large finite loop may disappear only when every
        // executed operation is nontrapping/pure and every escaping value is
        // available analytically. Zero-trip bodies need not be pure.
        if (l.trips) for (auto n : l.body) if (!discardable(p.instructions[n])) return false;
        std::vector<Operand> results;
        for (auto n : l.exports) {
            if (n == l.induction || (n == l.update && l.trips)) results.push_back(l.final);
            else if (n == l.compare) results.push_back(Operand::integer(!l.body_true));
            else if (!l.trips && p.instructions[n].opcode == Opcode::Phi) results.push_back(incoming(p.instructions[n],l.entry));
            else return false;
        }
        if (!charge(2*results.size()+1)) return false;
        for (unsigned n = 0; n < l.exports.size(); ++n) assign(planned,p.instructions[l.exports[n]],results[n]);
        ++stats.removed; return true;
    }
    bool fill(Loop& l) {
        if (!l.range) return false;
        const Instruction* store = nullptr;
        const Instruction* load = nullptr;
        for (auto n : l.body) {
            if (!charge()) return false;
            const auto& i = p.instructions[n];
            if (i.opcode == Opcode::Store && !store && !i.is_volatile) store = &i;
            else if (i.opcode == Opcode::Load && !load && !i.is_volatile) load = &i;
            else if (n != l.update) return false;
        }
        if (!store || !store->type.integer() || store->type.bytes() != l.stride) return false;
        auto destination = definition(p.operands[store->operands.begin+1]);
        bool carrier = false;
        for (auto n : l.phis) { if (!charge()) return false; carrier |= destination == &p.instructions[n]; }
        if (!carrier) return false;
        auto value = p.operands[store->operands.begin];
        if (load) {
            if (l.stride != 1 || load->type != store->type || load->type.width() != 8 ||
                value.kind != Operand::Temporary || value.ref != load->destination.index) return false;
            value = p.operands[load->operands.begin];
            if (value.kind == Operand::Temporary && (p.values[value.ref-1].type != Type::Ptr ||
                member[facts[ids.get(value.ref)].block] == l.header)) return false;
        } else if (value.kind == Operand::Temporary) {
            // Scalar byte stores truncate to their low byte. An immutable
            // runtime value is as reusable as a repeated-byte literal.
            if (store->type.width() != 8 || !p.values[value.ref-1].type.integer() ||
                p.values[value.ref-1].type.width() > 64 || member[facts[ids.get(value.ref)].block] == l.header) return false;
        } else {
            if (value.kind != Operand::Integer) return false;
            value = normalize_integer(value,store->type);
            unsigned byte = value.data.integer & 255;
            for (unsigned n = 1; n < l.stride; ++n) if (((value.data.integer>>(8*n))&255) != byte) return false;
            value = Operand::integer(byte);
        }
        // Every exported phi denotes the final pointer, even on the zero-trip
        // path. Body temporaries (including the load) have no zero-trip value.
        for (auto n : l.exports) if (n != l.compare && p.instructions[n].opcode != Opcode::Phi) return false;
        // Replace at least a compare, branch, update, jump and store. Exports
        // replace existing definitions, so this rewrite never grows the IR.
        if (!charge(4+2*l.exports.size())) return false;
        // The fill body adds one ordinary edge to the original exit. Charge
        // complete phi rewrites before committing; a shared exit can at most
        // double its original edge operands across all accepted loops.
        auto exit = range(l.exit);
        for (unsigned n = exit.begin; n < exit.end() && p.instructions[n].opcode == Opcode::Phi; ++n)
            if (!charge(1+p.instructions[n].operands.count+2)) return false;
        for (unsigned n = exit.begin; n < exit.end() && p.instructions[n].opcode == Opcode::Phi; ++n) {
            auto& phi = p.instructions[n]; auto old = phi.operands;
            auto value = incoming(phi,l.header);
            phi.operands.begin = p.operands.size(); phi.operands.count += 2;
            for (unsigned k = old.begin; k < old.end(); ++k) p.operands.push_back(p.operands[k]);
            p.operands.push_back(Operand::label(flow.blocks[l.blocks[1]])); p.operands.push_back(value);
        }
        auto runtime = fill_runtime(p,fill_bytes,work);
        unsigned start = planned.size();
        for (auto n : l.exports) assign(planned,p.instructions[n],n == l.compare ? Operand::integer(!l.body_true) : l.final);
        // Keep the zero-trip guard in the caller. Besides preserving a null
        // reference on that path, this avoids runtime-call overhead for empty
        // ranges. The byte count itself is the branch condition; no extra
        // comparison, block or growth reservation is required.
        Instruction branch(Opcode::Branch); branch.debug = p.instructions[l.compare].debug;
        emit(planned,branch,{l.count,Operand::label(flow.blocks[l.blocks[1]]),Operand::label(flow.blocks[l.exit])});
        replacements[l.header].begin = start; replacements[l.header].count = planned.size()-start;
        start = planned.size();
        if (load) { emit(planned,*load,{value}); value = Operand::value(load->destination); }
        Instruction call(Opcode::Call,Type::Void); call.debug = store->debug;
        emit(planned,call,{Operand::symbol(runtime),l.initial,value,l.count});
        Instruction jump(Opcode::Jump); jump.debug = p.instructions[range(l.latch).end()-1].debug;
        emit(planned,jump,{Operand::label(flow.blocks[l.exit])});
        replacements[l.blocks[1]].begin = start; replacements[l.blocks[1]].count = planned.size()-start;
        l.guarded = true;
        ++stats.fills; return true;
    }
    bool erase_pointer(const Loop& l) {
        if (!l.finite_pointer) return false;
        for (auto n : l.body) if (!discardable(p.instructions[n])) return false;
        for (auto n : l.exports) if (n != l.compare && p.instructions[n].opcode != Opcode::Phi) return false;
        if (!charge(l.body.size()+2*l.exports.size()+1)) return false;
        for (auto n : l.exports) assign(planned,p.instructions[n],n == l.compare ? Operand::integer(!l.body_true) : l.final);
        retire_entry_load(l);
        ++stats.removed; return true;
    }
    void retire_entry_load(const Loop& l) {
        auto load = definition(l.initial);
        if (!load || load->opcode != Opcode::Load || load->is_volatile ||
            facts[ids.get(l.initial.ref)].block != l.entry) return;
        auto entry = range(l.entry), exit = range(l.exit);
        const auto& terminal = p.instructions[entry.end()-1];
        if (terminal.opcode != Opcode::Jump || p.operands[terminal.operands.begin].ref != flow.blocks[l.header].index) return;
        const auto& store = p.instructions[exit.begin];
        if (store.opcode != Opcode::Store || store.is_volatile || store.type != load->type) return;
        auto address = p.operands[load->operands.begin];
        if (!same_scalar(address,p.operands[store.operands.begin+1]) ||
            (address.kind == Operand::Temporary && facts[ids.get(address.ref)].definitions != 1)) return;
        for (unsigned u = facts[ids.get(l.initial.ref)].users; u; u = uses[u].next) {
            if (!charge() || member[uses[u].block] != l.header) return;
        }
        for (unsigned n = load-p.instructions.data()+1; n+1 < entry.end(); ++n)
            if (!charge() || !discardable(p.instructions[n])) return;
        // The finite, pure path necessarily reaches an equal-extent write to
        // the same address. That access proves this now-unused read cannot
        // introduce a distinct memory fault or observable event.
        auto& dead = p.instructions[load-p.instructions.data()];
        dead.opcode = Opcode::Nop; dead.destination = ValueId(); dead.type = Type(); dead.operands.count = 0;
    }
    ValueId fresh(const Instruction& i) {
        if (!names_ready) {
            for (unsigned n = 1; n < facts.size(); ++n) if (facts[n].instruction) {
                auto name = p.values[p.instructions[facts[n].instruction-1].destination.index-1].name;
                if (name) names.insert(name,1);
            }
            auto params = p.signatures[f.signature.index-1].parameters;
            for (unsigned n = params.begin; n < params.end(); ++n) {
                auto name = p.values[p.parameters[n].value.index-1].name;
                if (name) names.insert(name,1);
            }
            work += facts.size()+params.count; names_ready = true;
        }
        Name name;
        do { name = p.intern("%opt_loop_"+std::to_string(serial++)); } while (!names.insert(name,1));
        Value v; v.name = name; v.type = i.result_type(); v.owner = p.values[i.destination.index-1].owner;
        v.truth = p.values[i.destination.index-1].truth;
        p.values.push_back(v); return ValueId(p.values.size());
    }
    bool unroll(const Loop& l) {
        if (level < 3 || l.trips > 4 || l.trips*l.body.size() > 64) return false;
        // Reserve the *unsimplified* expansion, phi snapshots and final
        // bindings included. No constant folding credit funds more cloning.
        std::uint64_t copies = l.trips*(l.body.size()+l.phis.size())+l.phis.size()+l.exports.size()+1;
        std::uint64_t operands = 0;
        for (auto n : l.body) operands += p.instructions[n].operands.count;
        if (copies > 256-growth || copies > unit_growth || !charge(4*copies+l.trips*operands+1)) return false;
        // A body temporary has no final value on a zero-trip path.
        if (!l.trips) for (auto n : l.exports) if (n != l.compare && p.instructions[n].opcode != Opcode::Phi) return false;
        // Def-use IDs form a bounded compact overlay, never a clone of the
        // function environment. Its key set is exactly the loop's definitions.
        cppgm::IdIndex map;
        std::vector<Operand> values(1), next;
        for (auto block : l.blocks) {
            auto r = range(block);
            for (unsigned n = r.begin; n < r.end(); ++n) if (p.instructions[n].destination) {
                unsigned id = p.instructions[n].destination.index;
                map.put(id,values.size()); values.push_back(Operand::value(ValueId(id)));
            }
        }
        auto resolve = [&](Operand a) { auto k = a.kind == Operand::Temporary ? map.get(a.ref) : 0; return k ? values[k] : a; };
        auto snapshot = [&](unsigned n, Operand source) {
            const auto& phi = p.instructions[n];
            Instruction i(source.literal() ? Opcode::Const : Opcode::Copy,phi.type);
            i.destination = fresh(phi); i.debug = phi.debug;
            emit(planned,i,{source});
            values[map.get(phi.destination.index)] = Operand::value(i.destination);
        };
        for (auto n : l.phis) snapshot(n,incoming(p.instructions[n],l.entry));
        // The header comparison also produces an i64 value. Every executed
        // body observes the continuing arm's truth, including backedge phi
        // inputs. Its final exit value is installed only after those snapshots.
        values[map.get(p.instructions[l.compare].destination.index)] = Operand::integer(l.body_true);
        std::vector<Operand> args;
        for (std::uint64_t trip = 0; trip < l.trips; ++trip) {
            for (auto n : l.body) {
                auto i = p.instructions[n]; args.clear();
                for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) args.push_back(resolve(p.operands[k]));
                if (i.destination) {
                    auto original = i.destination; i.destination = fresh(i);
                    values[map.get(original.index)] = Operand::value(i.destination);
                }
                emit(planned,i,args);
            }
            // Phi edges are parallel snapshots, including cross-phi cycles.
            next.clear();
            for (auto n : l.phis) next.push_back(resolve(incoming(p.instructions[n],l.latch)));
            for (unsigned n = 0; n < l.phis.size(); ++n) snapshot(l.phis[n],next[n]);
        }
        values[map.get(p.instructions[l.compare].destination.index)] = Operand::integer(!l.body_true);
        for (auto n : l.exports) assign(planned,p.instructions[n],resolve(Operand::value(p.instructions[n].destination)));
        growth += copies; unit_growth -= copies; stats.cloned += copies; ++stats.unrolled;
        return true;
    }
public:
    Loops(Program& p, const Function& f, unsigned level, std::uint64_t& work, LoopStats& stats, std::uint64_t& unit_growth,
          SymbolId& fill_bytes)
        : p(p), f(f), flow(p,f,work), work(work), stats(stats), level(level),
          budget(16*(flow.size+flow.edges.size()+1)), unit_growth(unit_growth), fill_bytes(fill_bytes),
          member(flow.blocks.size()), committed(flow.blocks.size()), replacements(flow.blocks.size()) {}
    bool run(Pool<Instruction>& out) {
        if (!flow.dominance(work)) return false;
        index();
        bool changed = false;
        Loop l;
        for (unsigned b = 1; b < flow.blocks.size() && budget; ++b) {
            if (committed[b] || p.instructions[range(b).begin].opcode != Opcode::Phi) continue;
            l.header = b; l.guarded = false; l.blocks.clear(); l.phis.clear(); l.body.clear(); l.exports.clear(); ++stats.candidates;
            bool accepted = analyze(l);
            unsigned start = planned.size();
            if (accepted) accepted = l.pointer ? erase_pointer(l) || fill(l) : erase(l) || unroll(l);
            if (accepted) {
                if (!l.guarded) {
                    Instruction jump(Opcode::Jump); jump.debug = p.instructions[range(b).end()-1].debug;
                    emit(planned,jump,{Operand::label(flow.blocks[l.exit])});
                    replacements[b].begin = start; replacements[b].count = planned.size()-start;
                }
                for (auto block : l.blocks) committed[block] = true;
                changed = true;
            } else ++stats.declined;
            for (auto block : l.blocks) member[block] = 0;
        }
        if (!changed) return false;
        for (unsigned b = 1; b < flow.blocks.size(); ++b) {
            auto& block = p.blocks[flow.blocks[b].index-1]; auto old = block.instructions;
            block.instructions.begin = out.size();
            auto r = replacements[b];
            if (r.count) for (unsigned n = r.begin; n < r.end(); ++n) out.push_back(planned[n]);
            else for (unsigned n = old.begin; n < old.end(); ++n) out.push_back(p.instructions[n]);
            block.instructions.count = out.size()-block.instructions.begin;
        }
        return true;
    }
};
}
bool simplify_loops(Program& p, unsigned level, std::uint64_t& work, LoopStats& stats)
{
    Pool<Instruction> out; bool changed = false;
    std::uint64_t unit_growth = std::min<std::uint64_t>(4096,2*(p.instructions.size()+1));
    SymbolId fill_bytes;
    unsigned functions = p.functions.size();
    for (unsigned fn = 0; fn < functions; ++fn) {
        const auto f = p.functions[fn];
        bool candidate = false;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b)
            candidate |= p.instructions[p.blocks[p.block_order[b].index-1].instructions.begin].opcode == Opcode::Phi;
        bool done = candidate && Loops(p,f,level,work,stats,unit_growth,fill_bytes).run(out);
        changed |= done;
        if (!done) for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto& block = p.blocks[p.block_order[b].index-1]; auto r = block.instructions;
            block.instructions.begin = out.size();
            for (unsigned n = r.begin; n < r.end(); ++n) out.push_back(p.instructions[n]);
        }
    }
    p.instructions.swap(out); return changed;
}
}
