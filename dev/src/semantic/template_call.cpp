#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::template_facts(EntityId e)
{
    TemplateFunction t; t.environment = entities[e].owner; t.offset = template_parameters.size();
    for (std::uint32_t d = scopes[t.environment].first_decl; d; d = declarations[d].next) {
        EntityId p = declarations[d].entity;
        if (entities[p].template_parameter) { template_parameters.push_back(p); ++t.count; }
    }
    entities[e].template_info = templates.size(); templates.push_back(t);
}
std::uint32_t Analyzer::intern_arguments(const std::vector<TypeId>& args)
{
    std::uint64_t hash = 1469598103934665603ULL;
    for (TypeId t : args) hash = (hash ^ t) * 1099511628211ULL;
    if (argument_slots.empty() || argument_packs.size() * 2 >= argument_slots.size()) {
        argument_slots.assign(argument_slots.empty() ? 32 : argument_slots.size() * 2, 0);
        for (std::uint32_t i = 1; i < argument_packs.size(); ++i) {
            std::size_t p = argument_packs[i].hash & (argument_slots.size() - 1);
            while (argument_slots[p]) p = (p + 1) & (argument_slots.size() - 1);
            argument_slots[p] = i;
        }
    }
    std::size_t p = hash & (argument_slots.size() - 1);
    while (argument_slots[p]) {
        TypeArguments a = argument_packs[argument_slots[p]];
        bool equal = a.hash == hash && a.count == args.size();
        for (std::size_t i = 0; equal && i < args.size(); ++i) equal = argument_types[a.offset + i] == args[i];
        if (equal) return argument_slots[p];
        p = (p + 1) & (argument_slots.size() - 1);
    }
    TypeArguments a; a.offset = argument_types.size(); a.count = args.size(); a.hash = hash;
    argument_types.insert(argument_types.end(), args.begin(), args.end());
    argument_slots[p] = argument_packs.size(); argument_packs.push_back(a);
    return argument_slots[p];
}
TypeId Analyzer::substitute_type(TypeId pattern, const Index& bindings, Index& cache)
{
    if (cache.get(pattern)) return cache.get(pattern);
    Type p = types[pattern];
    TypeId result = pattern;
    if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        result = bindings.get(p.entity);
        if (!result) return 0;
        result = types.qualify(result, p.cv);
    } else if (p.kind == TypeKind::Function) {
        TypeId returned = substitute_type(p.child, bindings, cache);
        if (!returned || types[returned].kind == TypeKind::Array || types[returned].kind == TypeKind::Function) return 0;
        std::vector<TypeId> params;
        for (unsigned i = 0; i < p.count; ++i) {
            TypeId t = substitute_type(types.parameters[p.offset + i], bindings, cache);
            if (!t || fundamental(t, FT_VOID)) return 0;
            params.push_back(t);
        }
        result = types.signature(types.function(returned, params, p.variadic, p.cv));
    } else if (p.child) {
        TypeId child = substitute_type(p.child, bindings, cache);
        if (!child) return 0;
        bool reference = types[child].kind == TypeKind::LRef || types[child].kind == TypeKind::RRef;
        if (reference && (p.kind == TypeKind::Pointer || p.kind == TypeKind::Array || p.kind == TypeKind::MemberPointer)) return 0;
        if ((p.kind == TypeKind::LRef || p.kind == TypeKind::RRef) && fundamental(child, FT_VOID)) return 0;
        if (p.kind == TypeKind::Array && (fundamental(child, FT_VOID) || types[child].kind == TypeKind::Function)) return 0;
        result = p.kind == TypeKind::MemberPointer ? types.member_pointer(p.entity, child) : types.compound(p.kind, child, p.bound);
        result = types.qualify(result, p.cv);
    }
    cache.put(pattern, result); return result;
}
EntityId Analyzer::specialize(EntityId pattern, const std::vector<TypeId>& args)
{
    TemplateFunction t = templates[entities[pattern].template_info];
    if (args.size() != t.count) return 0;
    std::uint32_t pack = intern_arguments(args);
    std::uint32_t previous = specialization_index.get(key(pattern, pack));
    if (previous) return specializations[previous].entity;
    Specialization spec; spec.pattern = pattern; spec.arguments = pack; spec.declaration = FactState::Active;
    std::uint32_t index = specializations.size(); specializations.push_back(spec);
    specialization_index.put(key(pattern, pack), index);
    Index bindings, cache;
    for (unsigned i = 0; i < t.count; ++i) bindings.put(template_parameters[t.offset + i], args[i]);
    TypeId type = substitute_type(entities[pattern].type, bindings, cache);
    if (!type) { specializations[index].declaration = FactState::Failure; return 0; }
    EntityId e = make_entity(EntityKind::Function, scopes[t.environment].parent, entities[pattern].name, entities[pattern].source);
    entities[e].type = type; entities[e].specialization = index;
    if (scopes[entities[e].owner].kind == ScopeKind::Class) member_facts(e);
    specializations[index].entity = e; specializations[index].declaration = FactState::Success;
    return e;
}
bool Analyzer::deduce_type(TypeId pattern, TypeId actual, Index& bindings)
{
    Type p = types[pattern], a = types[actual];
    if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        TypeId old = bindings.get(p.entity);
        TypeId value = types.unqualified(actual);
        if (old && old != value) return false;
        bindings.put(p.entity, value); return true;
    }
    if (p.kind != a.kind || (p.cv & ~a.cv)) return false;
    if (p.kind == TypeKind::Function) {
        if (p.count != a.count || p.variadic != a.variadic) return false;
        for (unsigned i = 0; i < p.count; ++i)
            if (!deduce_type(types.parameters[p.offset + i], types.parameters[a.offset + i], bindings)) return false;
    }
    if (p.child) return deduce_type(p.child, a.child, bindings);
    return types.unqualified(pattern) == types.unqualified(actual);
}
EntityId Analyzer::deduce_function(EntityId pattern, const std::vector<NodeId>& args)
{
    Type f = types[entities[pattern].type];
    if (args.size() < f.count || (!f.variadic && args.size() != f.count)) return 0;
    Index bindings;
    for (unsigned i = 0; i < f.count; ++i) {
        TypeId p = types.parameters[f.offset + i], a = expressions[args[i]].type;
        if (types[p].kind == TypeKind::LRef || types[p].kind == TypeKind::RRef) p = types[p].child;
        else a = decay(a);
        if (!deduce_type(p, a, bindings)) return 0;
    }
    TemplateFunction t = templates[entities[pattern].template_info];
    std::vector<TypeId> arguments;
    for (unsigned i = 0; i < t.count; ++i) {
        TypeId a = bindings.get(template_parameters[t.offset + i]);
        if (!a) return 0;
        arguments.push_back(a);
    }
    return specialize(pattern, arguments);
}
EntityId Analyzer::explicit_template(NodeId name, EntityId binding, ScopeId s)
{
    NodeId list = child(ast[name].last, Kind::TemplateArguments);
    if (!list) return binding;
    std::vector<TypeId> args;
    for (NodeId a = ast[list].first; a; a = ast[a].next) {
        if (ast[a].kind != Kind::TypeId) throw std::runtime_error("type template argument required");
        args.push_back(type_id(a, s));
    }
    EntityId result = 0;
    for (EntityId e : candidates(binding)) {
        if (!entities[e].template_info) continue;
        EntityId instance = specialize(e, args);
        if (instance) result = merge_lookup(result, instance);
    }
    if (!result) throw std::runtime_error("invalid explicit function template arguments");
    return result;
}
void Analyzer::demand_specialization(EntityId e)
{
    std::uint32_t i = entities[e].specialization;
    if (!i || specializations[i].emission_demanded) return;
    specializations[i].emission_demanded = true;
    specialization_demand.push_back(e);
}
} }
