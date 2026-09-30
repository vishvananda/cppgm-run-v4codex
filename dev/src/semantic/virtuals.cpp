#include "semantic/analyzer.h"
#include <stdexcept>
#include <cstring>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::virtual_declaration(EntityId e, NodeId d, NodeId init, NodeId specs, NodeId source, ScopeId s)
{
    bool virt = spec_has(specs, KW_VIRTUAL) || spec_has(child(source, Kind::MemberSpecifiers), KW_VIRTUAL);
    bool over = false, final = false;
    for (NodeId n = ast[d].first; n; n = ast[n].next) if (ast[n].kind == Kind::VirtSpecifier) {
        auto text = ids.spelling(ast[n].text);
        over |= (text.size == 8 && !std::memcmp(text.data,"override",8)); final |= (text.size == 5 && !std::memcmp(text.data,"final",5));
    }
    bool member = entities[e].member_info;
    if ((virt || over || final) && (!member || entities[e].is_static || members[entities[e].member_info].constructor))
        throw std::runtime_error("virtual specifier requires a nonstatic ordinary member or destructor");
    if (!member) return;
    auto m = entities[e].member_info;
    if (virt && s != entities[e].owner) throw std::runtime_error("virtual on out-of-class definition");
    members[m].virtual_member |= virt;
    members[m].override_member |= over; members[m].final_member |= final;
    if (!init) init = child(source, Kind::Initializer);
    if (init && !child(init, Kind::SpecialInitializer)) {
        auto value = evaluate(ast[init].first, s);
        if (!value.valid || value.bits) throw std::runtime_error("invalid pure specifier");
        members[m].pure = true;
    }
    Type f = types[entities[e].type];
    std::vector<TypeId> params(types.parameters.begin()+f.offset, types.parameters.begin()+f.offset+f.count);
    // A conversion-type-id is part of a conversion function's name. Preserve
    // that canonical type in the shape; ordinary returns do not identify slots.
    members[m].virtual_signature = types.function(members[m].conversion_target ?
        members[m].conversion_target : types.fundamental(FT_VOID), params, f.variadic, f.cv, f.ref);
}
void Analyzer::check_covariance(EntityId e, EntityId base)
{
    TypeId result = types[entities[e].type].child, old = types[entities[base].type].child;
    if (result == old) return;
    Type r = types[result], b = types[old];
    if (r.kind != b.kind || (r.kind != TypeKind::Pointer && r.kind != TypeKind::LRef && r.kind != TypeKind::RRef) || r.cv != b.cv)
        throw std::runtime_error("incompatible virtual return type");
    r = types[r.child]; b = types[b.child];
    if (r.kind != TypeKind::Named || b.kind != TypeKind::Named || !entities[r.entity].class_info || !entities[b.entity].class_info ||
        (r.cv & ~b.cv) || !class_derives(r.entity,b.entity)) throw std::runtime_error("invalid covariance");
    if (r.entity != scopes[entities[e].owner].entity && !entities[r.entity].complete) throw std::runtime_error("incomplete covariant class");
    check_base_access(entities[r.entity].type, entities[b.entity].type, entities[e].owner);
}
void Analyzer::complete_virtuals(EntityId cls)
{
    auto info = entities[cls].class_info;
    // Import subobject identities, not a signature-only union: unrelated
    // roots can have the same signature and distinct final overriders.
    complete_virtual_bases(cls);
    VirtualClass completed;
    std::vector<VirtualSlot> primary;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
        if (!dynamic_class(bases[b].base) || bases[b].virtual_base) continue;
        class_facts[info].primary_base = b;
        const auto& inherited = virtual_class(bases[b].base);
        primary.assign(inherited.slots.begin(),inherited.slots.begin()+inherited.primary_count);
        completed.slots.assign(inherited.slots.begin()+inherited.primary_count,inherited.slots.end());
        for (auto& slot : primary) {
            slot.origin = prefix_subobject(b,slot.origin);
            slot.implementation = prefix_subobject(b,slot.implementation);
        }
        for (auto& slot : completed.slots) {
            slot.origin = prefix_subobject(b,slot.origin);
            slot.implementation = prefix_subobject(b,slot.implementation);
        }
        completed.views = inherited.views;
        for (auto& view : completed.views) {
            view.begin -= inherited.primary_count;
            view.subobject = prefix_subobject(b,view.subobject);
        }
        completed.signatures = inherited.signatures;
        virtual_slot_work += inherited.slots.size();
        break;
    }
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
        auto base = bases[b].base;
        if (!dynamic_class(base) || b == class_facts[info].primary_base) continue;
        const auto& inherited = virtual_class(base);
        unsigned root = completed.views.size()+1;
        auto import = [&](VirtualView view, unsigned begin, unsigned count) {
            view.subobject = prefix_subobject(b,view.subobject);
            view.begin = completed.slots.size(); view.count = count;
            for (unsigned j = 0; j < count; ++j) {
                auto slot = inherited.slots[begin+j]; slot.receiver += root;
                slot.origin = prefix_subobject(b,slot.origin);
                slot.implementation = prefix_subobject(b,slot.implementation);
                completed.slots.push_back(slot); ++virtual_slot_work;
            }
            completed.views.push_back(view);
        };
        VirtualView view; view.type = base; view.edge = b;
        import(view,0,inherited.primary_count);
        for (const auto& old : inherited.views) {
            auto copy = old; copy.parent += root;
            import(copy,old.begin,old.count);
        }
        // Primary-chain aliases need tables only in a secondary subobject.
        // Keep single-inheritance completion proportional to its real slots.
        auto parent = root;
        for (auto edge = class_facts[entities[base].class_info].primary_base; edge;
            edge = class_facts[entities[bases[edge].base].class_info].primary_base) {
            VirtualView alias; alias.type = bases[edge].base; alias.edge = edge; alias.parent = parent;
            auto count = virtual_class(alias.type).primary_count;
            import(alias,0,count);
            parent = completed.views.size();
        }
    }
    std::vector<EntityId> methods;
    Index seen, overrides;
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (!entities[e].member_info || entities[e].owner != entities[cls].scope || seen.get(e)) continue;
        seen.put(e,1); methods.push_back(e);
    }
    bool virtual_destructor = false;
    for (auto b = class_facts[info].first_base; b; b = bases[b].next) {
        auto dtor = type_destructor(entities[bases[b].base].type);
        virtual_destructor |= dtor && members[entities[dtor].member_info].virtual_member;
    }
    if (virtual_destructor && !class_facts[info].destructor) {
        EntityId dtor = destructor_declaration(entities[cls].type);
        members[entities[dtor].member_info].virtual_signature = types.function(types.fundamental(FT_VOID), {}, false);
        methods.push_back(dtor);
    }
    auto shape = [&](EntityId e) {
        auto m = entities[e].member_info;
        return key(members[m].destructor ? 0 : entities[e].name,members[m].virtual_signature);
    };
    for (auto e : methods) overrides.put(shape(e),e);
    Index matched;
    auto replace = [&](std::vector<VirtualSlot>& slots) {
        for (auto& slot : slots) {
            ++virtual_slot_work;
            EntityId old = slot.function, e = overrides.get(shape(old));
            if (!e) continue;
            auto m = entities[e].member_info;
            if (entities[e].is_static || members[entities[old].member_info].final_member)
                throw std::runtime_error("invalid virtual override");
            check_covariance(e,old);
            if (function_nonthrowing(old) && !function_nonthrowing(e))
                throw std::runtime_error("looser virtual exception specification");
            matched.put(e,1); members[m].virtual_member = true;
            slot.function = e; slot.receiver = 0; slot.implementation = 0;
        }
    };
    replace(primary); replace(completed.slots);
    for (EntityId e : methods) {
        ++virtual_declaration_work;
        auto m = entities[e].member_info;
        if (members[m].override_member && !matched.get(e)) throw std::runtime_error("override without matching base virtual");
        if ((members[m].pure || members[m].final_member) && !members[m].virtual_member) throw std::runtime_error("pure/final requires virtual member");
        if (!members[m].virtual_member) continue;
        auto k = shape(e), slot = std::uint64_t(completed.signatures.get(k));
        if (!slot) {
            slot = primary.size()+1;
            completed.signatures.put(k,slot);
            primary.push_back(VirtualSlot(e));
            if (members[m].destructor) primary.push_back(VirtualSlot(e));
        }
        members[m].virtual_slot = slot;
        if (!completed.key_function && !members[m].pure && !entities[e].inline_function)
            completed.key_function = e;
    }
    if (primary.empty() && completed.views.empty() && !virtual_base_count(cls)) return;
    completed.polymorphic = !primary.empty() || !completed.slots.empty();
    completed.primary_count = primary.size();
    for (auto& view : completed.views) view.begin += completed.primary_count;
    completed.slots.insert(completed.slots.begin(),primary.begin(),primary.end());
    resolve_final_overriders(cls,completed);
    // An inherited override introduced in a secondary view still needs its
    // callable slot in this class's primary group. Keep unrelated inherited
    // roots separate; only actual overrides add a new primary signature.
    std::vector<VirtualSlot> inherited_overrides;
    for (unsigned j = completed.primary_count; j < completed.slots.size(); ++j) {
        auto slot = completed.slots[j];
        auto e = slot.function;
        if (e == slot.declaration || scopes[entities[e].owner].entity == cls || completed.signatures.get(shape(e))) continue;
        completed.signatures.put(shape(e),completed.primary_count+inherited_overrides.size()+1);
        slot.declaration = e; slot.origin = slot.implementation;
        inherited_overrides.push_back(slot);
        if (members[entities[e].member_info].destructor) inherited_overrides.push_back(slot);
    }
    if (!inherited_overrides.empty()) {
        completed.slots.insert(completed.slots.begin()+completed.primary_count,inherited_overrides.begin(),inherited_overrides.end());
        completed.primary_count += inherited_overrides.size();
        for (auto& view : completed.views) view.begin += inherited_overrides.size();
    }
    if (virtual_base_count(cls)) {
        // Every path must participate in final-overrider checking above. Once
        // resolved, a shared physical view has one immutable slot slice. Do
        // not propagate path multiplicity into the next derived class: nested
        // virtual diamonds otherwise duplicate facts exponentially.
        Index identities;
        std::vector<unsigned> remap(completed.views.size()+1);
        std::vector<VirtualView> views;
        std::vector<VirtualSlot> slots(completed.slots.begin(),completed.slots.begin()+completed.primary_count);
        for (unsigned j = 0; j < completed.views.size(); ++j) {
            auto view = completed.views[j]; auto identity = key(view.subobject,view.type);
            if (auto known = identities.get(identity)) { remap[j+1] = known; continue; }
            remap[j+1] = views.size()+1; identities.put(identity,remap[j+1]);
            auto begin = view.begin; view.begin = slots.size(); view.parent = remap[view.parent];
            slots.insert(slots.end(),completed.slots.begin()+begin,completed.slots.begin()+begin+view.count);
            views.push_back(view);
        }
        for (auto& slot : slots) slot.receiver = remap[slot.receiver];
        completed.views.swap(views); completed.slots.swap(slots);
    }
    for (const auto& slot : completed.slots) completed.abstract |= members[entities[slot.function].member_info].pure;
    class_facts[info].aggregate = false;
    auto v = virtual_classes.size(); class_facts[info].virtual_info = v;
    virtual_classes.push_back(std::move(completed));
    if (virtual_classes[v].key_function) vtable_definition_available(virtual_classes[v].key_function);
}
bool Analyzer::abstract_value(TypeId t)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (definitions && class_value(t)) complete_class(types[t].entity);
    return class_value(t) && polymorphic(types[t].entity) && virtual_class(types[t].entity).abstract;
}
void Analyzer::reject_abstract(TypeId t)
{
    if (abstract_value(t)) throw std::runtime_error("abstract class value");
}
} }

