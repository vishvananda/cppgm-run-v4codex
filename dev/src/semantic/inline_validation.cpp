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
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    if (ast[n].kind != Kind::Call) return false;
    auto callee = ast[n].first, name = ast[callee].detail;
    if (ast[callee].kind != Kind::IdExpression || ast[name].kind != Kind::Name ||
        ast[name].first != ast[name].last) return false;
    auto spelling = ids.spelling(terminal(name));
    if (spelling.size <= 10 || !TextView(spelling.data,10).equals("__builtin_") ||
        hosted_builtin(spelling) || resolve(name,s)) return false;
    // Only a discarded direct call can be deferred without inventing a result
    // type. Its operands and every surrounding statement still require normal
    // semantic validation. A value-dependent use remains an explicit error.
    auto args = ast[callee].next;
    expand_expression_list(args,s);
    for (auto a = ast[args].first; a; a = ast[a].next) expression(a,s);
    deferred_builtin_bodies.put(current_function,n);
    return true;
}
} }
