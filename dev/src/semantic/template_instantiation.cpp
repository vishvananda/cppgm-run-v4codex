#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::demand_region(NodeId root)
{
    if (!root || !ast.pending_region(root)) return;
    auto context = ast.nodes.occurrences[root].context;
    ast.instantiate(root,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
}
void Analyzer::instantiate_function(EntityId e)
{
    auto index = entities[e].specialization;
    if (index && specializations[index].body == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::FunctionDefinition,e,entities[e].source);
    if (!index || specializations[index].body != FactState::NotStarted) return;
    auto spec = specializations[index];
    auto pattern = templates[entities[spec.pattern].template_info];
    if (!pattern.body) return;
    specializations[index].body = FactState::Active; ++template_bodies;
    try {
    auto context = spec.context ? spec.context : ast.new_context();
    auto source = ast.instantiate(pattern.source,context);
    auto declarator = ast.projected(pattern.declarator,context);
    auto body = ast.projected(pattern.body,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    ScopeId environment = specialization_environment(e);
    specializations[index].context = context;
    specializations[index].environment = environment;
    auto frame = substitution_frame(index,pattern.offset,pattern.count);
    attach_template_context(context,frame);
    instantiate_parameters(pattern.declarator,context,frame,environment);
    function_body({body,declarator,environment,e,source});
    specializations[index].body = FactState::Success;
    } catch (...) {
        specializations[index].body = FactState::Failure; throw;
    }
}
void Analyzer::instantiate_parameters(NodeId d, std::uint32_t context, std::uint32_t frame, ScopeId environment)
{
    // The source signature owns raw parameter types. Preserve body cv, array
    // and function forms independently from the adjusted callable signature.
    Index bindings, cache;
    NodeId parameters = 0;
    while (d) {
        if (auto p = child(d,syntax::Kind::Parameters)) parameters = p;
        auto nested = child(d,syntax::Kind::NestedDeclarator);
        d = nested ? ast[nested].first : 0;
    }
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != syntax::Kind::Parameter) continue;
        auto occurrence = ast.projected(p,context);
        if (!occurrence) throw std::logic_error("signature parameter has no occurrence identity");
        if (facts[occurrence].type) continue;
        auto type = facts[p].type;
        if (!type) throw std::logic_error("missing retained template parameter type");
        auto concrete = substitute_type(type,bindings,cache,frame);
        if (!concrete) throw std::runtime_error("invalid instantiated parameter type");
        { auto& published = facts.edit(occurrence); published.type = concrete; published.scope = environment; }
        ++parameter_publications;
    }
}
ScopeId Analyzer::default_environment(EntityId e, ScopeId head)
{
    auto index = entities[e].specialization;
    auto spec = specializations[index];
    auto pattern = templates[entities[spec.pattern].template_info];
    if (head == pattern.environment && pattern.body) return specialization_environment(e);
    auto k = key(index,head);
    if (auto known = default_environments.get(k)) return known;
    // A default belongs to its declaring head, independently of a later body
    // definition. This immutable overlay contains only that head's parameters.
    auto environment = make_scope(ScopeKind::Template,entities[spec.pattern].owner);
    ++default_environment_work;
    auto pack = argument_packs[spec.arguments]; unsigned ordinal = 0;
    for (auto d = scopes[head].first_decl; d; d = declarations[d].next) {
        auto parameter = declarations[d].entity;
        if (!entities[parameter].template_parameter) continue;
        if (ordinal == pack.count) throw std::logic_error("default head argument count mismatch");
        bind_argument(environment,parameter,argument_types[pack.offset+ordinal++]);
    }
    if (ordinal != pack.count) throw std::logic_error("incomplete default argument head");
    default_environments.put(k,environment); return environment;
}
NodeId Analyzer::instantiate_default(EntityId e, NodeId source)
{
    auto index = entities[e].specialization;
    auto context = specializations[index].context;
    if (!context) specializations[index].context = context = ast.new_context();
    auto root = ast.instantiate(source,context);
    facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
    facts.edit(root).scope = default_environment(e,facts[source].scope);
    return root;
}
} }