namespace cppgm { namespace semantic {
void Analyzer::vtable_definition_available(EntityId e)
{
    if (!calls) return;
    EntityId cls = scopes[entities[e].owner].entity;
    if (!dynamic_class(cls)) return;
    auto v = virtual_class_id(cls);
    // The owning class is the reverse dependency of its one selected key.
    // Availability is distinct from checking the body; defer the consumer until
    // finish so later source declarations can satisfy its outgoing demands.
    auto bit = static_cast<unsigned char>(VtableReason::KeyDefinition);
    if (virtual_classes[v].key_function != e || (virtual_classes[v].reasons & bit) ||
        (!entities[e].body && !members[entities[e].member_info].body)) return;
    virtual_classes[v].reasons |= bit;
    if (virtual_classes[v].demand == FactState::NotStarted) key_vtable_demand.push_back(cls);
}
unsigned Analyzer::virtual_dispatch(EntityId e)
{
    auto slot = members[entities[e].member_info].virtual_slot;
    auto cls = scopes[entities[e].owner].entity;
    if (slot && !unevaluated_depth && virtual_base_count(cls)) demand_vtable(cls,VtableReason::Dispatch);
    return slot;
}
void Analyzer::demand_vtable(EntityId cls, VtableReason reason)
{
    if (!dynamic_class(cls)) return;
    auto v = class_facts[entities[cls].class_info].virtual_info;
    virtual_classes[v].reasons |= static_cast<unsigned char>(reason);
    if (!virtual_classes[v].referenced) {
        virtual_classes[v].referenced = true;
        vtable_emission.push_back(cls);
    }
    // A key declaration fixes external ownership until its definition arrives.
    // Its reverse dependency wakes this one table; referencing an external
    // table must not instantiate its slots or their RTTI as a side effect.
    auto key_function = virtual_classes[v].key_function;
    if (key_function && !entities[key_function].body && !members[entities[key_function].member_info].body &&
        !entities[cls].specialization) return;
    if (virtual_classes[v].demand == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::Vtable,cls,entities[cls].source);
    if (virtual_classes[v].demand != FactState::NotStarted) return;
    virtual_classes[v].demand = FactState::Active; ++virtual_demands;
    try {
    // Slot identities are immutable after class completion. Reacquire by ID
    // because outgoing member demands may relocate the outer class vector.
    // Collect identities before member demand, which may relocate class facts.
    std::vector<EntityId> demanded;
    Index seen;
    auto collect = [&](const std::vector<VirtualSlot>& slots) {
        for (const auto& slot : slots) {
            ++virtual_slot_work;
            if (!seen.get(slot.function)) { seen.put(slot.function,1); demanded.push_back(slot.function); }
        }
    };
    collect(virtual_classes[v].slots);
    for (EntityId e : demanded) {
        auto m = entities[e].member_info;
        if (members[m].pure) continue;
        members[m].emission_reference = true;
        demand_member(e, MemberDemandReason::Vtable);
        if (members[m].destructor) {
            members[m].complete_entry = true;
            EntityId deallocation = select_deallocation(entities[cls].type,false,false,entities[e].owner);
            members[m].deleting_deallocation = deallocation;
        }
    }
    if (reason == VtableReason::Dispatch)
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next)
            demand_vtable(bases[b].base,reason);
    record_rtti_type(entities[cls].type);
    virtual_classes[v].demand = FactState::Success;
    } catch (...) {
        virtual_classes[v].demand = FactState::Failure; throw;
    }
}
} }

