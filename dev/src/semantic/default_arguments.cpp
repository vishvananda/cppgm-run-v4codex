#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
enum class DefaultBindingState : unsigned char { NotStarted, Queued, Active, Complete, Failed };
}
void Analyzer::bind_template_defaults(NodeId d, ScopeId s, ScopeId head, bool allowed)
{
    if (!d || ast.nodes.occurrences[d].context) return;
    auto source = ast.nodes.occurrences[d].source;
    auto state = DefaultBindingState(template_default_bindings.get(source));
    if (state == DefaultBindingState::Complete) return;
    if (state == DefaultBindingState::Active) throw std::runtime_error("recursive source default argument binding");
    if (state == DefaultBindingState::Failed) throw std::runtime_error("failed source default argument binding");
    if (state == DefaultBindingState::Queued && active_template_class) return;
    NodeId parameters = 0;
    for (auto node = d; node;) {
        if (auto p = child(node,Kind::Parameters)) parameters = p;
        auto nested = child(node,Kind::NestedDeclarator); node = nested ? ast[nested].first : 0;
    }
    bool needed = false;
    for (auto p = ast[parameters].first; p; p = ast[p].next) needed |= child(p,Kind::DefaultArgument) != 0;
    if (!needed) { template_default_bindings.put(source,unsigned(DefaultBindingState::Complete)); return; }
    if (!allowed) throw std::runtime_error("class template member default must appear on its initial declaration");
    // Default arguments see the whole enclosing class, including declarations
    // that follow this member and defaults in nested class member functions.
    // The source class completion event drains just its collected consumers.
    if (active_template_class && scopes[s].kind == ScopeKind::Class) {
        template_default_bindings.put(source,unsigned(DefaultBindingState::Queued));
        ++template_default_binding_queued;
        template_pending_defaults.push_back({d,s,head}); return;
    }
    template_default_bindings.put(source,unsigned(DefaultBindingState::Active));
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
        entities[e].type = type ? parameter_body_type(type) : 0;
        template_pattern_entities.put(e,!type || dependent_type(type) ? 2 : 1);
        signature_parameters.put(e,++ordinal); bind(scope,name,e);
        // Fixed names are definition-time obligations. Dependent calls/types
        // retain their bindings without demanding a concrete default value.
        bind_template_expression(child(p,Kind::DefaultArgument),scope);
    }
    template_default_bindings.put(source,unsigned(DefaultBindingState::Complete));
    } catch (...) {
        template_default_bindings.put(source,unsigned(DefaultBindingState::Failed)); throw;
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
            // Class specialization declares member defaults without demanding
            // their expressions. Preserve the declaration's access environment.
            if (definitions && ast.nodes.occurrences[a].context) {
                default_arguments[index] = a; facts.edit(a).scope = s;
                seen = true; continue;
            }
            NodeId value = ast[a].first;
            while (ast[value].kind == Kind::Initializer || ast[value].kind == Kind::ParenInitializer)
                value = ast[value].first;
            if (definitions && entities[e].template_info) {
                default_arguments[index] = a; facts.edit(a).scope = head;
                seen = true; continue;
            }
            if (ast[value].kind == Kind::BracedInit) {
                expression(value,s); require_conversion(value,types.parameters[f.offset+i]);
            } else initialize(ast[a].first,types.parameters[f.offset+i],s);
            default_arguments[index] = value;
        }
        if (default_arguments[index]) seen = true;
        else if (seen) throw std::runtime_error("missing trailing default argument");
    }
}
NodeId Analyzer::default_argument(EntityId e, unsigned parameter)
{
    if (definitions && entities[e].specialization) return instantiate_default(e,parameter);
    auto slot = entities[e].defaults+parameter;
    auto root = default_arguments[slot];
    if (ast[root].kind != Kind::DefaultArgument) return root;
    auto state = default_argument_states.get(root);
    if (state == unsigned(FactState::Active)) throw std::runtime_error("recursive member default argument");
    if (state == unsigned(FactState::Failure)) throw std::runtime_error("failed member default argument");
    default_argument_states.put(root,unsigned(FactState::Active));
    ++default_argument_work;
    try {
        demand_region(root);
        auto scope = facts[root].scope;
        auto type = types[entities[e].type];
        auto target = types.parameters[type.offset+parameter];
        auto value = ast[root].first;
        while (ast[value].kind == Kind::Initializer || ast[value].kind == Kind::ParenInitializer)
            value = ast[value].first;
        if (ast[value].kind == Kind::BracedInit) {
            expression(value,scope); require_conversion(value,target);
        } else initialize(ast[root].first,target,scope);
        default_arguments[slot] = value;
        default_argument_states.put(root,unsigned(FactState::Success));
        return value;
    } catch (...) {
        default_argument_states.put(root,unsigned(FactState::Failure));
        throw;
    }
}
} }
