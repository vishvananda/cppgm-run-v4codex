#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::EmptyLayout::insert(EntityId e)
{
    if (unknown || seen.get(e)) return;
    if (types.size() == 64) { unknown = true; return; }
    seen.put(e,1); types.push_back(e);
}
bool Analyzer::EmptyLayout::merge(TypeId t)
{
    // Arrays repeat the same type identities; never expand their elements.
    while (sem.types[t].kind == TypeKind::Array) t = sem.types[t].child;
    if (!sem.class_value(t)) return false;
    auto child = sem.class_facts[sem.entities[sem.types[t].entity].class_info];
    if (!child.empty_types_count) return false;
    if (child.empty_types_count > 64) { unknown = true; return true; }
    bool overlap = unknown;
    for (unsigned i = 0; i < child.empty_types_count; ++i) {
        ++sem.layout_empty_work;
        auto e = sem.layout_empty_types[child.empty_types_begin+i];
        overlap |= seen.get(e) != 0;
        insert(e);
    }
    return overlap;
}
void Analyzer::EmptyLayout::publish(std::uint32_t info, EntityId empty)
{
    if (empty) insert(empty);
    auto& fact = sem.class_facts[info];
    if (unknown) { fact.empty_types_count = 65; ++sem.layout_empty_fallbacks; return; }
    fact.empty_types_begin = sem.layout_empty_types.size();
    fact.empty_types_count = types.size();
    sem.layout_empty_types.insert(sem.layout_empty_types.end(),types.begin(),types.end());
}
} }
