#include "semantic/analyzer.h"
#include <stdexcept>
#include <ostream>

namespace cppgm { namespace semantic {
Conversion Analyzer::object_conversion(EntityId e, TypeId object, ValueCategory category, ScopeId naming)
{
    Type f = types[entities[e].type];
    TypeId wanted = types[types.parameters[types[call_type(e)].offset]].child;
    // A using-declaration changes the class of the implicit object parameter
    // for ranking. The actual declaration/base adjustment stays unchanged.
    EntityId cls = naming && scopes[naming].kind == ScopeKind::Class ? scopes[naming].entity : types[object].entity;
    while (cls && entities[cls].scope != entities[e].owner) {
        ScopeId scope = entities[cls].scope;
        if (using_access.get(key(scope, e))) { wanted = types.qualify(entities[cls].type, f.cv); break; }
        auto edge = access_base(cls);
        cls = edge ? bases[edge].base : 0;
    }
    // [over.match.funcs]/4: conversion functions rank as members of the
    // implied object's class. The selected call records its base adjustment.
    if (members[entities[e].member_info].conversion_target)
        wanted = types.qualify(types.unqualified(object),f.cv);
    Conversion c;
    c.target = types.compound(f.ref == RefQualifier::Rvalue ? TypeKind::RRef : TypeKind::LRef, wanted);
    c.reference = true;
    bool rvalue = category != ValueCategory::Lvalue;
    if (f.ref == RefQualifier::Rvalue && !rvalue) return c;
    if (f.ref == RefQualifier::Lvalue && rvalue && types[wanted].cv != 1) return c;
    if (!destructor_member(e) && (types[object].cv & ~types[wanted].cv)) return c;
    bool derived = types[object].kind == TypeKind::Named && types[wanted].kind == TypeKind::Named &&
        types[object].entity != types[wanted].entity && class_derives(types[object].entity,types[wanted].entity);
    if (types.unqualified(object) != types.unqualified(wanted) && !derived) return c;
    // Candidate viability needs only declared base edges, including retained
    // template patterns. It must not demand object layout or member definitions.
    if (derived) {
        auto named = naming && scopes[naming].kind == ScopeKind::Class ? scopes[naming].entity : types[object].entity;
        auto first = base_path(object,named), tail = base_path(entities[named].type,types[wanted].entity);
        if ((first && (!base_adjustments[first].edge || base_adjustments[first].ambiguous)) ||
            (tail && (!base_adjustments[tail].edge || base_adjustments[tail].ambiguous))) return c;
    }
    c.rank = derived ? 2 : 0; c.derived = derived;
    c.qualification = types[wanted].cv & ~types[object].cv;
    c.preference = rvalue && f.ref == RefQualifier::Lvalue;
    return c;
}
unsigned Analyzer::base_path(TypeId from, EntityId to)
{
    while (auto enclosing = injected_class_owners.get(to)) to = enclosing;
    EntityId e = types[from].entity;
    if (e == to) return 0;
    // Base edges are fixed before member checking; concrete specializations
    // have distinct entity identities from retained patterns. Cache graph facts
    // separately from layout, and share tails across derived uses.
    auto identity = key(e,to);
    if (auto known = base_adjustment_index.get(identity)) { ++base_adjustment_hits; return known; }
    BaseAdjustment path; bool found = false;
    for (auto b = access_base(e); b; b = bases[b].next) {
        ++base_adjustment_work;
        auto edge = bases[b];
        auto tail = base_path(entities[edge.base].type,to);
        if (tail && !base_adjustments[tail].edge) continue;
        if (found) { path.ambiguous = true; break; }
        found = true; path.edge = b; path.next = tail;
        path.ambiguous = base_adjustments[path.next].ambiguous;
    }
    // A nonzero record without an edge is a cached miss. Memoizing these
    // avoids repeated subtree searches in branching inheritance graphs.
    auto id = base_adjustments.size(); base_adjustments.push_back(path);
    base_adjustment_index.put(identity,id); return id;
}
std::uint64_t Analyzer::layout_base_path(unsigned id)
{
    if (!id || base_adjustments[id].laid_out) return base_adjustments[id].total;
    auto tail = layout_base_path(base_adjustments[id].next);
    auto& path = base_adjustments[id];
    path.offset = bases[path.edge].offset; path.total = path.offset + tail;
    path.laid_out = true; return path.total;
}
unsigned Analyzer::base_steps(TypeId from, EntityId to)
{
    auto path = base_path(from,to);
    if (path && !base_adjustments[path].edge) throw std::logic_error("missing selected base path");
    if (base_adjustments[path].ambiguous) throw std::runtime_error("ambiguous base subobject");
    if (path && !base_adjustments[path].laid_out) { size(from); layout_base_path(path); }
    return path;
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
void Analyzer::demand_member(EntityId e, MemberDemandReason reason)
{
    record_default_dependency(DefaultDependencyKind::Member,e);
    if (unevaluated_depth) return;
    if (definitions && entities[e].specialization && !entities[e].template_info) demand_specialization(e);
    entities[e].emission |= Entity::Used;
    std::uint32_t m = entities[e].member_info;
    if (m) { members[m].referenced = true; members[m].demand_reasons |= static_cast<unsigned char>(reason); }
    if (m && (members[m].constructor || members[m].destructor))
        demand_vtable(scopes[entities[e].owner].entity, members[m].constructor ? VtableReason::Constructor : VtableReason::Destructor);
    require_member_body(e);
}
void Analyzer::require_member_definition(EntityId e)
{
    auto m = entities[e].member_info;
    // A processed external declaration had no local body prerequisite. The
    // newly published definition is a distinct key and wakes just this member.
    if (members[m].demand == DemandState::Complete &&
        !(members[m].demand_reasons & static_cast<unsigned char>(MemberDemandReason::LocalDefinition)))
        members[m].demand = DemandState::Dormant;
    demand_member(e, MemberDemandReason::LocalDefinition);
}
void Analyzer::require_member_body(EntityId e)
{
    if (unevaluated_depth) return;
    if (entities[e].body_state == FactState::Failure || entities[e].lifetime_state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::FunctionDefinition,e,entities[e].definition);
    demand_friend_body(e);
    if (definitions) instantiate_member_definition(e);
    auto m = entities[e].member_info;
    if (m && members[m].demand == DemandState::Failed)
        throw FailedSemanticFact(SemanticFact::MemberBody,e,members[m].source);
    bool retained_owner = definitions && definition_owner(scopes[entities[e].owner].entity).specialization;
    if (!m || members[m].demand != DemandState::Dormant || (!members[m].body && !members[m].synthetic && !members[m].destructor && !retained_owner)) return;
    members[m].demand = DemandState::Queued;
    demand_queue.push_back(e);
}
void Analyzer::default_initialize(EntityId object, NodeId declarator)
{
    TypeId t = entities[object].type;
    while (types[t].kind == TypeKind::Array) t = types[t].child;
    // The source recipe belongs to this declarator. Entity::source names the
    // whole declaration and can contain several independently initialized objects.
    auto occurrence = ast.nodes.occurrences[declarator];
    auto source = occurrence.context ? template_declaration_sources.get(occurrence.source) : 0;
    auto ctor = template_default_constructors.get(source);
    if (ctor && types.unqualified(t) == entities[scopes[entities[ctor].owner].entity].type) {
        ++default_initialization_uses;
    } else ctor = check_default_initialization(t,entities[object].owner);
    if (!ctor) return;
    prepare_default_call(ctor);
    members[entities[ctor].member_info].source_demand = true;
    members[entities[ctor].member_info].complete_entry = true;
    if (scopes[entities[object].owner].kind == ScopeKind::Namespace) {
        for (ScopeId s = entities[object].owner; s && s != global; s = scopes[s].parent)
            if (scopes[s].kind == ScopeKind::Namespace && !scopes[s].name)
                members[entities[ctor].member_info].retained_root = true;
    }
    object_actions.put(object, actions.size());
    actions.push_back({object, ctor, types.compound(TypeKind::Pointer, t)});
    demand_member(ctor);
}
bool Analyzer::derived_from(TypeId from, TypeId to)
{
    if (types[from].kind != TypeKind::Named || types[to].kind != TypeKind::Named) return false;
    EntityId source = types[from].entity, target = types[to].entity;
    if (source == target) return false;
    if (definitions) complete_class(source);
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
{ return entities[e].kind == EntityKind::Variable && !entities[e].is_static && scopes[entities[e].owner].kind == ScopeKind::Class &&
    (entities[e].name || !field_fact(e).bit_field); }
EntityId Analyzer::injected_storage(EntityId e) const
{
    if (!nonstatic_field(e)) return 0;
    auto cls = scopes[entities[e].owner].entity;
    return class_facts[entities[cls].class_info].storage;
}
bool Analyzer::empty_value(TypeId t)
{
    if (types[t].kind != TypeKind::Named || !entities[types[t].entity].class_info) return false;
    auto c = entities[types[t].entity].class_info;
    if (!entities[types[t].entity].complete || !class_facts[c].aggregate) return false;
    size(t);
    return class_facts[c].empty;
}
void Analyzer::record_object(Expression& owner, NodeId node, TypeId type, unsigned adjustment)
{
    ObjectUse use; use.node = node; use.type = type; use.adjustment = adjustment;
    owner.object_use = object_uses.size(); object_uses.push_back(use);
}
} }
