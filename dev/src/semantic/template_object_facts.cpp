#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::check_fixed_member(NodeId n, ScopeId s)
{
    auto operand = ast[n].first;
    if (!template_fixed_expressions.get(ast.nodes.occurrences[operand].source)) return false;
    // A fixed receiver does not make its explicit template-id fixed. The
    // retained argument facts are substituted by the occurrence/lane owner.
    auto name = ast[ast[operand].next].detail;
    for (auto p = ast[name].first; p; p = ast[p].next)
        if (auto args = child(p,syntax::Kind::TemplateArguments))
            for (auto a = ast[args].first; a; a = ast[a].next)
                if (dependent_argument(template_argument_node(a,s))) return false;
    ++unevaluated_depth;
    try {
        expression(n,s);
        if (auto id = expressions[n].object_use) {
            if (object_uses[id].temporary) throw std::logic_error("source member owns a concrete temporary");
            object_uses[id].source_owned = true;
        }
    } catch (...) { --unevaluated_depth; throw; }
    --unevaluated_depth; return true;
}
ObjectUse Analyzer::project_object_use(ObjectUse use, NodeId n) const
{
    if (use.temporary) throw std::logic_error("source receiver owns a concrete temporary");
    auto context = ast.nodes.occurrences[n].context;
    if (!context) return use;
    auto project = [&](NodeId source) {
        auto node = ast.projected(source,context);
        if (source && !node) throw std::logic_error("missing concrete receiver expression");
        return node;
    };
    use.node = project(use.node); use.member_pointer = project(use.member_pointer);
    use.callee = project(use.callee);
    use.source_owned = false; return use;
}
} }
