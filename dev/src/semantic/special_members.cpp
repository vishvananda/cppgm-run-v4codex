#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::transfer_member(EntityId e) const
{ return e && entities[e].member_info && members[entities[e].member_info].transfer != TransferKind::None; }
bool Analyzer::trivial_transfer(EntityId e) const
{ return transfer_member(e) && members[entities[e].member_info].transfer_trivial; }
bool Analyzer::direct_transfer(EntityId e) const
{ return transfer_member(e) && members[entities[e].member_info].transfer_direct; }
bool Analyzer::copy_storage_type(TypeId t)
{
    auto info = entities[types[t].entity].class_info;
    if (class_facts[info].copy_storage_state) return class_facts[info].copy_storage_state == 3;
    class_facts[info].copy_storage_state = 1;
    EntityId copy = select_transfer(t, types.qualify(t, 1), ValueCategory::Lvalue, false);
    bool simple = copy && !deleted_transfer(copy) && trivial_transfer(copy);
    class_facts[info].copy_storage_state = simple ? 3 : 2;
    return simple;
}
void Analyzer::classify_transfer(EntityId e, NodeId special, ScopeId context)
{
    auto m = entities[e].member_info;
    EntityId cls = scopes[entities[e].owner].entity;
    auto info = entities[cls].class_info;
    Type f = types[entities[e].type];
    bool ctor = members[m].constructor, assignment = entities[e].key == OP_ASS;
    if (ctor) class_facts[info].user_constructor = true;
    if (members[m].destructor) class_facts[info].user_destructor = true;
    if ((ctor || assignment) && f.count && !entities[e].template_info) {
        TypeId p = types.parameters[f.offset]; Type param = types[p];
        bool ref = param.kind == TypeKind::LRef || param.kind == TypeKind::RRef;
        TypeId object = ref ? param.child : p;
        bool remaining_defaulted = f.count == 1 || (ctor && entities[e].defaults && default_arguments[entities[e].defaults+1]);
        if (types.unqualified(object) == entities[cls].type && remaining_defaulted && (ref || assignment)) {
            bool move = param.kind == TypeKind::RRef;
            members[m].transfer = ctor ? (move ? TransferKind::MoveConstructor : TransferKind::CopyConstructor) :
                (move ? TransferKind::MoveAssignment : TransferKind::CopyAssignment);
            class_facts[info].declared_transfers |= unsigned(members[m].transfer);
        }
    }
    if (!special || ast[special].op != KW_DEFAULT) return;
    if (!ctor && !members[m].destructor && !transfer_member(e)) throw std::runtime_error("only special members may be defaulted");
    if (ctor && f.count && !transfer_member(e)) throw std::runtime_error("only special constructors may be defaulted");
    for (unsigned j = 0; entities[e].defaults && j < f.count; ++j)
        if (default_arguments[entities[e].defaults+j]) throw std::runtime_error("defaulted function cannot have default arguments");
    if (assignment) {
        TypeId expected = types.compound(TypeKind::LRef, entities[cls].type);
        if (f.child != expected || f.cv || f.variadic || f.count != 1 ||
            (types[types.parameters[f.offset]].kind != TypeKind::LRef && types[types.parameters[f.offset]].kind != TypeKind::RRef))
            throw std::runtime_error("invalid defaulted assignment signature");
    }
    members[m].synthetic = true;
    members[m].defaulted_late = context != entities[e].owner;
    if (!members[m].defaulted_late) entities[e].inline_function = true;
    else { members[m].retained_root = true; demand_member(e); }
}
Conversion Analyzer::transfer_conversion(TypeId from, ValueCategory category, TypeId to)
{
    Conversion c; c.target = to;
    Type target = types[to];
    if (target.kind != TypeKind::LRef && target.kind != TypeKind::RRef) {
        if (types.unqualified(from) == types.unqualified(to)) c.rank = 0;
        return c;
    }
    bool rvalue = category != ValueCategory::Lvalue;
    if ((target.kind == TypeKind::RRef && !rvalue) ||
        (target.kind == TypeKind::LRef && rvalue && types[target.child].cv != 1) ||
        (types[from].cv & ~types[target.child].cv)) return c;
    bool derived = derived_from(from, target.child);
    if (types.unqualified(from) != types.unqualified(target.child) && !derived) return c;
    c.rank = derived ? 2 : 0; c.reference = true; c.derived = derived;
    c.qualification = types[target.child].cv & ~types[from].cv;
    c.preference = rvalue && target.kind == TypeKind::LRef;
    return c;
}
void Analyzer::ensure_transfers(TypeId t, bool assignment)
{
    EntityId cls = types[t].entity;
    auto info = entities[cls].class_info;
    unsigned mask = assignment ? 12 : 3;
    if ((class_facts[info].generated_transfers & mask) == mask) return;
    class_facts[info].generated_transfers |= mask;
    unsigned declared = class_facts[info].declared_transfers;
    bool implicit_move = !declared && !class_facts[info].user_destructor;
    bool const_source = true;
    auto accepts_const = [&](TypeId subobject) {
        while (types[subobject].kind == TypeKind::Array) subobject = types[subobject].child;
        if (types[subobject].kind != TypeKind::Named || !entities[types[subobject].entity].class_info) return;
        EntityId selected = select_transfer(subobject, types.qualify(subobject, 1), ValueCategory::Lvalue, assignment);
        if (!selected) const_source = false;
    };
    unsigned copy = assignment ? 4 : 1, move = assignment ? 8 : 2;
    if (!(declared & copy)) {
        for (auto b = class_facts[info].first_base; b; b = bases[b].next) accepts_const(entities[bases[b].base].type);
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == entities[cls].scope) accepts_const(entities[field].type);
        }
    }
    for (unsigned kind : {copy, move}) {
        if ((declared & kind) || (kind == move && !implicit_move)) continue;
        TypeId parameter = types.compound(kind == move ? TypeKind::RRef : TypeKind::LRef,
            types.qualify(entities[cls].type, kind == copy && const_source ? 1 : 0));
        TypeId result = assignment ? types.compound(TypeKind::LRef, entities[cls].type) : types.fundamental(FT_VOID);
        auto name = assignment ? operator_name(OP_ASS) : entities[cls].name;
        EntityId e = declare_function(entities[cls].scope, name, 0, types.function(result, {parameter}, false), !assignment);
        entities[e].access = Access::Public;
        entities[e].inline_function = true;
        if (assignment) entities[e].key = OP_ASS;
        member_facts(e);
        auto m = entities[e].member_info;
        members[m].synthetic = true; members[m].constructor = !assignment;
        members[m].transfer = TransferKind(kind);
        members[m].deleted = kind == copy && (declared & (2|8));
        if (!assignment) class_facts[info].constructor = merge_lookup(class_facts[info].constructor, e);
    }
}
EntityId Analyzer::select_transfer(TypeId target, TypeId source, ValueCategory category, bool assignment)
{
    ensure_transfers(target, assignment);
    EntityId cls = types[target].entity;
    EntityId family = assignment ? local(entities[cls].scope, operator_name(OP_ASS)) : class_facts[entities[cls].class_info].constructor;
    struct Candidate { EntityId entity; Conversion sequence[2]; };
    std::vector<Candidate> viable;
    for (EntityId e : candidates(family)) {
        Type f = types[entities[e].type];
        if (!f.count || (f.count > 1 && (!entities[e].defaults || !default_arguments[entities[e].defaults+1]))) continue;
        ++candidate_work;
        Candidate c; c.entity = e;
        c.sequence[0] = assignment ? object_conversion(e, target, ValueCategory::Lvalue) : Conversion();
        if (!assignment) c.sequence[0].rank = 0;
        c.sequence[1] = transfer_conversion(source, category, types.parameters[f.offset]);
        if (!c.sequence[0].valid() || !c.sequence[1].valid()) continue;
        auto m = entities[e].member_info;
        bool move = members[m].transfer == TransferKind::MoveConstructor || members[m].transfer == TransferKind::MoveAssignment;
        if (move && members[m].synthetic && deleted_transfer(e)) continue;
        viable.push_back(c);
    }
    if (viable.empty()) return 0;
    unsigned best = 0;
    for (unsigned j = 1; j < viable.size(); ++j) if (better(viable[j].sequence, viable[best].sequence, 2)) best = j;
    for (unsigned j = 0; j < viable.size(); ++j)
        if (j != best && !better(viable[best].sequence, viable[j].sequence, 2)) return 0;
    return viable[best].entity;
}
bool Analyzer::deleted_transfer(EntityId e)
{
    if (transfer_member(e) && members[entities[e].member_info].synthetic) prepare_transfer(e);
    return entities[e].member_info && members[entities[e].member_info].deleted;
}
bool Analyzer::transfer_accessible(EntityId e, ScopeId context) const
{
    EntityId owner = scopes[entities[e].owner].entity;
    if (entities[e].access == Access::Public || privileged(context, owner)) return true;
    if (entities[e].access == Access::Protected)
        for (auto s = context; s; s = scopes[s].parent)
            if (scopes[s].kind == ScopeKind::Class && class_derives(scopes[s].entity, owner)) return true;
    return false;
}
} }
