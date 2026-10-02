#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
namespace lowir_model {
namespace {
// Facts describe zero/nonzero, not an invented canonical 1 for arbitrary
// integer conditions. Their owner is a dominator scope and their values must
// have one definition. Removing an edge can only strengthen these proofs.
struct Restore { unsigned value, old; };
void propagate(Program& p, const Function& f, const std::vector<unsigned>& definitions, std::uint64_t& work)
{
    OrdinaryFlow flow(p,f,work); if (!flow.dominance(work)) return;
    cppgm::IdIndex local;
    std::vector<unsigned> facts(1), producers(1);
    auto value = [&](unsigned id) {
        if (auto old = local.get(id)) return old;
        unsigned n = facts.size(); local.put(id,n); facts.push_back(0); producers.push_back(0); return n;
    };
    for (unsigned b : flow.rpo) {
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            const auto& i = p.instructions[n];
            if (i.destination) producers[value(i.destination.index)] = n+1;
            for (unsigned k = i.operands.begin; k < i.operands.end(); ++k) if (p.operands[k].kind == Operand::Temporary) value(p.operands[k].ref);
        }
    }
    std::vector<Restore> undo;
    auto set = [&](Operand a, unsigned truth) {
        if (a.kind != Operand::Temporary || definitions[a.ref] != 1) return;
        unsigned v = local.get(a.ref); undo.push_back({v,facts[v]}); facts[v] = truth;
    };
    auto known = [&](Operand a) -> unsigned {
        if (a.kind == Operand::Integer) return a.data.integer || a.integer_high() ? 2 : 1;
        return a.kind == Operand::Temporary && definitions[a.ref] == 1 ? facts[local.get(a.ref)] : 0;
    };
    auto zero = [](Operand a) { return a.kind == Operand::Integer && !a.data.integer && !a.integer_high(); };
    auto visit = [&](unsigned b) {
        unsigned edge = flow.incoming[b];
        if (b != 1 && edge && !flow.edges[edge].next_in) {
            unsigned from = flow.edges[edge].from;
            const auto& t = p.instructions[p.blocks[flow.blocks[from].index-1].instructions.end()-1];
            if (t.opcode == Opcode::Branch) {
                auto a = p.operands[t.operands.begin];
                unsigned yes = p.operands[t.operands.begin+1].ref, no = p.operands[t.operands.begin+2].ref;
                if (yes != no) {
                    unsigned truth = yes == flow.blocks[b].index ? 2 : 1; set(a,truth);
                    unsigned producer = a.kind == Operand::Temporary ? producers[local.get(a.ref)] : 0;
                    if (producer && definitions[a.ref] == 1) {
                        const auto& i = p.instructions[producer-1];
                        if (i.opcode == Opcode::Compare && i.type.integer() && (i.operation == Operation::Eq || i.operation == Operation::Ne)) {
                            auto x = p.operands[i.operands.begin], y = p.operands[i.operands.begin+1];
                            if (zero(x) || zero(y)) set(zero(x) ? y : x,i.operation == Operation::Ne ? truth : 3-truth);
                        }
                    }
                }
            }
        }
        auto r = p.blocks[flow.blocks[b].index-1].instructions;
        for (unsigned n = r.begin; n < r.end(); ++n) {
            auto& i = p.instructions[n]; ++work;
            if (i.opcode == Opcode::Compare && i.type.integer() && (i.operation == Operation::Eq || i.operation == Operation::Ne)) {
                auto a = p.operands[i.operands.begin], other = p.operands[i.operands.begin+1];
                unsigned fact = zero(other) ? known(a) : zero(a) ? known(other) : 0;
                if (fact) {
                    bool result = (fact == 1) == (i.operation == Operation::Eq);
                    i.type = i.result_type(); i.opcode = Opcode::Const; i.operation = Operation::None;
                    i.operands.count = 1; p.operands[i.operands.begin] = Operand::integer(result); set(Operand::value(i.destination),result ? 2 : 1);
                }
            } else if (i.opcode == Opcode::Branch) {
                unsigned fact = known(p.operands[i.operands.begin]);
                if (fact) {
                    auto target = p.operands[i.operands.begin+(fact == 2 ? 1 : 2)];
                    i.opcode = Opcode::Jump; i.type = Type(); i.operands.count = 1; p.operands[i.operands.begin] = target;
                }
            }
        }
    };
    struct Scope { unsigned child, mark; };
    std::vector<Scope> stack;
    visit(1); stack.push_back({flow.first_child[1],0});
    while (!stack.empty()) {
        auto& s = stack.back();
        if (s.child) {
            unsigned b = s.child; s.child = flow.next_child[b]; unsigned mark = undo.size();
            visit(b); stack.push_back({flow.first_child[b],mark});
        } else {
            while (undo.size() > s.mark) { auto u = undo.back(); undo.pop_back(); facts[u.value] = u.old; }
            stack.pop_back();
        }
    }
}
}
void propagate_edge_facts(Program& p, std::uint64_t& work)
{
    std::vector<unsigned> definitions(p.values.size()+1);
    for (const auto& a : p.parameters) ++definitions[a.value.index];
    for (const auto& i : p.instructions) if (i.destination) ++definitions[i.destination.index];
    for (const auto& f : p.functions) if (!f.declaration) propagate(p,f,definitions,work);
}
}
