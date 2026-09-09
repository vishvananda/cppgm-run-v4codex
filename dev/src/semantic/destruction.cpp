#include "semantic/analyzer.h"
#include <stdexcept>
#include <algorithm>
namespace cppgm { namespace semantic {
using syntax::Kind;
EntityId Analyzer::type_destructor(TypeId t) const
{
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    return types[t].kind == TypeKind::Named && entities[types[t].entity].class_info ? class_facts[entities[types[t].entity].class_info].destructor : 0;
}
bool Analyzer::destructor_member(EntityId e) const
{
    return e && entities[e].member_info && members[entities[e].member_info].destructor;
}
EntityId Analyzer::default_destructor(TypeId t)
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
    demand_member(dtor);
    return dtor;
}
void Analyzer::register_destruction(EntityId e)
{
    EntityId dtor = default_destructor(entities[e].type);
    if (dtor) object_destructors.put(e, dtor);
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
    if (entities[cls].key != KW_UNION) {
        for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
            EntityId field = declarations[d].entity;
            if (!nonstatic_field(field) || entities[field].owner != entities[cls].scope) continue;
            EntityId dtor = default_destructor(entities[field].type);
            if (dtor) work.push_back({field, entities[field].type, dtor});
        }
        std::reverse(work.begin(), work.end());
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
            TypeId base = entities[bases[b].base].type;
            EntityId dtor = default_destructor(base);
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
    if (!members[m].synthetic) return true;
    if (members[m].destruction_state == 2) return members[m].destruction_needed;
    if (members[m].destruction_state == 1) throw std::logic_error("cyclic destruction actions");
    members[m].destruction_state = 1;
    bool needed = false;
    for (unsigned j = 0; j < members[m].destruction_count; ++j)
        needed |= destructor_needed(destruction_actions[members[m].destruction_begin+j].destructor);
    members[m].destruction_state = 2; members[m].destruction_needed = needed;
    return needed;
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
    if (spec) return spec == 1;
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
