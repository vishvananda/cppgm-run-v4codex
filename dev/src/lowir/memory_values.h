#pragma once
#include "lowir/model.h"
namespace lowir_model {
struct MemoryStats {
    std::uint64_t reused = 0, conditional = 0, copies = 0, diamonds = 0, declined = 0;
    std::uint64_t snapshots = 0, peak_snapshots = 0;
};
bool simplify_memory_values(Program&, std::uint64_t& work, MemoryStats&);
}
