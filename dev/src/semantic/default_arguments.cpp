#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
enum class SourceBindingState : unsigned char { NotStarted, Queued, Active, Complete, Failed };
}
void Analyzer::bind_template_defaults(NodeId d, ScopeId s, ScopeId head, bool allowed)
{
    if (!d || ast.nodes.occurrences[d].context) return;
    auto source = ast.nodes.occurrences[d].source;
    auto state = SourceBindingState(template_default_bindings.get(source));
    if (state == SourceBindingState::Complete) return;
    if (state == SourceBindingState::Active) throw std::runtime_error("recursive source default argument binding");
    if (state == SourceBindingState::Failed) throw FailedSemanticFact(SemanticFact::DefaultBinding,0,d);
    if (state == SourceBindingState::Queued && active_template_class) return;
    NodeId parameters = 0;
    for (auto node = d; node;) {
        if (auto p = child(node,Kind::Parameters)) parameters = p;
        auto nested = child(node,Kind::NestedDeclarator); node = nested ? ast[nested].first : 0;
    }
    bool needed = false;
    for (auto p = ast[parameters].first; p; p = ast[p].next) needed |= child(p,Kind::DefaultArgument) != 0;
    if (!needed) { template_default_bindings.put(source,unsigned(SourceBindingState::Complete)); return; }
    if (!allowed) {
        template_default_bindings.put(source,unsigned(SourceBindingState::Failed));
        throw std::runtime_error("class template member default must appear on its initial declaration");
    }
    // Default arguments see the whole enclosing class, including declarations
    // that follow this member and defaults in nested class member functions.
    // The source class completion event drains just its collected consumers.
    if (active_template_class && scopes[s].kind == ScopeKind::Class) {
        template_default_bindings.put(source,unsigned(SourceBindingState::Queued));
        ++template_default_binding_queued;
        template_class_uses.push_back({d,s,head,0,TemplateClassUseKind::DefaultArgument}); return;
    }
    template_default_bindings.put(source,unsigned(SourceBindingState::Active));
    ++template_default_binding_work;
    try {
    auto scope = make_scope(ScopeKind::Block,s,0,0,false);
    template_pattern_scopes.put(scope,1);
    if (head && head != s) for (auto d = scopes[head].first_decl; d; d = declarations[d].next) {
        auto parameter = declarations[d].entity;
        if (entities[parameter].template_parameter) bind(scope,entities[parameter].name,parameter);
    }
    unsigned ordinal = 0;
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != Kind::Parameter) continue;
        auto specs = ast[p].first, decl = ast[specs].next;
        auto type = facts[p].type;
        if (!type) type = bind_template_type(specs,decl,scope);
        auto name = terminal(decl_name(decl));
        auto e = make_entity(EntityKind::Parameter,scope,name,p);
        entities[e].template_pattern = true;
        entities[e].parameter_pack = child(decl,Kind::ParameterPack) != 0;
        entities[e].type = type ? parameter_body_type(type) : 0;
        template_pattern_entities.put(e,!type || dependent_type(type) ? 2 : 1);
        signature_parameters.put(e,++ordinal); bind(scope,name,e);
        // Fixed names are definition-time obligations. Dependent calls/types
        // retain their bindings without demanding a concrete default value.
        auto argument = child(p,Kind::DefaultArgument);
        bool dependent = bind_template_expression(argument,scope);
        if (argument && !dependent && type && !dependent_type(type))
            check_template_initialization(ast[argument].first,types.adjusted(type),scope,InitializationMode::Copy);
    }
    template_default_bindings.put(source,unsigned(SourceBindingState::Complete));
    } catch (...) {
        template_default_bindings.put(source,unsigned(SourceBindingState::Failed)); throw;
    }
}
void Analyzer::bind_template_initializer(EntityId e, ScopeId scope)
{
    auto state = SourceBindingState(template_initializer_bindings.get(e));
    if (state == SourceBindingState::Complete) return;
    if (state == SourceBindingState::Active) throw std::runtime_error("recursive source member initializer binding");
    if (state == SourceBindingState::Failed) throw FailedSemanticFact(SemanticFact::InitializerBinding,e,entities[e].initializer);
    if (state == SourceBindingState::Queued && active_template_class) return;
    if (active_template_class && !entities[e].is_static && scopes[scope].kind == ScopeKind::Class) {
        template_initializer_bindings.put(e,unsigned(SourceBindingState::Queued));
        ++template_initializer_binding_queued;
        template_class_uses.push_back({entities[e].initializer,scope,0,e,TemplateClassUseKind::MemberInitializer});
        return;
    }
    template_initializer_bindings.put(e,unsigned(SourceBindingState::Active));
    ++template_initializer_binding_work;
    try {
        if (!entities[e].initializer) {
            bind_template_default_initialization(e,scope);
            template_initializer_bindings.put(e,unsigned(SourceBindingState::Complete)); return;
        }
        bool dependent = bind_template_expression(entities[e].initializer,scope);
        auto type = entities[e].type;
        if (types[type].kind == TypeKind::Array && !types[type].bound) {
            type = complete_array_initializer(entities[e].initializer,type,scope,true);
            entities[e].type = type;
            dependent |= !types[type].bound;
        } else if (type) check_template_initialization(entities[e].initializer,type,scope);
        if (dependent) template_pattern_entities.put(e,2);
        if (!dependent && type && integral(type) && types[type].cv == 1) {
            auto value = evaluate(entities[e].initializer,scope);
            if (value.valid) entities[e].constant = convert(value,type);
        }
        template_initializer_bindings.put(e,unsigned(SourceBindingState::Complete));
    } catch (...) {
        template_initializer_bindings.put(e,unsigned(SourceBindingState::Failed)); throw;
    }
}
void Analyzer::function_defaults(EntityId e, NodeId d, ScopeId s, NodeId source)
{
    Type f = types[entities[e].type];
    if (!f.count) return;
    auto head = definitions && entities[e].template_info ?
        (active_template_scope ? active_template_scope : templates[entities[e].template_info].environment) : 0;
    if (head) bind_template_defaults(d,s,head);
    if (!entities[e].defaults) {
        if (default_arguments.empty()) default_arguments.push_back(0);
        entities[e].defaults = default_arguments.size();
        default_arguments.resize(default_arguments.size() + f.count);
    }
    NodeId params = 0;
    while (d) {
        NodeId candidate = child(d, Kind::Parameters);
        if (candidate) params = candidate;
        NodeId nested = child(d, Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    unsigned i = 0;
    bool seen = false;
    for (NodeId p = ast[params].first; p && i < f.count; p = ast[p].next, ++i) {
        NodeId a = child(p, Kind::DefaultArgument);
        unsigned index = entities[e].defaults + i;
        if (a) {
            if (head && source != entities[e].source)
                throw std::runtime_error("function template default added by a later declaration");
            if (default_arguments[index]) throw std::runtime_error("duplicate default argument");
            // Keep the declaration slot immutable. Function specializations
            // share the source slot but own separate concrete default facts.
            default_arguments[index] = a; facts.edit(a).scope = head ? head : s;
            // A member of a local class in a function specialization is also
            // a templated entity, though its class has no specialization ID.
            if (head || (definitions && (entities[e].template_pattern || entities[e].template_member ||
                    (scopes[s].kind == ScopeKind::Class && ast.nodes.occurrences[a].context)))) {
                seen = true; continue;
            }
            if (class_depth) declaration_defaults.push_back({e,i});
            else default_argument(e,i,0,DefaultReason::Declaration);
        }
        if (default_arguments[index]) seen = true;
        else if (seen) throw std::runtime_error("missing trailing default argument");
    }
}
std::uint64_t Analyzer::default_argument_key(EntityId e, unsigned parameter) const
{
    while (members[entities[e].member_info].inherited_constructor)
        e = members[entities[e].member_info].inherited_constructor;
    // Inherited constructors share their original declaration's slots/types.
    // Function template specializations additionally supply the substitution.
    return key(definitions && entities[e].specialization ? e : 0, entities[e].defaults+parameter);
}
NodeId Analyzer::default_argument_value(EntityId e, unsigned parameter) const
{
    auto id = default_argument_index.get(default_argument_key(e,parameter));
    if (!id || default_argument_facts[id].state != FactState::Success)
        throw std::logic_error("missing checked default argument");
    return default_argument_facts[id].value;
}
NodeId Analyzer::default_argument(EntityId e, unsigned parameter, Conversion* converted, DefaultReason reason)
{
    while (members[entities[e].member_info].inherited_constructor)
        e = members[entities[e].member_info].inherited_constructor;
    auto source = default_arguments[entities[e].defaults+parameter];
    if (!source) throw std::logic_error("missing default argument declaration");
    auto k = default_argument_key(e,parameter);
    auto id = default_argument_index.get(k);
    if (!id) {
        id = default_argument_facts.size(); default_argument_facts.emplace_back();
        default_argument_index.put(k,id);
    }
    if (reason != DefaultReason::Declaration) record_default_dependency(DefaultDependencyKind::Argument,id);
    default_argument_facts[id].reasons |= static_cast<unsigned char>(reason);
    auto state = default_argument_facts[id].state;
    if (state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::DefaultArgument,e,source);
    if (state == FactState::Active) throw std::runtime_error("recursive default argument");
    if (state == FactState::NotStarted) {
        default_argument_facts[id].state = FactState::Active; ++default_argument_work;
        auto saved_default = active_default_fact;
        auto saved_unevaluated = unevaluated_depth;
        active_default_fact = id; unevaluated_depth = 1;
        try {
            auto root = definitions && entities[e].specialization ? instantiate_default(e,source) : source;
            demand_region(root);
            auto scope = facts[root].scope;
            auto value = ast[root].first;
            while (ast[value].kind == Kind::Initializer || ast[value].kind == Kind::ParenInitializer)
                value = ast[value].first;
            expression(value,scope);
            auto type = types[entities[e].type];
            auto c = conversion(value,types.parameters[type.offset+parameter]);
            // Validate copy-initialization once, in the declaring environment.
            // The immutable recipe contains no per-call conversion temporary.
            check_fixed_conversion(expressions[value],value,c,scope);
            capture_default_conversion(c);
            auto conversion_id = conversions.size(); conversions.push_back(c);
            if (reason == DefaultReason::Declaration) {
                // Preserve the ordinary initializer view without instantiating
                // specializations before this default is used [temp.inst]/10.
                auto applied = copy_conversion_recipe(c); apply_conversion(value,applied);
                expressions.incoming(value,conversions.size()); conversions.push_back(applied);
            }
            auto& fact = default_argument_facts[id];
            fact.root = root; fact.value = value; fact.conversion = conversion_id;
            fact.state = FactState::Success;
        } catch (...) {
            active_default_fact = saved_default; unevaluated_depth = saved_unevaluated;
            default_argument_facts[id].state = FactState::Failure; throw;
        }
        active_default_fact = saved_default; unevaluated_depth = saved_unevaluated;
    }
    if (reason == DefaultReason::Argument && !active_default_fact) demand_default_fact(id);
    auto fact = default_argument_facts[id];
    if (converted) *converted = copy_conversion_recipe(conversions[fact.conversion]);
    return fact.value;
}
void Analyzer::record_default_dependency(DefaultDependencyKind kind, std::uint32_t target)
{
    if (!active_default_fact || !target || (kind != DefaultDependencyKind::Argument && unevaluated_depth != 1)) return;
    auto next = default_argument_facts[active_default_fact].dependencies;
    default_dependencies.emplace_back(target,next,kind);
    default_argument_facts[active_default_fact].dependencies = default_dependencies.size();
}
void Analyzer::capture_default_conversion(const Conversion& c)
{
    // Runtime materialization records ordinary conversion calls, including
    // whether a copy is elided. Only deferred template definitions need a
    // separate dependency here; demanding an ordinary elided copy would also
    // mark its unused entry for emission.
    if (c.function && (entities[c.function].template_member || entities[c.function].specialization)) {
        record_default_dependency(DefaultDependencyKind::Member,c.function);
        if (entities[c.function].specialization) record_default_dependency(DefaultDependencyKind::Specialization,c.function);
    }
    auto destruction = [&](TypeId t) {
        if (auto dtor = type_destructor(value_type(t))) record_default_dependency(DefaultDependencyKind::Member,dtor);
    };
    if (c.kind == Conversion::Kind::Construction) {
        destruction(c.target);
        auto call = conversion_objects[c.materialization].call;
        for (unsigned i = 0; i < call.count; ++i) capture_default_conversion(conversions[call.conversions+i]);
    } else if (c.kind == Conversion::Kind::User) {
        destruction(types[entities[c.function].type].child);
        capture_default_conversion(user_conversions[c.materialization].result);
    } else if (c.kind == Conversion::Kind::ListPlan) {
        auto plan = list_plans[c.materialization];
        if (!plan.direct_binding) destruction(plan.target);
        for (unsigned i = 0; i < plan.call.count; ++i) capture_default_conversion(conversions[plan.call.conversions+i]);
        if (plan.constructor) for (unsigned i = plan.explicit_count; i < plan.call.argument_count; ++i) {
            auto child = default_argument_index.get(default_argument_key(plan.constructor,i));
            if (!child) throw std::logic_error("list recipe lost its checked default");
            record_default_dependency(DefaultDependencyKind::Argument,child);
        }
    }
}
void Analyzer::demand_default_fact(std::uint32_t id)
{
    auto state = default_argument_facts[id].demand;
    if (state == FactState::Success || state == FactState::Active) return;
    if (state == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::DefaultDemand,0,default_argument_facts[id].root);
    default_argument_facts[id].demand = FactState::Active; ++default_demand_work;
    auto saved_unevaluated = unevaluated_depth; unevaluated_depth = 0;
    try {
        // A default is a separate definition [temp.decls]/2. Using it in a
        // call requires its initializer's dependencies even inside decltype.
        // A sizeof within the initializer did not record evaluated edges.
        for (auto edge = default_argument_facts[id].dependencies; edge;) {
            auto use = default_dependencies[edge-1]; edge = use.next; ++default_dependency_work;
            switch (use.kind) {
            case DefaultDependencyKind::Member: demand_member(use.target,MemberDemandReason::DefaultArgument); break;
            case DefaultDependencyKind::Specialization: demand_specialization(use.target); break;
            case DefaultDependencyKind::Storage: demand_template_storage(use.target); break;
            case DefaultDependencyKind::Argument: demand_default_fact(use.target); break;
            }
        }
        expressions.evaluated(default_argument_facts[id].value,true);
        default_argument_facts[id].demand = FactState::Success;
    } catch (...) {
        unevaluated_depth = saved_unevaluated;
        default_argument_facts[id].demand = FactState::Failure; throw;
    }
    unevaluated_depth = saved_unevaluated;
}
} }
