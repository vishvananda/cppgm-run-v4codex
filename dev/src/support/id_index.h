#pragma once
#include <cstdint>
#include <vector>
namespace cppgm {
// Translation-unit owned, open-addressed indexes; no allocation per binding.
class IdIndex {
    // Keep a 64-bit key without the alignment padding of a uint64_t member.
    struct Slot {
        std::uint32_t low = 0, high = 0, value = 0;
        std::uint64_t key() const { return low | (std::uint64_t(high) << 32); }
    };
    std::vector<Slot> slots;
    std::size_t used = 0, mask = 0;
    std::uint32_t find(std::uint64_t key) const;
public:
    bool empty() const { return used == 0; }
    std::size_t size() const { return used; }
    std::uint32_t get(std::uint64_t key) const { return slots.empty() ? 0 : find(key); }
    void put(std::uint64_t key, std::uint32_t value);
};

}
