#include "lowir/folding.h"
#include "support/id_index.h"
namespace lowir_model {
namespace {
struct LocalSlot {
    bool escape = false, loaded = false;
    unsigned block = 0;
    Operand value;
};
void forward(Program& p, const Function& f, const std::vector<unsigned>& definitions,
    std::vector<bool>& remove, std::uint64_t& work)
{
    cppgm::IdIndex index;
    std::vector<LocalSlot> slots(f.slots.count+1);
    for (unsigned n = f.slots.begin; n < f.slots.end(); ++n) index.put(p.slot_order[n].index,n-f.slots.begin+1);
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
        auto r = p.blocks[p.block_order[n].index-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) {
            const auto& i = p.instructions[k]; ++work;
            for (unsigned j = 0; j < i.operands.count; ++j) {
                auto a = p.operands[i.operands.begin+j]; if (a.kind != Operand::Slot) continue;
                auto& s = slots[index.get(a.ref)];
                bool memory = (i.opcode == Opcode::Load && j == 0) || (i.opcode == Opcode::Store && j == 1);
                s.escape |= !memory || i.is_volatile || i.type != p.slots[a.ref-1].type || !i.type.scalar();
            }
        }
    }
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
        auto id = p.block_order[n].index; auto r = p.blocks[id-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) {
            auto& i = p.instructions[k]; ++work;
            if (i.opcode != Opcode::Load && i.opcode != Opcode::Store) continue;
            auto a = p.operands[i.operands.begin+(i.opcode == Opcode::Store)];
            if (a.kind != Operand::Slot) continue;
            auto& s = slots[index.get(a.ref)]; if (s.escape) continue;
            if (i.opcode == Opcode::Store) {
                auto v = p.operands[i.operands.begin];
                // A store captures a value. Do not replace its later load by
                // a mutable temporary, or bypass a width/format conversion.
                bool literal = (v.kind == Operand::Integer && i.type.integer()) || (v.kind == Operand::Null && i.type == Type::Ptr);
                bool stable = v.kind == Operand::Temporary && definitions[v.ref] == 1 && p.values[v.ref-1].type == i.type;
                bool symbol = v.kind == Operand::Symbol && i.type == Type::Ptr;
                s.block = literal || stable || symbol ? id : 0;
                s.value = literal && i.type.integer() ? normalize_integer(v,i.type) : v;
            } else if (s.block == id) {
                i.opcode = Opcode::Copy; p.operands[i.operands.begin] = s.value;
            } else s.loaded = true;
        }
    }
    for (unsigned n = f.blocks.begin; n < f.blocks.end(); ++n) {
        auto r = p.blocks[p.block_order[n].index-1].instructions;
        for (unsigned k = r.begin; k < r.end(); ++k) {
            const auto& i = p.instructions[k]; ++work;
            if (i.opcode != Opcode::Store) continue;
            auto a = p.operands[i.operands.begin+1]; if (a.kind != Operand::Slot) continue;
            auto& s = slots[index.get(a.ref)];
            if (!s.escape && !s.loaded) remove[k] = true;
        }
    }
}
}
void forward_local_slots(Program& p, std::uint64_t& work)
{
    std::vector<unsigned> definitions(p.values.size()+1);
    for (const auto& a : p.parameters) ++definitions[a.value.index];
    for (const auto& i : p.instructions) if (i.destination) ++definitions[i.destination.index];
    std::vector<bool> remove(p.instructions.size());
    for (const auto& f : p.functions) if (!f.declaration) forward(p,f,definitions,remove,work);
    // Erase only stores: definition ordinals and operand identities do not
    // change yet. Scalar compaction removes these no-ops in its one sweep.
    for (unsigned n = 0; n < p.instructions.size(); ++n) if (remove[n]) {
        auto& i = p.instructions[n]; i.opcode = Opcode::Nop; i.type = Type(); i.operands.count = 0;
    }
}
}
