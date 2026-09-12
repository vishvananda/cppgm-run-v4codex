#include "lowering/procedural.h"
#include <array>
namespace cppgm { namespace lowering {
void Procedural::order_lifecycle_entries()
{
    using Group = std::array<FunctionId,3>;
    std::vector<Group> groups(1); semantic::Index owners;
    for (EntityId e = 1; e < sem.entities.size(); ++e) {
        if (!deleting_symbols[e]) continue;
        Group entries = {{FunctionId(),FunctionId(p.symbols[deleting_symbols[e].index-1].entity),FunctionId(p.symbols[symbols[e].index-1].entity)}};
        if (base_symbols[e]) entries[0] = FunctionId(p.symbols[base_symbols[e].index-1].entity);
        for (auto entry : entries) if (entry) owners.put(entry.index,groups.size());
        groups.push_back(entries);
    }
    if (groups.size() == 1 && p.function_order.empty()) return;
    // Earlier TU schedules are already complete. Newly emitted IDs are appended
    // in stable creation order, replacing each lifecycle group at its first ID.
    unsigned begin = p.function_order.size();
    std::vector<unsigned char> emitted(groups.size());
    for (unsigned i = begin; i < p.functions.size(); ++i) {
        unsigned group = owners.get(i+1);
        if (!group) p.function_order.push_back(FunctionId(i+1));
        else if (!emitted[group]) {
            emitted[group] = true;
            for (auto entry : groups[group]) if (entry) p.function_order.push_back(entry);
        }
    }
}
} }
