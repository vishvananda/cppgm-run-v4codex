#include "support/id_index.h"
namespace cppgm {
namespace {
std::uint64_t mix(std::uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
}
std::uint32_t IdIndex::get(std::uint64_t key) const
{
    if (slots.empty()) return 0;
    std::size_t p = mix(key) & (slots.size() - 1);
    while (slots[p].value) {
        if (slots[p].key == key) return slots[p].value;
        p = (p + 1) & (slots.size() - 1);
    }
    return 0;
}
void IdIndex::put(std::uint64_t key, std::uint32_t value)
{
    if (slots.empty() || (used + 1) * 2 >= slots.size()) {
        std::vector<Slot> old;
        old.swap(slots);
        slots.resize(old.empty() ? 32 : old.size() * 2);
        used = 0;
        for (const Slot& s : old) if (s.value) put(s.key, s.value);
    }
    std::size_t p = mix(key) & (slots.size() - 1);
    while (slots[p].value && slots[p].key != key) p = (p + 1) & (slots.size() - 1);
    if (!slots[p].value) ++used;
    slots[p].key = key;
    slots[p].value = value;
}
}
