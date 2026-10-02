#include "semantic/analyzer.h"
#include "support/builtin_registry.h"
namespace cppgm { namespace semantic {
bool Analyzer::deferred_inline_function(EntityId e) const
{
    // A hosted object exports ordinary definitions. An inline namespace
    // function instead needs a definition only after an evaluated use. The
    // explicit LowIR tool without -c retains its standalone export roots.
    return host_abi && e && entities[e].kind == EntityKind::Function &&
        entities[e].inline_function && scopes[entities[e].owner].kind == ScopeKind::Namespace;
}
bool Analyzer::defer_reserved_statement(NodeId n, ScopeId s)
{
    using syntax::Kind;
    if (!deferred_inline_function(current_function)) return false;
    while (ast.kind(n) == Kind::Parenthesized) n = ast.first(n);
    if (ast.kind(n) != Kind::Call) return false;
    auto callee = ast.first(n), name = ast.detail(callee);
    if (ast.kind(callee) != Kind::IdExpression || ast.kind(name) != Kind::Name ||
        ast.first(name) != ast.last(name)) return false;
    auto spelling = ids.spelling(terminal(name));
    if (spelling.size <= 10 || !TextView(spelling.data,10).equals("__builtin_") ||
        hosted_builtin(spelling) || resolve(name,s)) return false;
    // Only a discarded direct call can be deferred without inventing a result
    // type. Its operands and every surrounding statement still require normal
    // semantic validation. A value-dependent use remains an explicit error.
    auto args = ast.next(callee);
    expand_expression_list(args,s);
    for (auto a = ast.first(args); a; a = ast.next(a)) expression(a,s);
    deferred_builtin_bodies.put(current_function,n);
    return true;
}
} }
