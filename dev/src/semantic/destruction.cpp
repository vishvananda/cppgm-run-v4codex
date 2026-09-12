#include "semantic/analyzer.h"
#include <stdexcept>
#include <algorithm>
namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::converted_temporary(const Conversion& c) const
{
    if (c.kind == Conversion::Kind::Construction) return conversion_objects[c.materialization].temporary;
    if (c.kind == Conversion::Kind::User) return user_conversions[c.materialization].temporary;
    if (c.kind == Conversion::Kind::List) return list_objects[c.materialization].temporary;
    return 0;
}
EntityId Analyzer::bound_temporary(NodeId n) const
{
    auto c = conversions[expressions[n].incoming];
    if (c.reference) {
        if (auto temporary = converted_temporary(c)) return temporary;
        if (c.temporary || c.kind == Conversion::Kind::User) return 0;
    }
    if (auto temporary = object_fact(n).temporary) return temporary;
    if (auto operand = reference_operand(n)) return bound_temporary(operand);
    return 0;
}
EntityId Analyzer::type_destructor(TypeId t) const
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    return types[t].kind == TypeKind::Named && entities[types[t].entity].class_info ? class_facts[entities[types[t].entity].class_info].destructor : 0;
}
bool Analyzer::destructor_member(EntityId e) const
{
    return e && entities[e].member_info && members[entities[e].member_info].destructor;
}
EntityId Analyzer::default_destructor(TypeId t, ScopeId s, bool demand)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return 0;
    EntityId cls = types[t].entity;
    auto c = entities[cls].class_info;
    EntityId dtor = class_facts[c].destructor;
    if (!dtor) {
        TextView text = ids.spelling(entities[cls].name);
        std::string name = "~" + std::string(text.data, text.size);
        dtor = make_entity(EntityKind::Function, entities[cls].scope, ids.intern(TextView(name.data(), name.size())), 0);
        entities[dtor].type = types.function(types.fundamental(FT_VOID), {}, false);
        entities[dtor].inline_function = true;
        member_facts(dtor);
        members[entities[dtor].member_info].synthetic = true;
        members[entities[dtor].member_info].destructor = true;
        class_facts[c].destructor = dtor;
    }
    if (members[entities[dtor].member_info].deleted) throw std::runtime_error("deleted destructor");
    check_access(dtor, s, entities[dtor].owner);
    if (demand) demand_member(dtor);
    return dtor;
}
void Analyzer::register_destruction(EntityId e)
{
    auto kind = types[entities[e].type].kind;
    if ((kind == TypeKind::LRef || kind == TypeKind::RRef) && scopes[entities[e].owner].kind == ScopeKind::Namespace && !entities[e].thread_local_storage)
        static_reference(e);
    if ((kind == TypeKind::LRef || kind == TypeKind::RRef) && !entities[e].is_static &&
        (scopes[entities[e].owner].kind == ScopeKind::Block || scopes[entities[e].owner].kind == ScopeKind::Control)) {
        NodeId n = entities[e].initializer;
        while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::Parenthesized || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) n = ast[n].first;
        local_reference(n,e);
        if (reference_temporary(e) || reference_choices(e)) return;
    }
    EntityId dtor = default_destructor(entities[e].type, entities[e].owner);
    if (dtor) object_destructors.put(e, dtor);
    if (dtor && entities[e].kind == EntityKind::Parameter && !trivial_destructor(entities[e].type))
        members[entities[dtor].member_info].retained_root = true;
}
bool Analyzer::trivial_destructor(TypeId t)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return true;
    EntityId cls = types[t].entity;
    auto c = entities[cls].class_info;
    if (class_facts[c].trivial_destructor_state) return class_facts[c].trivial_destructor_state == 3;
    class_facts[c].trivial_destructor_state = 1;
    EntityId dtor = class_facts[c].destructor;
    bool trivial = !dtor || (members[entities[dtor].member_info].synthetic && !members[entities[dtor].member_info].defaulted_late);
    for (auto d = scopes[entities[cls].scope].first_decl; trivial && d; d = declarations[d].next) {
        EntityId field = declarations[d].entity;
        if (nonstatic_field(field) && entities[field].owner == entities[cls].scope) trivial = trivial_destructor(entities[field].type);
    }
    for (auto b = class_facts[c].first_base; trivial && b; b = bases[b].next) trivial = trivial_destructor(entities[bases[b].base].type);
    class_facts[c].trivial_destructor_state = trivial ? 3 : 2;
    return trivial;
}
void Analyzer::destructor_actions(EntityId e)
{
    auto m = entities[e].member_info;
    if (members[m].actions_ready) return;
    members[m].actions_ready = true;
    EntityId cls = scopes[entities[e].owner].entity;
    if (!entities[e].scope) entities[e].scope = make_scope(ScopeKind::Function, entities[e].owner, entities[e].name, e);
    size(entities[cls].type);
    std::vector<DestructionAction> work;
    // Anonymous union storage represents the enclosing class's variants; it
    // is not an independently invoked union destructor in a user body.
    if (entities[cls].key == KW_UNION && members[m].synthetic && !class_facts[entities[cls].class_info].storage)
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (nonstatic_field(field) && entities[field].owner == entities[cls].scope && !trivial_destructor(entities[field].type)) {
                members[m].deleted = true;
                throw std::runtime_error("defaulted union destructor has a nontrivial variant");
            }
        }
    if (entities[cls].key != KW_UNION) {
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope) continue;
            EntityId dtor = default_destructor(entities[field].type, entities[e].scope);
            if (dtor) work.push_back({field, entities[field].type, dtor});
        }
        std::reverse(work.begin(), work.end());
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
            TypeId base = entities[bases[b].base].type;
            EntityId dtor = default_destructor(base, entities[e].scope);
            members[entities[dtor].member_info].base_entry = true;
            work.push_back({0, base, dtor});
        }
    }
    members[m].destruction_begin = destruction_actions.size(); members[m].destruction_count = work.size();
    destruction_actions.insert(destruction_actions.end(), work.begin(), work.end());
}
bool Analyzer::destructor_needed(EntityId e)
{
    if (!e) return false;
    auto m = entities[e].member_info;
    if (!members[m].synthetic && (!members[m].body || ast[members[m].body].kind != Kind::Compound || ast[members[m].body].first)) return true;
    if (members[m].destruction_state == 2) return members[m].destruction_needed;
    if (members[m].destruction_state == 1) throw std::logic_error("cyclic destruction actions");
    members[m].destruction_state = 1;
    bool needed = false;
    EntityId cls = scopes[entities[e].owner].entity;
    // O0 keeps an explicit union boundary when an inactive variant has an
    // effectful destructor. This is a conservative emission policy: it does
    // not add variant destruction to the union's action list.
    if (!members[m].synthetic && entities[cls].key == KW_UNION)
        needed = variant_destruction_effects(entities[cls].type);
    for (unsigned j = 0; j < members[m].destruction_count; ++j)
        needed |= destructor_needed(destruction_actions[members[m].destruction_begin+j].destructor);
    members[m].destruction_state = 2; members[m].destruction_needed = needed;
    // A defined empty body with effect-free subobjects needs no call, but its
    // externally visible ABI entry remains available to other translation units.
    if (!needed && !trivial_destructor(entities[cls].type)) members[m].retained_root = true;
    return needed;
}
bool Analyzer::temporary_cleanup(EntityId object)
{
    return object && object_destructor(object) && !trivial_destructor(entities[object].type);
}
bool Analyzer::variant_destruction_effects(TypeId t)
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return false;
    EntityId cls = types[t].entity;
    if (auto state = variant_destruction_index.get(cls)) return state != 2;
    variant_destruction_index.put(cls, 1);
    EntityId dtor = type_destructor(t);
    bool effects = false;
    if (dtor && !members[entities[dtor].member_info].synthetic) {
        auto body = members[entities[dtor].member_info].body;
        effects = (entities[dtor].exception_spec & 3) || !body || ast[body].kind != Kind::Compound || ast[body].first;
    }
    for (auto d = scopes[entities[cls].scope].first_decl; d && !effects; d = declarations[d].next) {
        EntityId field = declarations[d].entity;
        if (nonstatic_field(field) && entities[field].owner == entities[cls].scope)
            effects = variant_destruction_effects(entities[field].type);
    }
    for (auto b = class_facts[entities[cls].class_info].first_base; b && !effects; b = bases[b].next)
        effects = variant_destruction_effects(entities[bases[b].base].type);
    variant_destruction_index.put(cls, effects ? 3 : 2);
    return effects;
}
void Analyzer::exception_specification(EntityId e, NodeId d, ScopeId s)
{
    unsigned char spec = 0;
    for (NodeId c = ast[d].first; c; c = ast[c].next) {
        if (ast[c].kind != Kind::FunctionQualifier || ast[c].op != KW_NOEXCEPT) continue;
        if (!ast[c].first) spec = 1;
        else {
            Constant value = evaluate(ast[c].first, s);
            if (!value.valid) throw std::runtime_error("nonconstant noexcept specification");
            spec = value.bits ? 3 : 2;
        }
    }
    auto old = entities[e].exception_spec;
    if (!spec && entities[e].key == KW_DELETE) spec = 1;
    unsigned previous = old & 3;
    bool destructor = ast[ast[decl_name(d)].last].op == OP_COMPL;
    bool prior_throwing = previous == 0 || previous == 2;
    bool current_throwing = spec == 0 || spec == 2;
    if ((old & 128) && (!destructor || (previous && spec)) && prior_throwing != current_throwing)
        throw std::runtime_error("conflicting exception specifications");
    entities[e].exception_spec = 128 | (spec ? spec : destructor ? previous : 0);
}
bool Analyzer::function_nonthrowing(EntityId e)
{
    auto spec = entities[e].exception_spec & 3;
    if (spec) return spec == 1 || spec == 3;
    if (transfer_member(e) && members[entities[e].member_info].synthetic) {
        prepare_transfer(e); return members[entities[e].member_info].transfer_noexcept;
    }
    if (!destructor_member(e)) return false;
    auto m = entities[e].member_info;
    if (members[m].exception_state == 2) return members[m].nonthrowing;
    if (members[m].exception_state == 1) throw std::logic_error("cyclic destructor exception specification");
    members[m].exception_state = 1;
    bool no_throw = true;
    for (unsigned j = 0; j < members[m].destruction_count; ++j)
        no_throw &= function_nonthrowing(destruction_actions[members[m].destruction_begin+j].destructor);
    members[m].exception_state = 2; members[m].nonthrowing = no_throw;
    return no_throw;
}
} }
