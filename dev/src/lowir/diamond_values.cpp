#include "lowir/folding.h"
#include "lowir/ordinary_flow.h"
namespace lowir_model {
namespace {
bool equal_when(const Instruction& comparison, const Program& p, bool taken, Operand x, Operand y, Type type)
{
    // Equality after a conversion proves equality only in that same domain.
    if (comparison.opcode != Opcode::Compare || !comparison.type.integer() || comparison.type != type) return false;
    auto a = p.operands[comparison.operands.begin], b = p.operands[comparison.operands.begin+1];
    auto normalize = [&](Operand v) { return v.kind == Operand::Integer ? normalize_integer(v,comparison.type) : v; };
    a = normalize(a); b = normalize(b); x = normalize(x); y = normalize(y);
    if (!((same_scalar(a,x) && same_scalar(b,y)) || (same_scalar(a,y) && same_scalar(b,x)))) return false;
    auto op = comparison.operation;
    if (op == Operation::Eq) return taken;
    if (op == Operation::Ne) return !taken;
    Operand zero = normalize(Operand::integer(0));
    Operand maximum = Operand::integer(~std::uint64_t(0));
    maximum.integer_high(~std::uint64_t(0)); maximum = normalize(maximum);
    if (op == Operation::Ult && !taken && same_scalar(b,maximum)) return true;
    if (op == Operation::Ule && taken && same_scalar(b,zero)) return true;
    if (op == Operation::Ult && !taken && same_scalar(a,zero)) return true;
    if (op == Operation::Ule && taken && same_scalar(a,maximum)) return true;
    return false;
}
}
void simplify_diamond_values(Program& p, std::uint64_t& work)
{
    std::vector<unsigned> definitions(p.values.size()+1), producer(p.values.size()+1);
    for (const auto& a : p.parameters) ++definitions[a.value.index];
    for (unsigned n = 0; n < p.instructions.size(); ++n) if (p.instructions[n].destination) {
        unsigned v = p.instructions[n].destination.index; ++definitions[v]; producer[v] = n+1;
    }
    for (const auto& f : p.functions) if (!f.declaration) {
        bool has_phi = false;
        for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n)
            has_phi |= p.instructions[p.blocks[p.block_order[n].index-1].instructions.begin].opcode == Opcode::Phi;
        if (!has_phi) continue;
        OrdinaryFlow flow(p,f,work); if (flow.exceptional) continue;
        for (unsigned merge = 1; merge < flow.blocks.size(); ++merge) {
            auto r = p.blocks[flow.blocks[merge].index-1].instructions;
            auto& phi = p.instructions[r.begin];
            if (phi.opcode != Opcode::Phi || phi.operands.count != 4 || definitions[phi.destination.index] != 1) continue;
            if (r.count > 1 && p.instructions[r.begin+1].opcode == Opcode::Phi) continue;
            unsigned left = flow.local.get(p.operands[phi.operands.begin].ref), right = flow.local.get(p.operands[phi.operands.begin+2].ref);
            auto le = flow.incoming[left], re = flow.incoming[right];
            if (!le || !re || flow.edges[le].next_in || flow.edges[re].next_in || flow.edges[le].from != flow.edges[re].from) continue;
            auto lr = p.blocks[flow.blocks[left].index-1].instructions, rr = p.blocks[flow.blocks[right].index-1].instructions;
            if (lr.count != 1 || rr.count != 1 || p.instructions[lr.begin].opcode != Opcode::Jump || p.instructions[rr.begin].opcode != Opcode::Jump) continue;
            unsigned from = flow.edges[le].from;
            auto fr = p.blocks[flow.blocks[from].index-1].instructions;
            auto& branch = p.instructions[fr.end()-1];
            if (branch.opcode != Opcode::Branch || from == merge || left == right) continue;
            auto condition = p.operands[branch.operands.begin];
            if (condition.kind != Operand::Temporary || definitions[condition.ref] != 1) continue;
            bool left_true = p.operands[branch.operands.begin+1].ref == flow.blocks[left].index;
            auto yes = p.operands[phi.operands.begin+(left_true ? 1 : 3)];
            auto no = p.operands[phi.operands.begin+(left_true ? 3 : 1)]; ++work;
            auto available = [&](Operand a) {
                // With two empty arms, a source-earlier definition of an
                // incoming value precedes the controlling branch. Mutable
                // reads must not move from their edge snapshots to the join.
                return a.literal() || (a.kind == Operand::Temporary && definitions[a.ref] == 1 && p.values[a.ref-1].definition <= fr.end());
            };
            Operand replacement; bool replace = false;
            if (same_scalar(yes,no) && available(yes)) { replacement = yes; replace = true; }
            unsigned cmp = producer[condition.ref];
            if (!replace && cmp && available(yes) && equal_when(p.instructions[cmp-1],p,false,yes,no,phi.type)) { replacement = yes; replace = true; }
            if (!replace && cmp && available(no) && equal_when(p.instructions[cmp-1],p,true,yes,no,phi.type)) { replacement = no; replace = true; }
            if (replace) {
                phi.opcode = replacement.literal() ? Opcode::Const : Opcode::Copy;
                if (phi.opcode == Opcode::Copy) phi.debug = DebugLocation();
                p.operands[phi.operands.begin] = replacement; phi.operands.count = 1;
            } else if (phi.type == Type::I64 && yes.kind == Operand::Integer && no.kind == Operand::Integer &&
                ((yes.data.integer == 1 && no.data.integer == 0) || (yes.data.integer == 0 && no.data.integer == 1)) &&
                !yes.integer_high() && !no.integer_high()) {
                // Reconstruct the canonical truth value; arbitrary nonzero
                // conditions cannot be substituted for a materialized 1.
                phi.opcode = Opcode::Compare; phi.type = p.values[condition.ref-1].type;
                phi.operation = yes.data.integer ? Operation::Ne : Operation::Eq;
                p.operands[phi.operands.begin] = condition; p.operands[phi.operands.begin+1] = Operand::integer(0); phi.operands.count = 2;
                p.values[phi.destination.index-1].truth = true;
            }
            if (phi.opcode != Opcode::Phi) {
                // Both arms are empty and their only value choice is now an
                // ordinary instruction. Retire this diamond locally; an
                // unrelated downstream phi conflict must not keep its edges.
                branch.opcode = Opcode::Jump; branch.type = Type();
                p.operands[branch.operands.begin] = Operand::label(flow.blocks[merge]);
                branch.operands.count = 1;
            }
        }
    }
}
}
