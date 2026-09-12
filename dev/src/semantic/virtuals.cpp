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
    members[m].virtual_signature = types.function(types.fundamental(FT_VOID), params, f.variadic, f.cv, f.ref);
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
    EntityId base = direct_base(cls);
    std::uint32_t v = 0;
    if (base && polymorphic(base)) {
        auto inherited = class_facts[entities[base].class_info].virtual_info;
        v = virtual_classes.size(); virtual_classes.push_back(VirtualClass());
        virtual_slot_work += virtual_classes[inherited].slots.size();
        virtual_classes[v].slots = virtual_classes[inherited].slots;
        virtual_classes[v].signatures = virtual_classes[inherited].signatures;
        class_facts[info].virtual_info = v;
    }
    std::vector<EntityId> methods;
    Index seen;
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        EntityId e = declarations[d].entity;
        if (!entities[e].member_info || entities[e].owner != entities[cls].scope || seen.get(e)) continue;
        seen.put(e,1); methods.push_back(e);
    }
    if (base && type_destructor(entities[base].type) && members[entities[type_destructor(entities[base].type)].member_info].virtual_member && !class_facts[info].destructor) {
        EntityId dtor = default_destructor(entities[cls].type, entities[cls].scope, false);
        members[entities[dtor].member_info].virtual_signature = types.function(types.fundamental(FT_VOID), {}, false);
        methods.push_back(dtor);
    }
    for (EntityId e : methods) {
        ++virtual_declaration_work;
        auto m = entities[e].member_info;
        auto k = key(members[m].destructor ? 0 : entities[e].name, members[m].virtual_signature);
        auto slot = v ? virtual_classes[v].signatures.get(k) : 0;
        if (slot) {
            EntityId old = virtual_classes[v].slots[slot-1];
            if (entities[e].is_static || members[entities[old].member_info].final_member) throw std::runtime_error("invalid virtual override");
            check_covariance(e,old);
            if (function_nonthrowing(old) && !function_nonthrowing(e)) throw std::runtime_error("looser virtual exception specification");
            members[m].virtual_member = true;
        }
        if (members[m].override_member && !slot) throw std::runtime_error("override without matching base virtual");
        if ((members[m].pure || members[m].final_member) && !members[m].virtual_member) throw std::runtime_error("pure/final requires virtual member");
        if (!members[m].virtual_member) continue;
        if (!v) { v = virtual_classes.size(); virtual_classes.push_back(VirtualClass()); class_facts[info].virtual_info = v; }
        if (!slot) {
            slot = virtual_classes[v].slots.size()+1;
            virtual_classes[v].signatures.put(k,slot);
            virtual_classes[v].slots.push_back(e);
            if (members[m].destructor) virtual_classes[v].slots.push_back(e);
        } else {
            virtual_classes[v].slots[slot-1] = e;
            if (members[m].destructor) virtual_classes[v].slots[slot] = e;
        }
        members[m].virtual_slot = slot;
        if (!virtual_classes[v].key_function && !members[m].pure && !entities[e].inline_function)
            virtual_classes[v].key_function = e;
    }
    if (v) {
        class_facts[info].aggregate = false;
        for (EntityId e : virtual_classes[v].slots) virtual_classes[v].abstract |= members[entities[e].member_info].pure;
    }
}
void Analyzer::reject_abstract(TypeId t)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (class_value(t) && polymorphic(types[t].entity) && virtual_class(types[t].entity).abstract)
        throw std::runtime_error("abstract class value");
}
} }

namespace cppgm { namespace semantic {
void Analyzer::demand_vtable(EntityId cls)
{
    if (!polymorphic(cls)) return;
    auto v = class_facts[entities[cls].class_info].virtual_info;
    if (virtual_classes[v].demanded) return;
    virtual_classes[v].demanded = true; ++virtual_demands;
    // Copy compact IDs because demanded bodies may introduce local classes.
    auto slots = virtual_classes[v].slots;
    EntityId previous = 0;
    for (EntityId e : slots) {
        ++virtual_slot_work;
        if (e == previous) continue;
        previous = e;
        auto m = entities[e].member_info;
        if (members[m].pure) continue;
        members[m].emission_reference = true;
        demand_member(e);
        if (members[m].destructor) {
            members[m].complete_entry = true;
            EntityId deallocation = select_deallocation(entities[cls].type,false,false,entities[e].owner);
            members[m].deleting_deallocation = deallocation;
        }
    }
}
} }
