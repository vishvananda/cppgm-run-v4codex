#include "preprocess/identifier_table.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace cppgm {

IdentifierTable::IdentifierTable(LexStats* stats) : slots_(16, 0), stats_(stats) {}

void IdentifierTable::grow()
{
    slots_.assign(slots_.size() * 2, 0);
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        std::size_t slot = entries_[i].hash & (slots_.size() - 1);
        while (slots_[slot]) slot = (slot + 1) & (slots_.size() - 1);
        slots_[slot] = static_cast<IdentifierId>(i + 1);
    }
    if (stats_) ++stats_->storage_growths;
}

IdentifierId IdentifierTable::intern(TextView text)
{
    std::uint64_t hash = 14695981039346656037ull;
    for (std::size_t i = 0; i < text.size; ++i) {
        hash ^= static_cast<unsigned char>(text.data[i]);
        hash *= 1099511628211ull;
    }
    if ((entries_.size() + 1) * 2 > slots_.size()) grow();
    std::size_t slot = hash & (slots_.size() - 1);
    while (slots_[slot]) {
        if (stats_) ++stats_->intern_probes;
        const Entry& e = entries_[slots_[slot] - 1];
        if (e.hash == hash && e.length == text.size &&
            std::memcmp(bytes_.data() + e.offset, text.data, text.size) == 0)
            return slots_[slot];
        slot = (slot + 1) & (slots_.size() - 1);
    }
    if (entries_.size() == std::numeric_limits<IdentifierId>::max())
        throw std::runtime_error("too many identifiers in translation unit");
    Entry entry = {hash, bytes_.size(), text.size};
    if (stats_) {
        stats_->storage_growths += entries_.size() == entries_.capacity();
        stats_->storage_growths += bytes_.size() + text.size > bytes_.capacity();
    }
    bytes_.insert(bytes_.end(), text.data, text.data + text.size);
    entries_.push_back(entry);
    slots_[slot] = static_cast<IdentifierId>(entries_.size());
    return slots_[slot];
}

TextView IdentifierTable::spelling(IdentifierId id) const
{
    assert(id && id <= entries_.size());
    const Entry& entry = entries_[id - 1];
    return TextView(bytes_.data() + entry.offset, entry.length);
}

std::size_t IdentifierTable::storage_bytes() const
{
    return entries_.capacity() * sizeof(Entry) + bytes_.capacity() +
           slots_.capacity() * sizeof(IdentifierId);
}

} // namespace cppgm
