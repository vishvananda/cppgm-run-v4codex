#include "native/selection.h"
#include <climits>
namespace native {
void Selector::carry_reloads()
{
    // Two linear walks collect complete frame-use intervals. Only windows of
    // at most 64 selected instructions are inspected; no fixed-point retries.
    // Homes are private compiler temporaries, identified at creation (not by
    // rendered offsets). Unknown uses, aliases and effects keep the frame form.
    struct Use { unsigned first = ~0u, last = 0, block = 0, stores = 0; bool safe = true; };
    std::vector<Use> uses(f.frame.size()+1);
    for (const auto& b : f.blocks) for (unsigned n = b.instructions.begin; n != b.instructions.end(); ++n) {
        const auto& i = f.instructions[n];
        for (unsigned k = 0; k < i.count; ++k) {
            const auto& a = i.args[k];
            if (a.kind != Operand::Memory || !a.temporary || !a.id) continue;
            auto& u = uses[a.id];
            if (u.first == ~0u) { u.first = n; u.block = b.id; }
            u.last = n;
            bool store = i.op == Op::Store && k == 0;
            bool read = (i.op == Op::Load && k == 1) || (k == 1 &&
                (i.op == Op::Add || i.op == Op::Sub || i.op == Op::And || i.op == Op::Or ||
                 i.op == Op::Xor || i.op == Op::Mul || i.op == Op::Compare));
            u.stores += store;
            u.safe &= b.id == u.block && (store || read) && scalar_integer(i.type) && i.type.width() == 64 &&
                a.reg == XR_RBP && a.index < 0 && !a.address && a.displacement == f.frame[a.id-1].offset;
        }
    }
    std::array<unsigned,16> occupied = {{0}};
    for (unsigned n = 0; n < f.instructions.size(); ++n) {
        auto& store = f.instructions[n];
        auto home = store.args[0];
        if (store.op != Op::Store || home.kind != Operand::Memory || !home.temporary || !home.id || store.args[1].kind != Operand::Reg) continue;
        const auto& u = uses[home.id];
        if (!u.safe || u.first != n || u.stores != 1 || u.last <= n || u.last-n > 64) continue;
        int carrier = -1;
        for (int r : {XR_R11,XR_RAX,XR_R10}) {
            if (occupied[r] > n) continue;
            bool safe = true;
            for (unsigned j = n+1; j <= u.last && safe; ++j) {
                const auto& i = f.instructions[j];
                // This whitelist is also the effect boundary: integer moves,
                // typed loads and simple ALU operations have no hidden scratch
                // except the explicitly rejected large immediate case.
                safe = i.op == Op::Mov || i.op == Op::Load || i.op == Op::Store || i.op == Op::Lea ||
                    i.op == Op::ExtendSigned || i.op == Op::ExtendUnsigned || i.op == Op::Add ||
                    i.op == Op::Sub || i.op == Op::Mul || i.op == Op::And || i.op == Op::Or ||
                    i.op == Op::Xor || i.op == Op::Compare || i.op == Op::Set;
                for (unsigned k = 0; k < i.count && safe; ++k) {
                    const auto& a = i.args[k];
                    safe &= a.kind != Operand::Symbol;
                    if (a.kind == Operand::Reg || a.kind == Operand::Memory)
                        safe &= a.reg != r && a.index != r;
                    if (a.kind == Operand::Immediate &&
                        (std::int64_t(a.bits) < INT32_MIN || std::int64_t(a.bits) > INT32_MAX)) safe = false;
                }
            }
            if (safe) { carrier = r; break; }
        }
        if (carrier < 0) continue;
        occupied[carrier] = u.last+1;
        store.op = Op::Mov; store.args[0] = Operand::r(carrier);
        for (unsigned j = n+1; j <= u.last; ++j) {
            auto& i = f.instructions[j];
            for (unsigned k = 0; k < i.count; ++k) if (i.args[k].kind == Operand::Memory && i.args[k].id == home.id) {
                i.args[k] = Operand::r(carrier);
                if (i.op == Op::Load) i.op = Op::Mov;
                ++stats.scratch_carried_reloads;
            }
        }
    }
}
} // namespace native
