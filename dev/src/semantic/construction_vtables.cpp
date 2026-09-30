#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::demand_construction_vtable(EntityId cls)
{
    auto id = virtual_class_id(cls);
    if (virtual_classes[id].construction_demand == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::Vtable,cls,entities[cls].source);
    if (virtual_classes[id].construction_demand != FactState::NotStarted) return;
    virtual_classes[id].construction_demand = FactState::Active;
    try {
        // A construction segment uses this base's final overriders even when
        // its ordinary table is key-owned by another TU. Demand only those
        // members, independently of ordinary table definition ownership.
        std::vector<EntityId> functions; Index seen;
        for (const auto& slot : virtual_classes[id].slots) {
            ++virtual_slot_work;
            if (seen.get(slot.function)) continue;
            seen.put(slot.function,1); functions.push_back(slot.function);
        }
        for (auto e : functions) {
            auto m = entities[e].member_info;
            if (members[m].pure) continue;
            members[m].emission_reference = true;
            demand_member(e,MemberDemandReason::Vtable);
        }
        if (polymorphic(cls)) record_rtti_type(entities[cls].type);
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            if (!bases[b].virtual_base && virtual_base_count(bases[b].base)) demand_construction_vtable(bases[b].base);
        virtual_classes[id].construction_demand = FactState::Success;
    } catch (...) {
        virtual_classes[id].construction_demand = FactState::Failure; throw;
    }
}
} }
