#include "lowir/loop_simplify.h"
#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
#include <algorithm>
namespace lowir_model {
namespace {
struct Fact { unsigned definitions = 0, instruction = 0, block = 0, users = 0; };
struct Use { unsigned block, next; };
struct Loop {
    unsigned header = 0, entry = 0, latch = 0, exit = 0, compare = 0, induction = 0, update = 0;
    bool body_true = false;
    std::uint64_t trips = 0;
    Operand final;
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
    const Function& f;
    OrdinaryFlow flow;
    std::uint64_t& work;
    LoopStats& stats;
    unsigned level;
    std::uint64_t budget, growth = 0;
    std::uint64_t& unit_growth;
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
                    auto& v = facts[local(a.ref)]; uses.push_back({b,v.users}); v.users = uses.size()-1;
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
        if (!cmp || cmp->opcode != Opcode::Compare || !cmp->type.integer() || cmp->type.width() > 64) return false;
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
        if (a.kind == Operand::Integer) { std::swap(a,b); op = swapped(op); }
        auto phi = definition(a);
        if (!phi || phi->opcode != Opcode::Phi || phi->type != cmp->type ||
            facts[ids.get(a.ref)].block != l.header || b.kind != Operand::Integer) return false;
        l.induction = phi-p.instructions.data();
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
                            if (!flow.dominates(l.header,uses[u].block)) return false;
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
    Loops(Program& p, const Function& f, unsigned level, std::uint64_t& work, LoopStats& stats, std::uint64_t& unit_growth)
        : p(p), f(f), flow(p,f,work), work(work), stats(stats), level(level),
          budget(16*(flow.size+flow.edges.size()+1)), unit_growth(unit_growth),
          member(flow.blocks.size()), committed(flow.blocks.size()), replacements(flow.blocks.size()) {}
    bool run(Pool<Instruction>& out) {
        if (!flow.dominance(work)) return false;
        index();
        bool changed = false;
        Loop l;
        for (unsigned b = 1; b < flow.blocks.size() && budget; ++b) {
            if (committed[b] || p.instructions[range(b).begin].opcode != Opcode::Phi) continue;
            l.header = b; l.blocks.clear(); l.phis.clear(); l.body.clear(); l.exports.clear(); ++stats.candidates;
            bool accepted = analyze(l);
            unsigned start = planned.size();
            if (accepted) accepted = erase(l) || unroll(l);
            if (accepted) {
                Instruction jump(Opcode::Jump); jump.debug = p.instructions[range(b).end()-1].debug;
                emit(planned,jump,{Operand::label(flow.blocks[l.exit])});
                replacements[b].begin = start; replacements[b].count = planned.size()-start;
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
    for (const auto& f : p.functions) {
        bool candidate = false;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b)
            candidate |= p.instructions[p.blocks[p.block_order[b].index-1].instructions.begin].opcode == Opcode::Phi;
        bool done = candidate && Loops(p,f,level,work,stats,unit_growth).run(out);
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
