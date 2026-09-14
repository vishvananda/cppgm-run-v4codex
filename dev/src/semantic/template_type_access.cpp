#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
void Analyzer::retain_type_access(NodeId part, TypeId qualifier, ScopeId scope)
{
    auto occurrence = ast.nodes.occurrences[part];
    if (occurrence.context || template_type_access_sources.get(occurrence.source)) return;
    TemplateTypeAccess use; use.qualifier = qualifier; use.name = ast[part].text; use.scope = scope;
    template_type_access_sources.put(occurrence.source,template_type_accesses.size());
    template_type_accesses.push_back(use);
}
bool Analyzer::type_access_subtree(NodeId node)
{
    if (!node) return false;
    auto source = ast.nodes.occurrences[node].source;
    if (auto known = template_type_access_subtrees.get(source)) return known == 2;
    // Source edges are immutable. Bodies and constructor initializers have
    // their own later type demands; signature checks do not traverse them.
    auto n = ast.nodes[node];
    using syntax::Kind;
    if (n.kind == Kind::Compound || n.kind == Kind::FunctionTry || n.kind == Kind::CtorInitializer)
        return false;
    bool present = template_type_access_sources.get(source);
    present |= type_access_subtree(n.detail);
    for (auto c = n.first; c; c = ast.nodes[c].next) present |= type_access_subtree(c);
    template_type_access_subtrees.put(source,present ? 2 : 1); return present;
}
void Analyzer::check_substituted_type_access(NodeId node, std::uint32_t frame)
{
    if (!frame || template_type_accesses.size() == 1 || !type_access_subtree(node)) return;
    // Recipes belong to the template definition, even when the specialization
    // is first demanded by an exempt explicit-instantiation declarator.
    struct DefinitionAccess {
        bool& flag; bool saved;
        DefinitionAccess(bool& f) : flag(f), saved(f) { flag = false; }
        ~DefinitionAccess() { flag = saved; }
    } access(explicit_instantiation_naming);
    std::vector<NodeId> work(1,node);
    for (std::size_t j = 0; j < work.size(); ++j) {
        auto n = work[j]; auto source = ast.nodes.occurrences[n].source;
        if (auto recipe = template_type_access_sources.get(source)) {
            auto k = key(frame,recipe);
            auto state = FactState(template_type_access_states.get(k));
            if (state == FactState::Failure) throw std::runtime_error("failed substituted type access");
            if (state == FactState::NotStarted) {
                auto use = template_type_accesses[recipe];
                template_type_access_states.put(k,unsigned(FactState::Active));
                try {
                    Index bindings, cache;
                    auto params = argument_packs[expansion_parameters(use.qualifier)];
                    std::vector<ArgumentId> unexpanded;
                    for (unsigned i = 0; i < params.count; ++i) {
                        auto p = argument_types[params.offset+i];
                        if (argument_pack(substitution_argument(frame,p))) unexpanded.push_back(p);
                    }
                    auto pack = unexpanded.empty() ? 0 : intern_arguments(unexpanded);
                    auto count = pack ? expansion_count(pack,bindings,frame) : 1;
                    bool complete = count >= 0;
                    for (int lane = 0; lane < count; ++lane) {
                        auto environment = pack ? expansion_frame(frame,pack,lane) : frame;
                        auto qualifier = substitute_type(use.qualifier,bindings,cache,environment);
                        if (!qualifier || dependent_type(qualifier)) { complete = false; continue; }
                        if (types[qualifier].kind != TypeKind::Named) throw std::runtime_error("invalid substituted type qualifier");
                        auto scope = entities[types[qualifier].entity].scope;
                        auto member = qualified_type_member(qualifier,use.name);
                        if (!member) throw std::runtime_error("substituted type member not found");
                        check_access(member,substitution_scope(environment,use.scope),scope);
                        ++template_type_access_work;
                        template_type_access_states.put(key(environment,recipe),unsigned(FactState::Success));
                    }
                    template_type_access_states.put(k,unsigned(complete ? FactState::Success : FactState::NotStarted));
                } catch (...) { template_type_access_states.put(k,unsigned(FactState::Failure)); throw; }
            }
        }
        auto source_node = ast.nodes[n];
        if (type_access_subtree(source_node.detail)) work.push_back(source_node.detail);
        for (auto c = source_node.first; c; c = ast.nodes[c].next)
            if (type_access_subtree(c)) work.push_back(c);
    }
}
} }