namespace cppgm { namespace semantic {
void Analyzer::layout_virtual_views(EntityId cls)
{
    auto id = virtual_class_id(cls);
    Index stored;
    virtual_classes[id].address_point = 16+std::uint64_t(virtual_base_count(cls))*8;
    std::uint64_t group_end = virtual_classes[id].address_point + std::uint64_t(virtual_classes[id].primary_count)*8;
    for (unsigned j = 0; j < virtual_classes[id].views.size(); ++j) {
        auto& view = virtual_classes[id].views[j];
        view.offset = bases[view.edge].virtual_base ? virtual_base_offset(cls,bases[view.edge].base) :
            (view.parent ? virtual_classes[id].views[view.parent-1].offset : 0) + bases[view.edge].offset;
        view.virtual_anchor = bases[view.edge].virtual_base ? bases[view.edge].base :
            view.parent ? virtual_classes[id].views[view.parent-1].virtual_anchor : 0;
        view.virtual_tail = view.virtual_anchor ? view.offset-virtual_base_offset(cls,view.virtual_anchor) : 0;
        view.vbase_rows = virtual_base_count(view.type);
        view.vcall_rows = 0;
        if (bases[view.edge].virtual_base) {
            Index signatures;
            for (unsigned k = 0; k < view.count; ++k) {
                ++virtual_slot_work;
                auto e = virtual_classes[id].slots[view.begin+k].declaration;
                auto member = members[entities[e].member_info];
                auto shape = key(member.destructor ? 0 : entities[e].name,member.virtual_signature);
                if (!signatures.get(shape)) { signatures.put(shape,1); ++view.vcall_rows; }
            }
        }
        view.address_point = 16+std::uint64_t(view.vcall_rows+view.vbase_rows)*8;
        view.store = view.offset && !stored.get(view.offset);
        if (view.store) {
            view.group_address_point = group_end + view.address_point;
            stored.put(view.offset,j+1);
            group_end += view.address_point + std::uint64_t(view.count)*8;
        } else view.group_address_point = view.offset ? virtual_classes[id].views[stored.get(view.offset)-1].group_address_point : virtual_classes[id].address_point;
    }
    auto adjust = [&](unsigned begin, unsigned count, std::uint64_t offset) {
        for (unsigned j = 0; j < count; ++j) {
            auto slot = virtual_classes[id].slots[begin+j];
            slot.this_adjustment = std::int64_t(slot.receiver ? virtual_classes[id].views[slot.receiver-1].offset : 0) - std::int64_t(offset);
            auto actual = types[entities[slot.function].type].child;
            auto expected = types[entities[slot.declaration].type].child;
            if (actual != expected) {
                auto derived = types[actual].child, base = types[expected].child;
                auto path = base_adjustments[base_steps(derived,types[base].entity)];
                slot.result_virtual_row = path.virtual_row;
                slot.result_adjustment = path.virtual_row ? path.virtual_tail : path.total;
            }
            // A return-type layout can complete a deferred class and relocate
            // the class arena. Publish by identity after that outgoing demand.
            virtual_classes[id].slots[begin+j] = slot;
        }
    };
    adjust(0,virtual_classes[id].primary_count,0);
    auto views = virtual_classes[id].views.size();
    for (unsigned j = 0; j < views; ++j) {
        auto view = virtual_classes[id].views[j];
        adjust(view.begin,view.count,view.offset);
    }
    auto& v = virtual_classes[id];
    auto count = v.primary_count;
    std::vector<VirtualSlot> additional;
    for (unsigned j = 0; j < count; ++j) {
        auto slot = v.slots[j];
        auto e = slot.function, m = entities[e].member_info;
        if ((!slot.result_adjustment && !slot.result_virtual_row) || scopes[entities[e].owner].entity != cls || members[m].virtual_slot != j+1) continue;
        // The inherited slot still returns the base view; a call naming the
        // overriding declaration needs its unadjusted covariant result.
        members[m].virtual_slot = count+additional.size()+1;
        v.signatures.put(key(entities[e].name,members[m].virtual_signature),members[m].virtual_slot);
        additional.push_back(VirtualSlot(e));
    }
    if (!additional.empty()) {
        v.slots.insert(v.slots.begin()+count,additional.begin(),additional.end());
        for (auto& view : v.views) view.begin += additional.size();
        v.primary_count += additional.size();
        for (auto& view : v.views) if (view.offset) view.group_address_point += additional.size()*8;
    }
}
} }
