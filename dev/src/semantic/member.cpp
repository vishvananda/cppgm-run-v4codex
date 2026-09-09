#include "semantic/analyzer.h"
#include <stdexcept>
#include <ostream>

namespace cppgm { namespace semantic {
unsigned Analyzer::base_steps(TypeId from, EntityId to) const
{
    EntityId e = types[from].entity;
    unsigned count = 0;
    while (e && e != to) {
        auto b = class_facts[entities[e].class_info].first_base;
        if (!b) return 0;
        e = bases[b].base; ++count;
    }
    return count;
}
TypeId Analyzer::implicit_object_type(ScopeId s)
{
    while (s && scopes[s].kind != ScopeKind::Function) s = scopes[s].parent;
    EntityId e = scopes[s].entity;
    if (!e || !entities[e].member_info || entities[e].is_static) return 0;
    Type f = types[members[entities[e].member_info].call_type];
    return types.parameters[f.offset];
}
void Analyzer::member_facts(EntityId e)
{
    if (!entities[e].member_info) {
        entities[e].member_info = members.size(); members.push_back(MemberFacts());
    }
    Type f = types[entities[e].type];
    std::vector<TypeId> params;
    if (!entities[e].is_static) {
        TypeId object = types.qualify(entities[scopes[entities[e].owner].entity].type, f.cv);
        params.push_back(types.compound(TypeKind::Pointer, object));
    }
    for (unsigned i = 0; i < f.count; ++i) params.push_back(types.parameters[f.offset + i]);
    members[entities[e].member_info].call_type = types.function(f.child, params, f.variadic);
}
void Analyzer::demand_member(EntityId e)
{
    if (unevaluated_depth) return;
    std::uint32_t m = entities[e].member_info;
    if (m) members[m].referenced = true;
    if (!m || members[m].demand != DemandState::Dormant || (!members[m].body && !members[m].synthetic && !members[m].destructor)) return;
    members[m].demand = DemandState::Queued;
    demand_queue.push_back(e);
}
void Analyzer::default_initialize(EntityId object)
{
    TypeId t = entities[object].type;
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return;
    EntityId ctor = default_constructor(t);
    members[entities[ctor].member_info].source_demand = true;
    object_actions.put(object, actions.size());
    actions.push_back({object, ctor, types.compound(TypeKind::Pointer, t)});
    demand_member(ctor);
}
bool Analyzer::derived_from(TypeId from, TypeId to)
{
    if (types[from].kind != TypeKind::Named || types[to].kind != TypeKind::Named) return false;
    EntityId source = types[from].entity, target = types[to].entity;
    if (source == target) return false;
    std::vector<EntityId> work(1, source);
    Index seen;
    while (!work.empty()) {
        EntityId e = work.back(); work.pop_back();
        if (e == target) return true;
        if (seen.get(e)) continue;
        seen.put(e, 1);
        for (std::uint32_t b = class_facts[entities[e].class_info].first_base; b; b = bases[b].next) work.push_back(bases[b].base);
    }
    return false;
}
} }

namespace cppgm { namespace semantic {
namespace {
void indent(std::ostream& out, unsigned depth) { for (unsigned i = 0; i < depth; ++i) out << "  "; }
}
void Analyzer::write_object(std::ostream& out, EntityId e, NodeId init, unsigned depth) const
{
    indent(out, depth);
    out << (entities[e].kind == EntityKind::Alias ? "type-alias " : entities[e].kind == EntityKind::Function ? "function-declaration " : "variable ");
    if (entities[e].kind == EntityKind::Function) write_entity_name(out, e); else spelling(out, entities[e].name);
    out << ' '; write_type(out, entities[e].type); out << '\n';
    if (init) write_expression(out, init, depth + 1);
    std::uint32_t action = object_actions.get(e);
    if (action) write_action(out, actions[action], depth + 1);
}
void Analyzer::write_action(std::ostream& out, const ObjectAction& a, unsigned depth) const
{
    indent(out, depth); out << "constructor-action "; write_entity_name(out, a.constructor); out << '\n';
    indent(out, depth + 1); out << "call-expression prvalue void\n";
    indent(out, depth + 2); out << "callee "; write_entity_name(out, a.constructor); out << ' ';
    write_type(out, members[entities[a.constructor].member_info].call_type); out << '\n';
    indent(out, depth + 2); out << "unary-expression prvalue "; write_type(out, a.address_type); out << " OP_AMP:&\n";
    indent(out, depth + 3); out << "id-expression lvalue "; write_type(out, entities[a.object].type);
    out << ' '; spelling(out, entities[a.object].name); out << '\n';
}
void Analyzer::write_function(std::ostream& out, EntityId e, NodeId body, ScopeId scope, unsigned depth, bool definition) const
{
    TypeId signature = entities[e].member_info ? members[entities[e].member_info].call_type : entities[e].type;
    indent(out, depth); out << (definition ? "function-definition " : "function-declaration ");
    write_entity_name(out, e); out << ' '; write_type(out, signature); out << '\n';
    Type function = types[signature];
    unsigned parameter = 0;
    if (entities[e].member_info && !entities[e].is_static) {
        indent(out, depth + 1); out << "parameter this "; write_type(out, types.parameters[function.offset + parameter++]); out << '\n';
    }
    for (std::uint32_t d = scopes[scope].first_decl; d; d = declarations[d].next) {
        EntityId p = declarations[d].entity;
        if (entities[p].kind != EntityKind::Parameter) continue;
        indent(out, depth + 1); out << "parameter "; spelling(out, entities[p].name); out << ' ';
        write_type(out, types.parameters[function.offset + parameter++]); out << '\n';
    }
    if (!definition && entities[e].specialization) {
        for (; parameter < function.count; ++parameter) {
            indent(out, depth + 1); out << "parameter  "; write_type(out, types.parameters[function.offset + parameter]); out << '\n';
        }
    }
    if (body) write_resolved(out, body, depth + 1);
    else if (definition) { indent(out, depth + 1); out << "compound-statement\n"; }
}
} }

namespace cppgm { namespace semantic {
TypeId Analyzer::call_type(EntityId e) const
{ return entities[e].member_info ? members[entities[e].member_info].call_type : entities[e].type; }
bool Analyzer::member_demanded(EntityId e) const
{ return entities[e].member_info && members[entities[e].member_info].referenced; }
bool Analyzer::constructor_member(EntityId e) const
{ return entities[e].member_info && members[entities[e].member_info].constructor; }
EntityId Analyzer::value_constructor(TypeId t) const
{ return types[t].kind == TypeKind::Named && entities[types[t].entity].class_info ? class_facts[entities[types[t].entity].class_info].value_constructor : 0; }
EntityId Analyzer::object_constructor(EntityId e) const
{ auto a = object_actions.get(e); return a ? actions[a].constructor : 0; }
bool Analyzer::synthetic_member(EntityId e) const
{ return entities[e].member_info && members[entities[e].member_info].synthetic; }
bool Analyzer::nonstatic_field(EntityId e) const
{ return entities[e].kind == EntityKind::Variable && !entities[e].is_static && scopes[entities[e].owner].kind == ScopeKind::Class; }
void Analyzer::record_object(Expression& owner, NodeId node, TypeId type, unsigned adjustment)
{
    ObjectUse use; use.node = node; use.type = type; use.adjustment = adjustment;
    owner.object_use = object_uses.size(); object_uses.push_back(use);
}
} }
