#pragma once
#include "semantic/model.h"
#include <memory>

namespace cppgm { namespace semantic {
// Only published facts own records. Syntax/occurrence IDs keep a compact
// optional index; a read of an absent fact does not materialize semantic state.
// Slabs keep references stable when recursive analysis publishes other facts.
class FactStore {
    static const std::uint32_t slab_size = 1024;
    std::vector<std::uint32_t> index;
    std::vector<std::unique_ptr<Fact[]> > slabs;
    std::uint32_t used = 0;
public:
    const Fact& operator[](NodeId n) const {
        auto id = index[n];
        static const Fact empty;
        return id ? slabs[(id-1)/slab_size][(id-1)%slab_size] : empty;
    }
    Fact& edit(NodeId n);
    void resize(std::size_t n) { index.resize(n); }
    std::size_t slot_count() const { return index.size(); }
    std::size_t fact_count() const { return used; }
    std::size_t storage_bytes() const {
        return index.capacity()*sizeof(std::uint32_t) +
            slabs.capacity()*sizeof(std::unique_ptr<Fact[]>) + slabs.size()*slab_size*sizeof(Fact);
    }
};
} }
