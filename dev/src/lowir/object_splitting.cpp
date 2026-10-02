#include "lowir/folding.h"
#include "support/id_index.h"
#include <algorithm>
#include <array>
namespace lowir_model {
namespace {
// Function-owned address identities. Only single-definition address chains
// are resolved; a mutable pointer or an unknown projection makes its home
// escape. Copy components share a byte partition, not storage identity.
struct Origin { unsigned slot = 0, offset = 0; };
struct Address {
    unsigned definitions = 0, instruction = 0, users = 0;
    Origin origin;
};
struct Use { unsigned instruction, next; };
struct Home {
    SlotId id;
    unsigned parent = 0, rank = 0, bytes = 0;
    bool escape = false, conflict = false, bulk = false;
    std::array<Type,64> types;
    std::array<SlotId,64> fields;
};
class Objects {
    Program& p;
    Function& f;
    std::uint64_t& work;
    cppgm::IdIndex slot_ids, value_ids;
    std::vector<Home> homes = std::vector<Home>(1);
    std::vector<Address> addresses = std::vector<Address>(1);
    std::vector<Use> uses = std::vector<Use>(1);
    std::vector<unsigned> body, pending;
    NameIndex& names;
    bool& names_ready;
    unsigned& serial;
    unsigned local(unsigned id) {
        if (auto n = value_ids.get(id)) return n;
        unsigned n = addresses.size(); value_ids.put(id,n); addresses.emplace_back(); return n;
    }
    Origin origin(Operand a) const {
        Origin o;
        if (a.kind == Operand::Slot) o.slot = slot_ids.get(a.ref);
        if (a.kind == Operand::Temporary) o = addresses[value_ids.get(a.ref)].origin;
        return o;
    }
    unsigned root(unsigned n) {
        unsigned r = n;
        while (homes[r].parent != r) { r = homes[r].parent; ++work; }
        while (n != r) { unsigned next = homes[n].parent; homes[n].parent = r; n = next; }
        return r;
    }
    void join(unsigned a, unsigned b) {
        a = root(a); b = root(b); if (a == b) return;
        if (homes[a].rank < homes[b].rank) std::swap(a,b);
        homes[b].parent = a;
        if (homes[a].rank == homes[b].rank) ++homes[a].rank;
    }
    void index() {
        for (unsigned n = f.slots.begin; n < f.slots.end(); ++n) {
            auto id = p.slot_order[n]; auto t = p.slots[id.index-1].type;
            if (t.kind() != Type::Object || t.vector() || t.complex() || t.bytes() > 64) continue;
            Home h; h.id = id; h.bytes = t.bytes(); h.parent = homes.size();
            slot_ids.put(id.index,homes.size()); homes.push_back(h);
        }
        auto params = p.signatures[f.signature.index-1].parameters;
        for (unsigned n = params.begin; n < params.end(); ++n) ++addresses[local(p.parameters[n].value.index)].definitions;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto r = p.blocks[p.block_order[b].index-1].instructions;
            for (unsigned n = r.begin; n < r.end(); ++n) {
                body.push_back(n); const auto& i = p.instructions[n]; ++work;
                if (i.destination) { auto& a = addresses[local(i.destination.index)]; ++a.definitions; a.instruction = n+1; }
                for (unsigned j = i.operands.begin; j < i.operands.end(); ++j) {
                    auto a = p.operands[j]; ++work;
                    if (a.kind != Operand::Temporary) continue;
                    auto& v = addresses[local(a.ref)]; uses.push_back({n,v.users}); v.users = uses.size()-1;
                }
            }
        }
        pending = body;
        for (unsigned next = 0; next < pending.size(); ++next) {
            const auto& i = p.instructions[pending[next]]; ++work;
            if (!i.destination) continue;
            auto& v = addresses[value_ids.get(i.destination.index)];
            if (v.definitions != 1 || v.origin.slot || i.operands.count == 0) continue;
            if (i.opcode != Opcode::Addr && i.opcode != Opcode::Copy && i.opcode != Opcode::Index) continue;
            if (i.result_type() != Type::Ptr) continue;
            auto o = origin(p.operands[i.operands.begin]); if (!o.slot) continue;
            if (i.opcode == Opcode::Index) {
                auto offset = p.operands[i.operands.begin+1];
                if (offset.kind != Operand::Integer || offset.integer_high() || offset.negative_integer ||
                    !i.type.bytes() || offset.data.integer > 64/i.type.bytes()) continue;
                o.offset += unsigned(offset.data.integer)*i.type.bytes();
                if (o.offset > homes[o.slot].bytes) continue;
            }
            v.origin = o;
            for (unsigned u = v.users; u; u = uses[u].next) { pending.push_back(uses[u].instruction); ++work; }
        }
    }
    bool complete(Origin o, const Instruction& i) const {
        return o.slot && !o.offset && i.bytes == homes[o.slot].bytes;
    }
    void census() {
        // Complete local copies connect layout constraints before field uses
        // are collected. Escape is per home: a returned object remains in
        // memory, but a private object copied into it can still be split.
        for (auto n : body) {
            const auto& i = p.instructions[n]; ++work;
            if (i.opcode != Opcode::CopyObject) continue;
            auto a = origin(p.operands[i.operands.begin]), b = origin(p.operands[i.operands.begin+1]);
            if (complete(a,i) && complete(b,i)) join(a.slot,b.slot);
        }
        for (auto n : body) {
            const auto& i = p.instructions[n]; ++work;
            // Object values have no scalar extraction opcode. Keep the
            // materialization until inlining supplies an addressable home.
            if (i.opcode == Opcode::CopyObject) {
                auto a = p.operands[i.operands.begin];
                auto b = origin(p.operands[i.operands.begin+1]);
                if (b.slot && a.kind == Operand::Temporary && p.values[a.ref-1].type.kind() == Type::Object)
                    homes[b.slot].escape = true;
            }
            for (unsigned j = 0; j < i.operands.count; ++j) {
                auto o = origin(p.operands[i.operands.begin+j]); ++work;
                if (!o.slot) continue;
                auto& h = homes[o.slot]; auto& layout = homes[root(o.slot)];
                bool address = j == 0 && (i.opcode == Opcode::Addr || i.opcode == Opcode::Copy || i.opcode == Opcode::Index);
                if (address && i.destination && addresses[value_ids.get(i.destination.index)].origin.slot) continue;
                bool memory = (i.opcode == Opcode::Load && j == 0) || (i.opcode == Opcode::Store && j == 1);
                if (memory && !i.is_volatile && i.type.scalar() && i.type.bytes() &&
                    i.type.bytes() <= h.bytes && o.offset <= h.bytes-i.type.bytes()) {
                    auto& t = layout.types[o.offset];
                    if (t != Type() && t != i.type) layout.conflict = true;
                    t = i.type; continue;
                }
                if (i.opcode == Opcode::CopyObject || i.opcode == Opcode::ZeroInit) {
                    if (complete(o,i)) { layout.bulk = true; continue; }
                }
                h.escape = true;
            }
        }
        for (unsigned n = 1; n < homes.size(); ++n) if (root(n) == n) {
            auto& h = homes[n]; unsigned count = 0;
            for (unsigned b = 0; b < h.bytes;) {
                auto t = h.types[b];
                if (t == Type()) {
                    if (!h.bulk) { ++b; continue; }
                    unsigned end = b+1;
                    while (end < h.bytes && h.types[end] == Type()) ++end;
                    unsigned width = 8;
                    while (width > end-b) width /= 2;
                    t = Type(width == 8 ? Type::I64 : width == 4 ? Type::I32 : width == 2 ? Type::I16 : Type::I8);
                    h.types[b] = t;
                }
                ++count;
                for (unsigned k = b+1; k < b+t.bytes() && k < h.bytes; ++k)
                    if (h.types[k] != Type()) h.conflict = true;
                // Bulk transfers preserve representations, including padding.
                // Do not turn raw bytes into floating conversions or i1 truth.
                if (h.bulk && (t.floating() || t == Type::I1 || t == Type::I128)) h.conflict = true;
                b += t.bytes(); ++work;
            }
            if (count > 16) h.conflict = true;
        }
    }
    bool selected(Origin o) {
        return o.slot && !homes[o.slot].escape && !homes[root(o.slot)].conflict;
    }
    Name fresh(const char* prefix) {
        if (!names_ready) {
            for (const auto& v : p.values) if (v.name) { names.insert(v.name,1); ++work; }
            for (const auto& s : p.slots) if (s.name) { names.insert(s.name,1); ++work; }
            names_ready = true;
        }
        Name name;
        do { name = p.intern(std::string(prefix)+std::to_string(serial++)); } while (!names.insert(name,1));
        return name;
    }
    ValueId value(Type t) {
        Value v; v.owner = p.slots[homes[1].id.index-1].owner; v.name = fresh("%opt_object_"); v.type = t;
        p.values.push_back(v); return ValueId(p.values.size());
    }
    void emit(Pool<Instruction>& out, Instruction i, std::initializer_list<Operand> args) {
        i.operands.begin = p.operands.size(); i.operands.count = args.size();
        for (auto a : args) p.operands.push_back(a);
        out.push_back(i); work += 1+args.size();
    }
    Operand field(Pool<Instruction>& out, Operand base, Origin o, unsigned offset, DebugLocation debug) {
        if (selected(o)) return Operand::slot(homes[o.slot].fields[offset]);
        if (!offset) return base;
        Instruction i(Opcode::Index,Type::I8); i.projection = IPK_FIELD; i.destination = value(Type::Ptr); i.debug = debug;
        emit(out,i,{base,Operand::integer(offset)}); return Operand::value(i.destination);
    }
    void rewrite(Pool<Instruction>& out, Instruction i) {
        bool copy = i.opcode == Opcode::CopyObject, zero = i.opcode == Opcode::ZeroInit;
        if (copy || zero) {
            auto a = p.operands[i.operands.begin], b = copy ? p.operands[i.operands.begin+1] : a;
            auto source = copy ? origin(a) : Origin(), dest = origin(b);
            if (selected(source) || selected(dest)) {
                auto& layout = homes[root(selected(source) ? source.slot : dest.slot)];
                for (unsigned offset = 0; offset < layout.bytes;) {
                    auto t = layout.types[offset];
                    Operand from = t == Type::Ptr ? Operand::null() : Operand::integer(0);
                    if (copy) {
                        auto storage = field(out,a,source,offset,i.debug);
                        Instruction load(Opcode::Load,t); load.debug = i.debug; load.destination = value(t);
                        emit(out,load,{storage}); from = Operand::value(load.destination);
                    }
                    auto storage = field(out,b,dest,offset,i.debug);
                    Instruction store(Opcode::Store,t); store.debug = i.debug; emit(out,store,{from,storage});
                    offset += t.bytes();
                }
                return;
            }
        }
        if (i.opcode == Opcode::Load || i.opcode == Opcode::Store) {
            unsigned at = i.operands.begin+unsigned(i.opcode == Opcode::Store);
            auto o = origin(p.operands[at]);
            if (selected(o)) p.operands[at] = Operand::slot(homes[o.slot].fields[o.offset]);
        }
        out.push_back(i);
    }
public:
    Objects(Program& p, Function& f, std::uint64_t& work, NameIndex& names, bool& ready, unsigned& serial)
        : p(p), f(f), work(work), names(names), names_ready(ready), serial(serial) {}
    bool run(Pool<Instruction>& out, Pool<SlotId>& slots) {
        index(); if (homes.size() == 1) return false;
        census();
        // Preflight all growth before changing any home. At most 16 fields per
        // object and eight input instructions worth of added IR per function.
        std::uint64_t growth = 0; bool any = false;
        for (auto n : body) {
            const auto& i = p.instructions[n];
            if (i.opcode != Opcode::CopyObject && i.opcode != Opcode::ZeroInit) continue;
            auto a = origin(p.operands[i.operands.begin]);
            auto b = i.opcode == Opcode::CopyObject ? origin(p.operands[i.operands.begin+1]) : a;
            if (!selected(a) && !selected(b)) continue;
            const auto& h = homes[root(selected(a) ? a.slot : b.slot)];
            for (auto t : h.types) if (t != Type()) growth += 4;
        }
        if (growth > 8*(body.size()+1)) return false;
        p.stats.split_growth_reserved += growth;
        for (unsigned n = 1; n < homes.size(); ++n) {
            Origin o; o.slot = n; if (!selected(o)) continue;
            const auto& layout = homes[root(n)];
            bool added = false;
            for (unsigned b = 0; b < layout.bytes; ++b) if (layout.types[b] != Type()) {
                Slot s; s.name = fresh("$opt_object_"); s.type = layout.types[b]; s.owner = p.slots[homes[n].id.index-1].owner;
                p.slots.push_back(s); homes[n].fields[b] = SlotId(p.slots.size()); slots.push_back(homes[n].fields[b]); any = true;
                ++p.stats.split_fields;
                added = true;
            }
            p.stats.split_objects += added;
        }
        if (!any) return false;
        for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto& block = p.blocks[p.block_order[b].index-1]; auto old = block.instructions;
            block.instructions.begin = out.size();
            for (unsigned n = old.begin; n < old.end(); ++n) rewrite(out,p.instructions[n]);
            block.instructions.count = out.size()-block.instructions.begin;
        }
        return true;
    }
};
}
bool split_local_objects(Program& p, std::uint64_t& work)
{
    if (p.slots.empty()) return false;
    bool candidate = false;
    for (auto id : p.slot_order) {
        auto t = p.slots[id.index-1].type; ++work;
        candidate |= t.kind() == Type::Object && !t.vector() && !t.complex() && t.bytes() <= 64;
    }
    if (!candidate) return false;
    NameIndex names;
    bool names_ready = false;
    unsigned serial = p.values.size()+p.slots.size()+1;
    Pool<Instruction> instructions; Pool<SlotId> slots;
    bool changed = false;
    for (auto& f : p.functions) {
        auto old = f.slots; unsigned start = slots.size();
        bool candidate = false;
        for (unsigned n = old.begin; n < old.end(); ++n) slots.push_back(p.slot_order[n]);
        for (unsigned n = old.begin; n < old.end(); ++n) {
            auto t = p.slots[p.slot_order[n].index-1].type; ++work;
            candidate |= t.kind() == Type::Object && !t.vector() && !t.complex() && t.bytes() <= 64;
        }
        bool split = !f.declaration && candidate && Objects(p,f,work,names,names_ready,serial).run(instructions,slots);
        changed |= split;
        if (!split) for (unsigned b = f.blocks.begin; b < f.blocks.end(); ++b) {
            auto& block = p.blocks[p.block_order[b].index-1]; auto range = block.instructions;
            block.instructions.begin = instructions.size();
            for (unsigned n = range.begin; n < range.end(); ++n) instructions.push_back(p.instructions[n]);
        }
        f.slots.begin = start; f.slots.count = slots.size()-start;
    }
    p.instructions.swap(instructions); p.slot_order.swap(slots);
    return changed;
}
void retire_unused_slots(Program& p, std::uint64_t& work)
{
    std::vector<bool> used(p.slots.size()+1);
    for (const auto& i : p.instructions) for (unsigned n = i.operands.begin; n < i.operands.end(); ++n) {
        auto a = p.operands[n]; ++work;
        if (a.kind == Operand::Slot) used[a.ref] = true;
    }
    Pool<SlotId> slots;
    for (auto& f : p.functions) {
        auto old = f.slots; f.slots.begin = slots.size();
        for (unsigned n = old.begin; n < old.end(); ++n) {
            auto id = p.slot_order[n]; ++work;
            if (used[id.index]) slots.push_back(id);
        }
        f.slots.count = slots.size()-f.slots.begin;
    }
    p.slot_order.swap(slots);
}
}
