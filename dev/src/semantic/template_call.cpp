#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::template_facts(EntityId e, ScopeId environment)
{
    TemplateFunction t; t.environment = environment ? environment : entities[e].owner; t.offset = template_parameters.size();
    for (std::uint32_t d = scopes[t.environment].first_decl; d; d = declarations[d].next) {
        EntityId p = declarations[d].entity;
        if (entities[p].template_parameter) { parameter_ordinals.put(p,t.count+1); template_parameters.push_back(p); ++t.count; }
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
bool Analyzer::dependent_type(TypeId id)
{
    if (type_dependence.size() <= id) type_dependence.resize(id + 1);
    if (type_dependence[id]) return type_dependence[id] == 2;
    ++dependence_work;
    Type t = types[id];
    bool dependent = t.kind == TypeKind::DependentName || t.kind == TypeKind::Decltype || t.kind == TypeKind::DependentArray ||
        (t.kind == TypeKind::Named && (entities[t.entity].template_parameter || entities[t.entity].template_pattern));
    if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto pack = specialization_arguments(t.entity);
        for (unsigned j = 0; j < pack.count; ++j) dependent |= dependent_argument(argument_types[pack.offset+j]);
    }
    if (t.child) dependent |= dependent_type(t.child);
    if (t.kind == TypeKind::Function) {
        for (unsigned i = 0; i < t.count; ++i) dependent |= dependent_type(types.parameters[t.offset + i]);
    }
    type_dependence[id] = dependent ? 2 : 1;
    return dependent;
}
TypeId Analyzer::substitute_type(TypeId pattern, const Index& bindings, Index& cache, std::uint32_t owner)
{
    if (!dependent_type(pattern)) return pattern;
    auto cache_key = key(owner,pattern);
    if (owner) {
        if (auto known = specialization_type_cache.get(cache_key)) { ++substitution_hits; return known; }
    } else if (auto known = cache.get(pattern)) return known;
    ++substitution_work;
    Type p = types[pattern];
    TypeId result = pattern;
    if (p.kind == TypeKind::DependentArray) {
        auto child = substitute_type(p.child,bindings,cache,owner);
        auto query = substitute_query(p.bound,bindings,cache,owner);
        if (!child || !query) return 0;
        if (query_fact(query).dependent) result = types.compound(TypeKind::DependentArray,child,query);
        else {
            auto value = constants[query_value(query)];
            if (!value.valid || !integral(value.type) || scoped_enum(value.type) || !value.bits ||
                (!is_unsigned(value.type) && static_cast<std::int64_t>(value.bits) < 0))
                throw std::runtime_error("substituted array bound must be a positive integral constant");
            result = types.compound(TypeKind::Array,child,value.bits);
        }
        result = types.qualify(result,p.cv);
    } else if (p.kind == TypeKind::Decltype) {
        auto query = substitute_query(p.entity,bindings,cache,owner);
        if (!query) return 0;
        result = types.qualify(query_decltype(query,p.bound),p.cv);
    } else if (p.kind == TypeKind::DependentName) {
        auto qualifier = substitute_type(p.child,bindings,cache,owner);
        if (!qualifier) return 0;
        std::vector<TypeId> args;
        for (unsigned j = 0; j < p.count; ++j) {
            auto arg = substitute_argument(types.parameters[p.offset+j],bindings,cache,owner);
            if (!arg) return 0;
            args.push_back(arg);
        }
        result = types.qualify(qualified_type(qualifier,p.entity,args,p.bound),p.cv);
    } else if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        result = owner ? substitution_argument(owner,p.entity) : bindings.get(p.entity);
        if (!result) return 0;
        result = types.qualify(result, p.cv);
    } else if (p.kind == TypeKind::Named && entities[p.entity].template_pattern) {
        // Local declaration identity is an input to substitution, independently
        // of whether the declaration's members use template parameters.
        if (!owner) return pattern;
        auto entity = substitution_binding(owner,p.entity);
        result = types.qualify(entities[entity].type,p.cv);
    } else if (p.kind == TypeKind::Named && entities[p.entity].specialization) {
        auto spec = specializations[entities[p.entity].specialization];
        auto pack = argument_packs[spec.arguments];
        std::vector<TypeId> args;
        for (unsigned j = 0; j < pack.count; ++j) {
            auto value = substitute_argument(argument_types[pack.offset+j],bindings,cache,owner);
            if (!value) return 0;
            args.push_back(value);
        }
        auto e = specialize_class(spec.pattern,args);
        result = types.qualify(entities[e].type,p.cv);
    } else if (p.kind == TypeKind::Function) {
        TypeId returned = substitute_type(p.child, bindings, cache,owner);
        if (!returned || types[returned].kind == TypeKind::Array || types[returned].kind == TypeKind::Function) return 0;
        std::vector<TypeId> params;
        for (unsigned i = 0; i < p.count; ++i) {
            TypeId t = substitute_type(types.parameters[p.offset + i], bindings, cache,owner);
            if (!t || fundamental(t, FT_VOID)) return 0;
            params.push_back(t);
        }
        result = types.signature(types.function(returned, params, p.variadic, p.cv, p.ref));
    } else if (p.child) {
        TypeId child = substitute_type(p.child, bindings, cache,owner);
        if (!child) return 0;
        bool reference = types[child].kind == TypeKind::LRef || types[child].kind == TypeKind::RRef;
        if (reference && (p.kind == TypeKind::Pointer || p.kind == TypeKind::Array || p.kind == TypeKind::MemberPointer)) return 0;
        if ((p.kind == TypeKind::LRef || p.kind == TypeKind::RRef) && fundamental(child, FT_VOID)) return 0;
        if (p.kind == TypeKind::Array && (fundamental(child, FT_VOID) || types[child].kind == TypeKind::Function)) return 0;
        result = p.kind == TypeKind::MemberPointer ? types.member_pointer(p.entity, child) : types.compound(p.kind, child, p.bound);
        result = types.qualify(result, p.cv);
    }
    if (owner) { specialization_type_cache.put(cache_key,result); ++substitution_records; }
    else cache.put(pattern, result);
    return result;
}
EntityId Analyzer::specialize(EntityId pattern, const std::vector<TypeId>& input)
{
    TemplateFunction t = templates[entities[pattern].template_info];
    auto args = input;
    if (args.size() > t.count) return 0;
    if (definitions && !template_defaults(pattern,args,true)) return 0;
    bool partial = args.size() < t.count;
    if (partial && !definitions) return 0;
    std::uint32_t pack = intern_arguments(args);
    std::uint32_t previous = specialization_index.get(key(pattern, pack));
    if (previous) return specializations[previous].entity;
    Specialization spec; spec.pattern = pattern; spec.arguments = pack; spec.declaration = FactState::Active;
    std::uint32_t index = specializations.size(); specializations.push_back(spec);
    specialization_index.put(key(pattern, pack), index);
    try {
    Index bindings, cache;
    auto frame = dependent_type(entities[pattern].type) ? substitution_frame(index,t.offset,t.count) : 0;
    TypeId type = substitute_type(entities[pattern].type, bindings, cache,frame);
    if (!type) { specializations[index].declaration = FactState::Failure; return 0; }
    EntityId e = make_entity(EntityKind::Function, entities[pattern].owner == t.environment ? scopes[t.environment].parent : entities[pattern].owner, entities[pattern].name, entities[pattern].source);
    entities[e].type = type; entities[e].specialization = index;
    entities[e].defaults = entities[pattern].defaults;
    if (partial) {
        t.primary = pattern; t.explicit_arguments = pack;
        entities[e].template_info = templates.size(); templates.push_back(t);
    }
    if (scopes[entities[e].owner].kind == ScopeKind::Class) member_facts(e);
    specializations[index].entity = e; specializations[index].declaration = FactState::Success;
    return e;
    } catch (...) {
        specializations[index].declaration = FactState::Failure; throw;
    }
}
bool Analyzer::deduce_type(TypeId pattern, TypeId actual, Index& bindings)
{
    Type p = types[pattern], a = types[actual];
    if (p.kind == TypeKind::DependentArray)
        return (a.kind == TypeKind::Array || a.kind == TypeKind::DependentArray) && deduce_type(p.child,a.child,bindings);
    if (p.kind == TypeKind::DependentName || p.kind == TypeKind::Decltype) return true; // non-deduced context
    if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        TypeId old = bindings.get(p.entity);
        TypeId value = types.qualify(types.unqualified(actual), a.cv & ~p.cv);
        if (old && old != value) return false;
        bindings.put(p.entity, value); return true;
    }
    if (p.kind != a.kind) return false;
    if (p.kind == TypeKind::Named && entities[p.entity].specialization && entities[a.entity].class_info &&
        (!entities[a.entity].specialization || specialization_pattern(p.entity) != specialization_pattern(a.entity))) {
        // [temp.deduct.call] permits a matching base specialization when the
        // parameter is a simple-template-id (also behind a pointer). Only
        // explicit base edges participate; non-type arguments stay non-deduced.
        complete_class(a.entity);
        Index selected; unsigned matches = 0;
        for (auto b = class_facts[entities[a.entity].class_info].first_base; b; b = bases[b].next) {
            Index trial = bindings;
            if (!deduce_type(pattern,types.qualify(entities[bases[b].base].type,a.cv),trial)) continue;
            if (++matches > 1) return false;
            selected = std::move(trial);
        }
        if (matches) bindings = std::move(selected);
        return matches != 0;
    }
    if (p.kind == TypeKind::Named && entities[p.entity].specialization && entities[a.entity].specialization) {
        auto ps = specializations[entities[p.entity].specialization], as = specializations[entities[a.entity].specialization];
        if (ps.pattern != as.pattern) return false;
        auto x = argument_packs[ps.arguments], y = argument_packs[as.arguments];
        if (x.count != y.count) return false;
        for (unsigned j = 0; j < x.count; ++j)
            if (value_argument(argument_types[x.offset+j]) || value_argument(argument_types[y.offset+j])) {
                if (argument_types[x.offset+j] != argument_types[y.offset+j] && !dependent_argument(argument_types[x.offset+j])) return false;
            } else if (!deduce_type(argument_types[x.offset+j],argument_types[y.offset+j],bindings)) return false;
        return true;
    }
    if (p.kind == TypeKind::Function) {
        if (p.count != a.count || p.variadic != a.variadic) return false;
        for (unsigned i = 0; i < p.count; ++i)
            if (!deduce_type(types.parameters[p.offset + i], types.parameters[a.offset + i], bindings)) return false;
    }
    if (p.child) return deduce_type(p.child, a.child, bindings);
    return types.unqualified(pattern) == types.unqualified(actual);
}
template<class Arguments>
EntityId Analyzer::deduce_function_values(EntityId pattern, const Arguments& args)
{
    Type f = types[entities[pattern].type];
    if ((!f.variadic && args.size() > f.count) ||
        (args.size() < f.count && (!entities[pattern].defaults || !default_arguments[entities[pattern].defaults+args.size()]))) return 0;
    TemplateFunction t = templates[entities[pattern].template_info];
    TypeArguments explicit_args = argument_packs[t.explicit_arguments];
    Index bindings;
    for (unsigned i = 0; i < explicit_args.count; ++i)
        bindings.put(template_parameters[t.offset+i],argument_types[explicit_args.offset+i]);
    for (unsigned i = 0; i < std::min<std::size_t>(f.count,args.size()); ++i) {
        TypeId p = types.parameters[f.offset+i], a = args[i].type;
        if (!dependent_type(p)) continue;
        if (args[i].form == ExpressionForm::Overload) {
            auto kind = types[p].kind;
            auto adjusted = kind == TypeKind::LRef || kind == TypeKind::RRef ? types[p].child : p;
            Index selected;
            unsigned matches = 0;
            for (auto candidate : candidates(args[i].entity)) {
                // [temp.deduct.call]: an overload set containing a template or
                // multiple matching functions is a non-deduced context. Other
                // arguments can establish the eventual conversion target.
                if (entities[candidate].template_info) { matches = 2; break; }
                ++candidate_work;
                TypeId actual = entities[candidate].type;
                if (kind != TypeKind::LRef && kind != TypeKind::RRef) actual = decay(actual);
                Index trial;
                if (!deduce_type(adjusted,actual,trial)) continue;
                if (++matches > 1) break;
                selected = std::move(trial);
            }
            if (matches == 1) for (unsigned j = 0; j < t.count; ++j) {
                auto parameter = template_parameters[t.offset+j];
                auto value = selected.get(parameter), previous = bindings.get(parameter);
                if (value && previous && value != previous) return 0;
                if (value) bindings.put(parameter,value);
            }
            continue;
        }
        if (!a) return 0;
        auto param = types[p];
        if (param.kind == TypeKind::RRef && types[param.child].kind == TypeKind::Named &&
            entities[types[param.child].entity].template_parameter && !types[param.child].cv && args[i].category == ValueCategory::Lvalue)
            a = types.compound(TypeKind::LRef,a);
        if (param.kind == TypeKind::LRef || param.kind == TypeKind::RRef) p = param.child;
        else a = decay(a);
        if (!deduce_type(p,a,bindings)) return 0;
    }
    std::vector<TypeId> arguments;
    for (unsigned i = 0; i < t.count; ++i) {
        TypeId a = bindings.get(template_parameters[t.offset+i]);
        if (!a) break;
        arguments.push_back(a);
    }
    auto primary = t.primary ? t.primary : pattern;
    if (arguments.size() != t.count && (!definitions || !template_defaults(primary,arguments))) return 0;
    return specialize(primary,arguments);
}
EntityId Analyzer::deduce_function(EntityId pattern, const std::vector<NodeId>& args, unsigned begin)
{
    struct Values {
        const ExpressionStore& facts; const std::vector<NodeId>& nodes; unsigned begin;
        std::size_t size() const { return nodes.size()-begin; }
        Expression operator[](std::size_t i) const { return facts[nodes[begin+i]]; }
    } values{expressions,args,begin};
    return deduce_function_values(pattern,values);
}
EntityId Analyzer::deduce_function(EntityId pattern, const std::vector<Expression>& args, unsigned begin)
{
    struct Values {
        const std::vector<Expression>& facts; unsigned begin;
        std::size_t size() const { return facts.size()-begin; }
        const Expression& operator[](std::size_t i) const { return facts[begin+i]; }
    } values{args,begin};
    return deduce_function_values(pattern,values);
}
EntityId Analyzer::deduce_target(EntityId pattern, TypeId target)
{
    auto t = templates[entities[pattern].template_info];
    auto explicit_args = argument_packs[t.explicit_arguments];
    Index bindings;
    for (unsigned j = 0; j < explicit_args.count; ++j)
        bindings.put(template_parameters[t.offset+j],argument_types[explicit_args.offset+j]);
    if (!deduce_type(entities[pattern].type,target,bindings)) return 0;
    std::vector<TypeId> args;
    for (unsigned j = 0; j < t.count; ++j) {
        auto a = bindings.get(template_parameters[t.offset+j]);
        if (!a) break;
        args.push_back(a);
    }
    auto primary = t.primary ? t.primary : pattern;
    if (args.size() != t.count && !template_defaults(primary,args)) return 0;
    return specialize(primary,args);
}
EntityId Analyzer::explicit_template(NodeId name, EntityId binding, ScopeId s)
{
    NodeId list = child(ast[name].last, Kind::TemplateArguments);
    if (!list) return binding;
    std::vector<TypeId> args;
    for (NodeId a = ast[list].first; a; a = ast[a].next) {
        args.push_back(template_argument_node(a,s));
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
    if (entities[e].specialization) record_default_dependency(DefaultDependencyKind::Specialization,e);
    if (unevaluated_depth) return;
    std::uint32_t i = entities[e].specialization;
    if (!i || specializations[i].emission_demanded) return;
    specializations[i].emission_demanded = true;
    specialization_demand.push_back(e);
}
} }

namespace cppgm { namespace semantic {
bool Analyzer::template_more_specialized(EntityId a, EntityId b)
{
    if (!a || !b || !entities[a].specialization || !entities[b].specialization) return false;
    auto shape = [&](EntityId e) {
        auto t = types[entities[specialization_pattern(e)].type];
        std::vector<TypeId> parameters(types.parameters.begin()+t.offset,types.parameters.begin()+t.offset+t.count);
        return types.function(types.fundamental(FT_VOID),parameters,t.variadic,t.cv,t.ref);
    };
    auto x = shape(a), y = shape(b);
    Index xy, yx;
    return deduce_type(y,x,yx) && !deduce_type(x,y,xy);
}
} }
