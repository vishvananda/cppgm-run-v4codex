#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::append_template_argument(NodeId node, ScopeId, ArgumentId arg, std::vector<ArgumentId>& out)
{
    auto context = ast.nodes.occurrences[node].context;
    if (context && !value_argument(arg) && types[arg].kind == TypeKind::PackExpansion) {
        Index bindings, cache;
        substitute_arguments(arg,bindings,cache,template_type_contexts.get(context),out);
    } else out.push_back(arg);
}
std::uint32_t Analyzer::source_expansion_parameters(NodeId root)
{
    auto source = ast.nodes.occurrences[root].source;
    if (auto known = source_expansion_index.get(source)) return known;
    std::vector<NodeId> work(1,root); std::vector<ArgumentId> params; Index seen, parameters;
    for (unsigned j = 0; j < work.size(); ++j) {
        auto n = work[j];
        if (!n || seen.get(n)) continue;
        seen.put(n,1); ++expansion_work;
        auto node = ast[n];
        if (node.kind == Kind::PackExpression || node.kind == Kind::SizeofPack) continue;
        if (node.kind == Kind::Name) {
            auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[n].source)];
            auto e = binding.qualifier_pack ? binding.qualifier_pack : binding.entity;
            if (e && entities[e].parameter_pack && !parameters.get(e)) { parameters.put(e,1); params.push_back(e); }
        }
        if (node.detail) work.push_back(node.detail);
        for (auto c = node.first; c; c = ast[c].next) work.push_back(c);
    }
    auto id = intern_arguments(params); source_expansion_index.put(source,id); return id;
}
ScopeId Analyzer::expanded_scope(NodeId node, ScopeId parent)
{
    auto frame = template_type_contexts.get(ast.nodes.occurrences[node].context);
    return frame && substitution_frames[frame].overlay ? expansion_scope(frame,parent) : parent;
}
ScopeId Analyzer::expansion_scope(std::uint32_t frame, ScopeId parent)
{
    if (expansion_scope_frames.get(parent) == frame) return parent;
    auto k = key(frame,parent);
    if (auto known = expansion_scope_index.get(k)) return known;
    auto f = substitution_frames[frame];
    if (!f.overlay) return parent;
    auto scope = make_scope(ScopeKind::Template,parent);
    auto args = argument_packs[f.overlay];
    for (unsigned j = 0; j < args.count; j += 2) {
        auto parameter = argument_types[args.offset+j], arg = argument_types[args.offset+j+1];
        if (entities[parameter].template_parameter) bind_argument(scope,parameter,arg);
        else bind(scope,entities[parameter].name,arg);
    }
    expansion_scope_frames.put(scope,frame);
    expansion_scope_index.put(k,scope); return scope;
}
void Analyzer::expand_expression_list(NodeId list, ScopeId scope)
{
    auto context = ast.nodes.occurrences[list].context;
    if (!context || expanded_expression_lists.get(list)) return;
    bool expansion = false;
    for (auto a = ast[list].first; a; a = ast[a].next) expansion |= ast[a].kind == Kind::PackExpression || child(a,Kind::PackExpansion);
    if (!expansion) return;
    std::vector<NodeId> result; Index bindings;
    auto frame = template_type_contexts.get(context);
    for (auto a = ast[list].first; a; a = ast[a].next) {
        if (ast[a].kind != Kind::PackExpression && !child(a,Kind::PackExpansion)) { result.push_back(a); continue; }
        auto pattern = ast[a].kind == Kind::PackExpression ? ast[a].first : a;
        auto params = source_expansion_parameters(pattern);
        auto count = expansion_count(params,bindings,frame);
        if (count < 0) throw std::runtime_error("expression expansion requires an unexpanded pack");
        for (int j = 0; j < count; ++j) {
            auto lane = expansion_frame(frame,params,j);
            auto node = ast.instantiate(pattern,expansion_context(lane));
            facts.resize(ast.nodes.size()); expressions.resize(ast.nodes.size());
            facts.edit(node).scope = expansion_scope(lane,scope);
            result.push_back(node);
        }
    }
    ast.expanded_children(list,result); expanded_expression_lists.put(list,1);
}
} }
