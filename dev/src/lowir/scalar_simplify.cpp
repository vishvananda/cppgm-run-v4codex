#include "lowir/folding.h"
#include "lowir/call_effects.h"
#include "support/id_index.h"
namespace lowir_model {
namespace {
struct Fact {
    unsigned definitions = 0, instruction = 0, users = 0;
    bool known = false;
    Operand replacement;
};
struct Use { unsigned instruction, next; };
class Scalars {
    Program& p;
    const Function& f;
    std::vector<bool>& removed;
    std::uint64_t& work;
    std::uint64_t budget;
    cppgm::IdIndex locals;
    std::vector<Fact> facts = std::vector<Fact>(1);
    std::vector<Use> uses = std::vector<Use>(1);
    std::vector<unsigned> body, pending;
    std::vector<bool> queued;
    std::vector<Operand> args;
    unsigned local(unsigned id) {
        if (auto old = locals.get(id)) return old;
        unsigned n = facts.size(); facts.emplace_back(); locals.put(id,n); return n;
    }
    Operand resolve(Operand a) {
        // Each alias is a same-type, single-definition earlier value. Charge
        // chain traversal as well as evaluation; exhaustion keeps valid IR.
        while (a.kind == Operand::Temporary && budget) {
            --budget; ++work;
            auto& fact = facts[locals.get(a.ref)];
            if (!fact.known || fact.definitions != 1) break;
            a = fact.replacement;
        }
        return a;
    }
    void enqueue(unsigned n) {
        if (!queued[n]) { queued[n] = true; pending.push_back(n); }
    }
    bool alias(const Instruction& i, Operand a) const {
        if (a.kind == Operand::Temporary) {
            auto n = locals.get(a.ref); auto& fact = facts[n];
            return fact.definitions == 1 && p.values[a.ref-1].type == i.result_type() &&
                p.values[a.ref-1].definition < p.values[i.destination.index-1].definition;
        }
        return i.result_type() == Type::Ptr && (a.kind == Operand::Symbol || a.kind == Operand::Null);
    }
    void reassociate(Instruction& i) {
        if (i.opcode != Opcode::Binary || !i.type.integer() || args[0].kind != Operand::Temporary ||
            args[1].kind != Operand::Integer) return;
        if (i.operation != Operation::Add && i.operation != Operation::Mul && i.operation != Operation::And &&
            i.operation != Operation::Or && i.operation != Operation::Xor) return;
        const auto& v = facts[locals.get(args[0].ref)];
        if (v.definitions != 1 || !v.instruction || !v.users || uses[v.users].next) return;
        const auto& inner = p.instructions[body[v.instruction-1]];
        if (inner.opcode != i.opcode || inner.operation != i.operation || inner.type != i.type) return;
        auto root = resolve(p.operands[inner.operands.begin]);
        auto constant = resolve(p.operands[inner.operands.begin+1]);
        if (constant.kind != Operand::Integer) return;
        // Moving a read to the outer expression requires a stable value;
        // a mutable input may have changed since the inner expression ran.
        if (root.kind == Operand::Temporary && !alias(i,root)) return;
        Operand inputs[] = {constant,args[1]}, combined;
        if (!fold_integer(i,inputs,combined)) return;
        args[0] = root; args[1] = combined;
        p.operands[i.operands.begin] = root; p.operands[i.operands.begin+1] = combined;
    }
    bool evaluate(Instruction& i, Operand& out) {
        reassociate(i);
        if (fold_integer(i,args.data(),out)) return true;
        auto a = args.empty() ? Operand() : args[0];
        if ((i.opcode == Opcode::Copy || (i.opcode == Opcode::Convert && i.type == i.source_type)) && alias(i,a)) { out = a; return true; }
        if (i.opcode == Opcode::Index && args[1].kind == Operand::Integer && !args[1].data.integer &&
            !args[1].integer_high() && alias(i,a)) { out = a; return true; }
        if (i.opcode == Opcode::Phi) {
            bool have = false;
            for (unsigned n = 1; n < args.size(); n += 2) {
                if (args[n].kind == Operand::Temporary && args[n].ref == i.destination.index) continue;
                if (!have) { a = args[n]; have = true; }
                else if (!same_scalar(a,args[n])) return false;
            }
            if (!have) return false;
            if (a.kind == Operand::Integer && i.type.integer()) { out = normalize_integer(a,i.type); return true; }
            if (alias(i,a)) { out = a; return true; }
        }
        if (i.opcode == Opcode::Compare && i.type.integer()) {
            auto zero = [&](Operand v) { return v.kind == Operand::Integer &&
                !normalize_integer(v,i.type).data.integer && !normalize_integer(v,i.type).integer_high(); };
            if ((zero(args[1]) && (i.operation == Operation::Ult || i.operation == Operation::Uge)) ||
                (zero(a) && (i.operation == Operation::Ugt || i.operation == Operation::Ule))) {
                out = Operand::integer(i.operation == Operation::Uge || i.operation == Operation::Ule); return true;
            }
            if (a.kind == Operand::Temporary && p.values[a.ref-1].truth && args[1].kind == Operand::Integer) {
                auto b = normalize_integer(args[1],i.type);
                bool identity = (i.operation == Operation::Ne && !b.data.integer && !b.integer_high()) ||
                    (i.operation == Operation::Eq && b.data.integer == 1 && !b.integer_high());
                if (identity && alias(i,a)) { out = a; return true; }
            }
            if (same_scalar(a,args[1])) {
                bool yes = i.operation == Operation::Eq || i.operation == Operation::Le || i.operation == Operation::Ge ||
                    i.operation == Operation::Ule || i.operation == Operation::Uge;
                out = Operand::integer(yes); return true;
            }
        }
        if (i.opcode == Opcode::Binary && i.type.integer() && args[1].kind == Operand::Integer) {
            auto b = normalize_integer(args[1],i.type);
            bool zero = !b.data.integer && !b.integer_high(), one = b.data.integer == 1 && !b.integer_high();
            if (((zero && (i.operation == Operation::Add || i.operation == Operation::Sub ||
                    i.operation == Operation::Or || i.operation == Operation::Xor || i.operation == Operation::Shl ||
                    i.operation == Operation::Shr || i.operation == Operation::Ushr)) ||
                 (one && i.operation == Operation::Mul)) && alias(i,a)) { out = a; return true; }
        }
        return false;
    }
    void index() {
        auto parameters = p.signatures[f.signature.index-1].parameters;
        for (unsigned n = parameters.begin; n < parameters.end(); ++n) ++facts[local(p.parameters[n].value.index)].definitions;
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
            auto r = p.blocks[p.block_order[n].index-1].instructions;
            for (unsigned k = r.begin; k < r.end(); ++k) {
                auto& i = p.instructions[k]; unsigned id = body.size(); body.push_back(k);
                if (i.destination) { auto& v = facts[local(i.destination.index)]; ++v.definitions; v.instruction = id+1; }
                for (unsigned a = i.operands.begin; a < i.operands.end(); ++a) if (p.operands[a].kind == Operand::Temporary) {
                    auto& v = facts[local(p.operands[a].ref)]; uses.push_back({id,v.users}); v.users = uses.size()-1;
                }
            }
        }
        queued.resize(body.size()); budget = 16*(body.size()+uses.size()+1);
        for (unsigned n = body.size(); n; --n) enqueue(n-1);
    }
    void propagate() {
        while (!pending.empty() && budget) {
            unsigned n = pending.back(); pending.pop_back(); queued[n] = false; ++work; --budget;
            auto& i = p.instructions[body[n]];
            if (!i.destination) continue;
            auto& fact = facts[locals.get(i.destination.index)];
            if (fact.definitions != 1) continue;
            if (fact.known && fact.replacement.kind != Operand::Temporary) continue;
            if (i.operands.count > budget/2) break;
            budget -= 2*i.operands.count; work += 2*i.operands.count;
            args.clear();
            for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) args.push_back(resolve(p.operands[j]));
            Operand result;
            if (!evaluate(i,result) || (fact.known && same_scalar(fact.replacement,result))) continue;
            fact.known = true; fact.replacement = result;
            for (unsigned u = fact.users; u && budget; u = uses[u].next) { --budget; ++work; enqueue(uses[u].instruction); }
        }
        // Rewriting also has a linear work allowance. An unresolved suffix of
        // an alias chain is still equivalent; no correctness depends on budget.
        budget = 16*(body.size()+uses.size()+1);
        for (unsigned n : body) {
            auto& i = p.instructions[n];
            for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) p.operands[j] = resolve(p.operands[j]);
            if (!i.destination) continue;
            const auto& fact = facts[locals.get(i.destination.index)];
            if (!fact.known || fact.definitions != 1) continue;
            auto replacement = resolve(fact.replacement);
            i.type = i.result_type(); i.opcode = replacement.literal() ? Opcode::Const : Opcode::Copy;
            i.operation = Operation::None; i.source_type = Type();
            p.operands[i.operands.begin] = replacement; i.operands.count = 1;
        }
    }
    void dead_code() {
        std::vector<bool> live(body.size()); pending.clear();
        auto mark = [&](unsigned n) { if (!live[n]) { live[n] = true; pending.push_back(n); } };
        for (unsigned n = 0; n < body.size(); ++n) {
            auto& i = p.instructions[body[n]];
            if (i.opcode == Opcode::Nop) continue;
            bool pure_call = false;
            if (i.opcode == Opcode::Call) {
                auto b = call_boundary(p,i);
                pure_call = b.effects != CFXM_DEFAULT && b.unwind == CUM_NO && b.returns != CRM_NORETURN;
            }
            if ((!i.destination && !pure_call) || (!discardable(i) && !pure_call) ||
                (i.destination && facts[locals.get(i.destination.index)].definitions != 1)) mark(n);
        }
        while (!pending.empty()) {
            unsigned n = pending.back(); pending.pop_back(); ++work;
            auto r = p.instructions[body[n]].operands;
            for (unsigned j = r.begin; j < r.end(); ++j) if (p.operands[j].kind == Operand::Temporary) {
                auto& v = facts[locals.get(p.operands[j].ref)];
                if (v.definitions == 1 && v.instruction) mark(v.instruction-1);
            }
        }
        for (unsigned n = 0; n < body.size(); ++n) removed[body[n]] = !live[n];
    }
