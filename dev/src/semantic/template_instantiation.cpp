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
    if (entities[e].explicit_specialization || instantiation_suppressed(e)) return;
    auto index = entities[e].specialization;
    if (index && specializations[index].body == FactState::Failure)
        throw FailedSemanticFact(SemanticFact::FunctionDefinition,e,entities[e].source);
    if (!index || specializations[index].body != FactState::NotStarted) return;
    auto spec = specializations[index];
    instantiate_member_definition(spec.pattern);
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
    auto parent = pattern.parent_frame;
    if (pattern.source_count)
        parent = substitution_frame(index,pattern.source_parameters,pattern.source_count,parent);
    auto frame = substitution_frame(index,pattern.offset,pattern.count,parent);
    attach_template_context(context,frame);
    auto signature = template_signature_sources.get(ast.nodes.occurrences[pattern.declarator].source);
    if (auto first = template_first_signature_index.get(spec.pattern)) {
        // A redeclaration's callable type and body parameters share the first
        // declaration's lookup. For a member template this recipe already
        // includes its enclosing class substitution and renamed member head.
        auto selected = template_first_signatures[first].selected_parameters;
        if (selected) signature = selected;
    }
    instantiate_parameters(signature ? signature : pattern.declarator,context,frame,environment);
    if (auto m = entities[e].member_info) {
        members[m].source = source; members[m].declarator = declarator;
        members[m].body = body; members[m].body_environment = environment;
    }
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
    std::vector<NodeId> expanded;
    bool has_pack = false;
    for (auto p = ast[parameters].first; p; p = ast[p].next) {
        if (ast[p].kind != syntax::Kind::Parameter) continue;
        auto occurrence = ast.projected(p,context);
        if (!occurrence) throw std::logic_error("signature parameter has no occurrence identity");
        auto type = facts[p].type;
        check_substituted_type_access(p,frame);
        if (child(ast[ast[p].first].next,syntax::Kind::ParameterPack)) {
            if (!has_pack) {
                // Ordinary parameter lists retain their projected source edges;
                // allocate an override only when a pack changes the topology.
                for (auto q = ast[parameters].first; q != p; q = ast[q].next)
                    if (ast[q].kind == syntax::Kind::Parameter) expanded.push_back(ast.projected(q,context));
                has_pack = true;
            }
            auto params = expansion_parameters(type);
            auto count = expansion_count(params,bindings,frame);
            if (count < 0) throw std::runtime_error("unbound function parameter pack");
            for (int j = 0; j < count; ++j) {
                auto lane = expansion_frame(frame,params,j);
                auto node = ast.instantiate(p,expansion_context(lane));
                facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
                auto concrete = substitute_type(type,bindings,cache,lane);
                if (!concrete) throw std::runtime_error("invalid expanded function parameter");
                auto& f = facts.edit(node); f.type = concrete; f.scope = environment;
                expanded.push_back(node); ++parameter_publications;
            }
            continue;
        }
        if (has_pack) expanded.push_back(occurrence);
        if (facts[occurrence].type) continue;
        if (!type) throw std::logic_error("missing retained template parameter type");
        auto concrete = substitute_type(type,bindings,cache,frame);
        if (!concrete) throw std::runtime_error("invalid instantiated parameter type");
        { auto& published = facts.edit(occurrence); published.type = concrete; published.scope = environment; }
        ++parameter_publications;
    }
    if (has_pack) ast.expanded_children(ast.projected(parameters,context),expanded);
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
