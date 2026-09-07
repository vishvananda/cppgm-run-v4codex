#pragma once

#include "preprocess/source.h"
#include <vector>

namespace cppgm {

typedef std::uint32_t IdentifierId;

// Flat open addressing, <= 1/2 occupancy; IDs survive all storage growth.
// The table belongs to one translation unit. Views must be reacquired after
// insertion; consumers retain IDs instead of pointers into the byte arena.
class IdentifierTable {
public:
    explicit IdentifierTable(LexStats* stats = 0);
    IdentifierId intern(TextView text);
    TextView spelling(IdentifierId id) const;
    std::size_t size() const { return entries_.size(); }
    std::size_t storage_bytes() const;

private:
    struct Entry { std::uint64_t hash; std::size_t offset, length; };
    std::vector<Entry> entries_;
    std::vector<char> bytes_;
    std::vector<IdentifierId> slots_;
    LexStats* stats_;
    void grow();
};

} // namespace cppgm
