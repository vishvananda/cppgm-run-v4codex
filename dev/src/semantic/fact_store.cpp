#include "semantic/fact_store.h"

namespace cppgm { namespace semantic {
Fact& FactStore::edit(NodeId n)
{
    auto id = index[n];
    if (!id) {
        if (used%slab_size == 0) slabs.emplace_back(new Fact[slab_size]);
        id = ++used; index[n] = id;
    }
    return slabs[(id-1)/slab_size][(id-1)%slab_size];
}
} }
