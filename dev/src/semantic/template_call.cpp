#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::template_facts(EntityId e, ScopeId environment)
{
    entities[e].template_info = retain_template_head(environment ? environment : entities[e].owner);
}
std::uint32_t Analyzer::retain_template_head(ScopeId environment)
{
    TemplateFunction t; t.environment = environment; t.offset = template_parameters.size();
    for (std::uint32_t d = scopes[t.environment].first_decl; d; d = declarations[d].next) {
        EntityId p = declarations[d].entity;
        if (entities[p].template_parameter) { parameter_ordinals.put(p,t.count+1); template_parameters.push_back(p); ++t.count; }
    }
    auto id = templates.size(); templates.push_back(t); return id;
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
    bool dependent = t.kind == TypeKind::PackExpansion;
    if (t.kind == TypeKind::ArgumentPack) {
        auto args = argument_packs[t.bound];
        for (unsigned j = 0; j < args.count; ++j) dependent |= dependent_argument(argument_types[args.offset+j]);
    }
    dependent |= t.kind == TypeKind::DependentName || t.kind == TypeKind::Decltype || t.kind == TypeKind::DependentArray ||
        (t.kind == TypeKind::Named && (entities[t.entity].template_parameter || entities[t.entity].template_pattern));
    if (t.kind == TypeKind::Named && entities[t.entity].specialization) {
        auto pattern = specialization_pattern(t.entity);
        dependent |= entities[pattern].template_parameter;
        dependent |= entities[pattern].template_pattern;
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
    const TypeId failed_substitution = ~TypeId(0);
    auto cache_key = key(owner,pattern);
    if (owner) {
        if (auto known = specialization_type_cache.get(cache_key)) {
            ++substitution_hits; return known == failed_substitution ? 0 : known;
        }
    } else if (auto known = cache.get(pattern)) return known == failed_substitution ? 0 : known;
    ++substitution_work;
    Type p = types[pattern];
    TypeId result = pattern;
    bool complete_failure = false;
    if (p.kind == TypeKind::ArgumentPack) {
        auto args = argument_packs[p.bound]; std::vector<ArgumentId> values;
        for (unsigned j = 0; j < args.count; ++j)
            substitute_arguments(argument_types[args.offset+j],bindings,cache,owner,values);
        for (auto value : values) if (!value) return 0;
        result = make_argument_pack(values);
    } else if (p.kind == TypeKind::PackExpansion) {
        auto arg = substitute_argument(p.bound,bindings,cache,owner);
        if (!arg) return 0;
        result = types.compound(TypeKind::PackExpansion,0,arg);
    } else if (p.kind == TypeKind::DependentArray) {
        auto child = substitute_type(p.child,bindings,cache,owner);
        auto query = substitute_query(p.bound,bindings,cache,owner);
        if (!child || !query) return 0;
        if (query_fact(query).dependent) result = types.compound(TypeKind::DependentArray,child,query);
        else {
            auto value = constants[query_value(query)];
            if (!value.valid || !integral(value.type) || scoped_enum(value.type) || !value.bits ||
                (!is_unsigned(value.type) && static_cast<std::int64_t>(value.bits) < 0))
                return 0;
            if (fundamental(child,FT_VOID) || types[child].kind == TypeKind::Function ||
                types[child].kind == TypeKind::LRef || types[child].kind == TypeKind::RRef || abstract_value(child)) return 0;
            result = types.compound(TypeKind::Array,child,value.bits);
        }
        result = types.qualify(result,p.cv);
    } else if (p.kind == TypeKind::Decltype) {
        auto query = substitute_query(p.entity,bindings,cache,owner);
        if (!query) return 0;
        result = query_decltype(query,p.bound);
        if (result) result = types.qualify(result,p.cv);
        else complete_failure = true;
    } else if (p.kind == TypeKind::DependentName) {
        auto qualifier = substitute_type(p.child,bindings,cache,owner);
        if (!qualifier) return 0;
        std::vector<TypeId> args;
        for (unsigned j = 0; j < p.count; ++j)
            substitute_arguments(types.parameters[p.offset+j],bindings,cache,owner,args);
        for (auto arg : args) if (!arg) return 0;
        result = qualified_type(qualifier,p.entity,args,DependentNameKind(p.bound));
        // A failed lookup is not type zero with qualifiers: qualifying that
        // sentinel would manufacture a fundamental type and admit a candidate.
        if (result) result = types.qualify(result,p.cv);
        else complete_failure = types[qualifier].kind != TypeKind::Named ||
            entities[types[qualifier].entity].complete;
    } else if (p.kind == TypeKind::Named && entities[p.entity].template_parameter) {
        result = owner ? substitution_argument(owner,p.entity) : bindings.get(p.entity);
        if (!result) return 0;
        result = types.qualify(result, p.cv);
    } else if (p.kind == TypeKind::Named && entities[p.entity].template_pattern) {
        // Local declaration identity is an input to substitution, independently
        // of whether the declaration's members use template parameters.
        if (!owner) return pattern;
        if (entities[p.entity].template_info && !substitution_entity(owner,p.entity)) {
            bool concrete_owner = false;
            for (auto frame = owner; frame; frame = substitution_frames[frame].parent)
                concrete_owner |= substitution_frames[frame].specialization != 0;
            if (!concrete_owner) return pattern;
        }
        auto entity = substitution_binding(owner,p.entity);
        result = types.qualify(entities[entity].type,p.cv);
    } else if (p.kind == TypeKind::Named && entities[p.entity].specialization) {
        auto spec = specializations[entities[p.entity].specialization];
        auto pack = argument_packs[spec.arguments];
        std::vector<TypeId> args;
        for (unsigned j = 0; j < pack.count; ++j) {
            auto arg = argument_types[pack.offset+j];
            // A source expansion can occupy a fixed head position and later
            // supply several parameters (Element<I, Ts...>). Expand the source
            // sequence before the applied template repacks its own tail.
            if (argument_pack(arg)) {
                auto tail = pack_arguments(arg);
                for (unsigned k = 0; k < tail.count; ++k)
                    substitute_arguments(argument_types[tail.offset+k],bindings,cache,owner,args);
            } else substitute_arguments(arg,bindings,cache,owner,args);
        }
        for (auto arg : args) if (!arg) return 0;
        auto pattern = spec.pattern;
        if (owner && entities[pattern].template_pattern) {
            auto concrete = substitution_entity(owner,pattern);
            // Inside B<U>, the source class declaration binds to the current
            // specialization. A B<V> template-id still applies its template.
            if (concrete) pattern = entities[concrete].specialization ? specialization_pattern(concrete) : concrete;
            else {
                for (auto frame = owner; frame; frame = substitution_frames[frame].parent)
                    if (substitution_frames[frame].specialization)
                        throw std::logic_error("missing enclosing template declaration binding");
            }
        }
        if (entities[pattern].template_parameter) {
            auto target = owner ? substitution_argument(owner,pattern) : bindings.get(pattern);
            if (!target || value_argument(target) || types[target].kind != TypeKind::Named) return 0;
            pattern = types[target].entity;
            std::vector<ArgumentId> flat;
            for (auto arg : args) {
                if (argument_pack(arg)) {
                    auto pack = pack_arguments(arg);
                    flat.insert(flat.end(),argument_types.begin()+pack.offset,argument_types.begin()+pack.offset+pack.count);
                } else flat.push_back(arg);
            }
            args.swap(flat);
        }
        result = apply_type_template(pattern,args);
        if (result) result = types.qualify(result,p.cv);
    } else if (p.kind == TypeKind::Function) {
        TypeId returned = substitute_type(p.child, bindings, cache,owner);
        if (!returned || types[returned].kind == TypeKind::Array || types[returned].kind == TypeKind::Function) return 0;
        std::vector<TypeId> params;
        for (unsigned i = 0; i < p.count; ++i) {
            auto parameter = types.parameters[p.offset+i];
            if (types[parameter].kind != TypeKind::PackExpansion) {
                auto t = substitute_type(parameter,bindings,cache,owner);
                if (!t || fundamental(t,FT_VOID)) return 0;
                params.push_back(t);
                continue;
            }
            auto begin = params.size();
            substitute_arguments(parameter,bindings,cache,owner,params);
            for (auto j = begin; j < params.size(); ++j) {
                auto t = params[j];
                if (!t || value_argument(t) || fundamental(t,FT_VOID)) return 0;
            }
        }
        result = types.signature(types.function(returned, params, p.variadic, p.cv, p.ref));
    } else if (p.child) {
        TypeId child = substitute_type(p.child, bindings, cache,owner);
        if (!child) return 0;
        bool reference = types[child].kind == TypeKind::LRef || types[child].kind == TypeKind::RRef;
        if (reference && (p.kind == TypeKind::Pointer || p.kind == TypeKind::Array || p.kind == TypeKind::MemberPointer)) return 0;
        if ((p.kind == TypeKind::LRef || p.kind == TypeKind::RRef) && fundamental(child, FT_VOID)) return 0;
        if (p.kind == TypeKind::Array && (fundamental(child, FT_VOID) || types[child].kind == TypeKind::Function || abstract_value(child))) return 0;
        result = p.kind == TypeKind::MemberPointer ? types.member_pointer(p.entity, child) : types.compound(p.kind, child, p.bound);
        result = types.qualify(result, p.cv);
    }
    // A completed dependent-name failure belongs to the same immutable
    // type/frame key as success. Missing bindings and members of an active
    // class can still become available; they are not negative cache facts.
    if (!result && (!complete_failure || incomplete_substitution)) return 0;
    auto stored = result ? result : failed_substitution;
    if (owner) { specialization_type_cache.put(cache_key,stored); ++substitution_records; }
    else cache.put(pattern, stored);
    return result;
}
EntityId Analyzer::specialize(EntityId pattern, const std::vector<TypeId>& input, bool explicit_head)
{
    SubstitutionDependency dependency(incomplete_substitution);
    // Explicit template arguments and deduced arguments establish the same
    // immediate-context obligations. Class completion/body demand temporarily
    // restores hard diagnostics in its own owner.
    struct Probe {
        bool& type; bool saved_type; bool& query; bool saved_query;
        Probe(bool& t, bool& q) : type(t), saved_type(t), query(q), saved_query(q) { type = query = true; }
        ~Probe() { type = saved_type; query = saved_query; }
    } probe(template_type_probe,immediate_query_probe);
    TemplateFunction t = templates[entities[pattern].template_info];
    auto args = input;
    if (definitions && !template_defaults(pattern,args,true)) return 0;
    bool pack_prefix = false;
    if (explicit_head) for (unsigned j = 0; j < t.count; ++j)
        pack_prefix |= entities[template_parameters[t.offset+j]].parameter_pack;
    bool partial = pack_prefix || args.size() < t.count;
    if (partial && !definitions) return 0;
    std::uint32_t pack = intern_arguments(args);
    auto& index_owner = pack_prefix ? explicit_pack_index : specialization_index;
    std::uint32_t previous = index_owner.get(key(pattern, pack));
    if (previous) {
        auto blocked = substitution_prerequisites[incomplete_specializations.get(previous)];
        if (!blocked.query || blocked.revision == query_revisions.get(blocked.query)) {
            if (blocked.query) { incomplete_substitution = blocked.query; record_query_dependency(blocked.query); }
            dependency.succeeded = specializations[previous].entity != 0;
            return specializations[previous].entity;
        }
    }
    Specialization spec; spec.pattern = pattern; spec.arguments = pack; spec.declaration = FactState::Active;
    std::uint32_t index = previous ? previous : specializations.size();
    if (previous) { specializations[index] = spec; substitution_prerequisites[incomplete_specializations.get(index)] = QueryPrerequisite(); }
    else specializations.push_back(spec);
    index_owner.put(key(pattern, pack), index);
    try {
    Index bindings, cache;
    auto condition = members[entities[pattern].member_info].explicit_condition;
    auto frame = dependent_type(entities[pattern].type) || condition || t.parent_frame ?
        substitution_frame(index,t.offset,t.count,t.parent_frame) : 0;
    if (partial && !pack_prefix) {
        // Partial explicit arguments retain every unbound parameter as a typed
        // symbol, including non-type parameters used in array bounds/results.
        auto symbolic = args;
        for (unsigned j = symbolic.size(); j < t.count; ++j)
            symbolic.push_back(parameter_argument(template_parameters[t.offset+j]));
        frame = substitution_frame(index,t.offset,t.count,t.parent_frame,intern_arguments(symbolic));
    }
    if (pack_prefix) {
        frame = 0;
        for (unsigned j = 0; j < t.count; ++j) {
            auto p = template_parameters[t.offset+j];
            bindings.put(p,j < args.size() && !entities[p].parameter_pack ? args[j] : parameter_argument(p));
        }
    }
    TypeId type = substitute_type(entities[pattern].type, bindings, cache,frame);
    if (!type) {
        specializations[index].declaration = FactState::Failure;
        if (incomplete_substitution) retain_query_prerequisite(incomplete_specializations,index);
        return 0;
    }
    if (!partial) check_substituted_type_access(entities[pattern].source,frame);
    EntityId e = make_entity(EntityKind::Function, entities[pattern].owner == t.environment ? scopes[t.environment].parent : entities[pattern].owner, entities[pattern].name, entities[pattern].source);
    entities[e].template_pattern = false;
    entities[e].type = type; entities[e].specialization = index;
    entities[e].constexpr_function = entities[pattern].constexpr_function;
    entities[e].deleted_function = entities[pattern].deleted_function;
    entities[e].inline_function = entities[pattern].inline_function;
    entities[e].is_static = entities[pattern].is_static;
    entities[e].access = entities[pattern].access;
    entities[e].key = entities[pattern].key;
    if (auto suffix = literal_functions.get(pattern)) literal_functions.put(e,suffix);
    entities[e].defaults = entities[pattern].defaults;
    if (partial) {
        t.primary = pattern; t.explicit_arguments = pack;
        entities[e].template_info = templates.size(); templates.push_back(t);
    }
    if (scopes[entities[e].owner].kind == ScopeKind::Class) {
        member_facts(e);
        auto source = members[entities[pattern].member_info];
        auto& member = members[entities[e].member_info];
        member.constructor = source.constructor;
        member.explicit_constructor = source.explicit_constructor;
        member.deleted = source.deleted;
        member.conversion_target = source.conversion_target ? types[type].child : 0;
        if (condition) {
            auto query = substitute_query(condition,bindings,cache,frame);
            if (!query) { specializations[index].declaration = FactState::Failure; return 0; }
            members[entities[e].member_info].explicit_condition = query;
            if (!query_fact(query).dependent)
                members[entities[e].member_info].explicit_constructor = explicit_condition_value(query);
        }
    }
    specializations[index].entity = e; specializations[index].declaration = FactState::Success;
    dependency.succeeded = true;
    return e;
    } catch (...) {
        specializations[index].declaration = FactState::Failure; throw;
    }
}
template<class Arguments>
EntityId Analyzer::deduce_function_values(EntityId pattern, const Arguments& args)
{
    Type f = types[entities[pattern].type];
    bool pack_tail = f.count && types[types.parameters[f.offset+f.count-1]].kind == TypeKind::PackExpansion;
    auto minimum = f.count - unsigned(pack_tail);
    if ((!f.variadic && !pack_tail && args.size() > f.count) ||
        (args.size() < minimum && (!entities[pattern].defaults || !default_arguments[entities[pattern].defaults+args.size()]))) return 0;
    TemplateFunction t = templates[entities[pattern].template_info];
    TypeArguments explicit_args = argument_packs[t.explicit_arguments];
    Index bindings;
    for (unsigned i = 0; i < explicit_args.count; ++i)
        bindings.put(template_parameters[t.offset+i],argument_types[explicit_args.offset+i]);
    for (unsigned i = 0; i < std::min<std::size_t>(f.count,args.size()); ++i) {
        TypeId p = types.parameters[f.offset+i], a = args[i].type;
        if (types[p].kind == TypeKind::PackExpansion) {
            auto element = types[p].bound;
            std::vector<TypeId> actual;
            for (unsigned j = i; j < args.size(); ++j) {
                auto type = args[j].type, param = types[element];
                if (param.kind == TypeKind::RRef && types[param.child].kind == TypeKind::Named &&
                    entities[types[param.child].entity].template_parameter && !types[param.child].cv && args[j].category == ValueCategory::Lvalue)
                    type = types.compound(TypeKind::LRef,type);
                if (type && param.kind != TypeKind::LRef && param.kind != TypeKind::RRef) type = decay(type);
                actual.push_back(type);
            }
            if (types[element].kind == TypeKind::LRef || types[element].kind == TypeKind::RRef) element = types[element].child;
            auto prefix = t.primary ? substitution_frame(entities[pattern].specialization,t.offset,t.count) : 0;
            if (!deduce_expansion(element,actual,bindings,prefix)) return 0;
            break;
        }
        if (!dependent_type(p)) continue;
        // A braced-init-list has no argument type. Outside initializer_list
        // deduction (not part of this stage), the parameter is non-deduced;
        // other arguments, explicit arguments or defaults supply its type.
        if (args[i].form == ExpressionForm::InitializerList) continue;
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
        auto parameter = template_parameters[t.offset+i];
        TypeId a = bindings.get(parameter);
        if (!a && entities[parameter].parameter_pack) a = make_argument_pack({});
        arguments.push_back(a);
    }
    auto primary = t.primary ? t.primary : pattern;
    if (!definitions) for (auto argument : arguments) if (!argument) return 0;
    return deduced_specialization(primary,arguments);
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
    auto shape = types[entities[pattern].type];
    bool pack = shape.count && types[types.parameters[shape.offset+shape.count-1]].kind == TypeKind::PackExpansion;
    auto explicit_args = argument_packs[t.explicit_arguments];
    Index bindings;
    for (unsigned j = 0; j < explicit_args.count; ++j)
        bindings.put(template_parameters[t.offset+j],argument_types[explicit_args.offset+j]);
    if (pack) {
        // A target signature supplies types, not call expressions: preserve
        // references and arrays/functions behind references during deduction.
        auto actual = types[target];
        auto fixed = shape.count-1;
        if (actual.kind != TypeKind::Function || actual.count < fixed || shape.variadic != actual.variadic ||
            !deduce_type(shape.child,actual.child,bindings)) return 0;
        for (unsigned j = 0; j < fixed; ++j)
            if (!deduce_type(types.parameters[shape.offset+j],types.parameters[actual.offset+j],bindings)) return 0;
        std::vector<TypeId> tail(types.parameters.begin()+actual.offset+fixed,types.parameters.begin()+actual.offset+actual.count);
        auto prefix = t.primary ? substitution_frame(entities[pattern].specialization,t.offset,t.count) : 0;
        if (!deduce_expansion(types[types.parameters[shape.offset+fixed]].bound,tail,bindings,prefix)) return 0;
    } else if (!deduce_type(entities[pattern].type,target,bindings)) return 0;
    std::vector<TypeId> args;
    for (unsigned j = 0; j < t.count; ++j) {
        auto parameter = template_parameters[t.offset+j];
        auto a = bindings.get(parameter);
        if (!a && entities[parameter].parameter_pack) a = make_argument_pack({});
        args.push_back(a);
    }
    auto primary = t.primary ? t.primary : pattern;
    auto instance = deduced_specialization(primary,args);
    return instance && entities[instance].type == target ? instance : 0;
}
EntityId Analyzer::explicit_template(NodeId name, EntityId binding, ScopeId s)
{
    NodeId list = child(ast[name].last, Kind::TemplateArguments);
    if (!list) return binding;
    std::vector<TypeId> args;
    for (NodeId a = ast[list].first; a; a = ast[a].next) {
        append_template_argument(a,s,template_argument_node(a,s),args);
    }
    EntityId result = 0;
    for (EntityId e : candidates(binding)) {
        if (!entities[e].template_info) continue;
        EntityId instance = specialize(e, args,true);
        if (instance) result = merge_lookup(result, instance);
    }
    if (!result) throw std::runtime_error("invalid explicit function template arguments");
    return result;
}
void Analyzer::demand_specialization(EntityId e)
{
    if (entities[e].kind != EntityKind::Function) return;
    if (entities[e].specialization) record_default_dependency(DefaultDependencyKind::Specialization,e);
    if (unevaluated_depth) return;
    std::uint32_t i = entities[e].specialization;
    if (!i || specializations[i].emission_demanded) return;
    specializations[i].emission_demanded = true;
    bool first_use = !(entities[e].emission & Entity::Used);
    entities[e].emission |= Entity::Used;
    if (first_use) activate_deferred_function_uses(e);
    specialization_demand.push_back(e);
}
} }
