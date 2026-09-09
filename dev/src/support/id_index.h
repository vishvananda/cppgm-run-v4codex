#pragma once
#include <cstdint>
#include <vector>
namespace cppgm {
// Translation-unit owned, open-addressed indexes; no allocation per binding.
class IdIndex {
    struct Slot { std::uint64_t key = 0; std::uint32_t value = 0; };
    std::vector<Slot> slots;
    std::size_t used = 0;
public:
    std::uint32_t get(std::uint64_t key) const;
    void put(std::uint64_t key, std::uint32_t value);
};

}
