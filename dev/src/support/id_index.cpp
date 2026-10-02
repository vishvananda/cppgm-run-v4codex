#include "support/id_index.h"
namespace cppgm {
namespace {
std::uint64_t mix(std::uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
}
std::uint32_t IdIndex::find(std::uint64_t key) const
{
    std::size_t p = mix(key) & mask;
    while (slots[p].value) {
        if (slots[p].key() == key) return slots[p].value;
        p = (p + 1) & mask;
    }
    return 0;
}
void IdIndex::put(std::uint64_t key, std::uint32_t value)
{
    if (!value) {
        if (slots.empty()) return;
        auto hole = mix(key) & mask;
        while (slots[hole].value && slots[hole].key() != key) hole = (hole+1)&mask;
        if (!slots[hole].value) return;
        // Zero removes a binding. Close only its probe cluster so clearing a
        // pack lane cannot make a colliding, unrelated binding unreachable.
        for (auto next = (hole+1)&mask; slots[next].value; next = (next+1)&mask) {
            auto home = mix(slots[next].key())&mask;
            if (((next-home)&mask) >= ((next-hole)&mask)) {
                slots[hole] = slots[next]; hole = next;
            }
        }
        slots[hole] = Slot(); --used;
        return;
    }
    auto hash = mix(key);
    std::size_t p = hash & mask;
    if (!slots.empty()) {
        while (slots[p].value) {
            if (slots[p].key() == key) { slots[p].value = value; return; }
            p = (p+1)&mask;
        }
    }
    if (slots.empty() || (used + 1) * 2 >= slots.size()) {
        std::vector<Slot> old;
        old.swap(slots);
        slots.resize(old.empty() ? 32 : old.size() * 2);
        mask = slots.size()-1;
        if (old.empty()) used = 0;
        for (const Slot& s : old) if (s.value) {
            auto target = mix(s.key())&mask;
            while (slots[target].value) target = (target+1)&mask;
            slots[target] = s;
        }
        p = hash & mask;
        while (slots[p].value) p = (p+1)&mask;
    }
    ++used;
    slots[p].low = key;
    slots[p].high = key >> 32;
    slots[p].value = value;
}
}
