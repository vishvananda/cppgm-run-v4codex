#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
bool Analyzer::check_fixed_member(NodeId n, ScopeId s)
{
    auto operand = ast[n].first;
    if (!template_fixed_expressions.get(ast.nodes.occurrences[operand].source)) return false;
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
    use.source_owned = false; return use;
}
} }
