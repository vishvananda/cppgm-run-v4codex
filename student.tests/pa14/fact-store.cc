#include "semantic/fact_store.h"
#include <cassert>

int main()
{
    cppgm::semantic::FactStore facts;
    facts.resize(10000);
    const auto& read = facts;
    for (unsigned i = 0; i < 10000; ++i)
        assert(!read[i].entity && !read[i].type && !read[i].scope && !read[i].value && !read[i].target);
    assert(facts.fact_count() == 0);
    auto& object = facts.edit(7);
    object.entity = 91; object.scope = 17;
    auto& query = facts.edit(7000);
    query.type = 38; query.value = 44;
    facts.resize(50000);
    for (unsigned i = 10000; i < 50000; ++i) facts.edit(i).target = i-3;
    // References survive both sparse-index growth and multiple record slabs.
    object.target = 99; query.scope = 18;
    assert(&object == &read[7] && &query == &read[7000]);
    assert(read[7].entity == 91 && read[7].scope == 17 && read[7].target == 99);
    assert(read[7000].type == 38 && read[7000].value == 44 && read[7000].scope == 18);
    assert(read[49999].target == 49996);
    auto count = facts.fact_count();
    assert(!read[8].entity && !read[8].scope && facts.fact_count() == count);
}