public:
    Scalars(Program& p, const Function& f, std::vector<bool>& removed, std::uint64_t& work)
        : p(p), f(f), removed(removed), work(work), budget(0) {}
    void run() { index(); propagate(); dead_code(); }
};
}
void simplify_scalars(Program& p, std::uint64_t& work, const std::vector<bool>* selected)
{
    std::vector<bool> removed(p.instructions.size());
    for (unsigned fn = 0; fn < p.functions.size(); ++fn) {
        const auto& f = p.functions[fn];
        if (!f.declaration && (!selected || (*selected)[fn])) Scalars(p,f,removed,work).run();
    }
    Pool<Instruction> instructions; Pool<Operand> operands;
    instructions.reserve(p.instructions.size()); operands.reserve(p.operands.size());
    for (auto& v : p.values) { v.definition = 0; v.defined = false; }
    for (auto a : p.parameters) p.values[a.value.index-1].defined = true;
    for (auto id : p.block_order) {
        auto& b = p.blocks[id.index-1];
        auto old = b.instructions; b.instructions.begin = instructions.size(); b.instructions.count = 0;
        for (unsigned n = old.begin; n < old.end(); ++n) if (!removed[n]) {
            auto i = p.instructions[n]; auto args = i.operands; i.operands.begin = operands.size();
            for (unsigned j = args.begin; j < args.end(); ++j) operands.push_back(p.operands[j]);
            if (i.destination) {
                auto& v = p.values[i.destination.index-1];
                if (!v.defined) v.definition = instructions.size()+1;
                v.defined = true;
            }
            instructions.push_back(i); ++b.instructions.count;
        }
    }
    p.instructions.swap(instructions); p.operands.swap(operands);
}
}
