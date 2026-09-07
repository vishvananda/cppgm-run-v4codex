#include "semantic/analyzer.h"
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::resolve_condition(NodeId n, ScopeId s, bool is_switch)
{
    NodeId c = ast[n].first;
    if (!c) return;
    TypeId t;
    if (ast[c].kind == Kind::ConditionDeclaration) {
        NodeId specs = ast[c].first, d = ast[specs].next;
        t = declarator(d, specifiers(specs, s), s);
        declare_object(d, ast[d].next, t, specs, s, c);
    } else t = expression(c, s).type;
    if (!t) throw std::runtime_error("unresolved condition");
    if (is_switch) {
        if (!integral(t)) throw std::runtime_error("switch requires integral or enum condition");
    } else if (scoped_enum(t) || (!arithmetic(t) && !pointer(decay(t)))) throw std::runtime_error("invalid boolean condition");
    facts[n].type = is_switch ? promote(t) : types.fundamental(FT_BOOL);
}
void Analyzer::resolve_statement(NodeId n, ScopeId s)
{
    if (!n) return;
    switch (ast[n].kind) {
    case Kind::Compound: {
        ScopeId bs = make_scope(ScopeKind::Block, s);
        facts[n].scope = bs;
        for (NodeId c = ast[n].first; c; c = ast[c].next) resolve_statement(c, bs);
        return;
    }
    case Kind::If: case Kind::Switch: case Kind::While: case Kind::Do: case Kind::For: {
        bool loop = ast[n].kind == Kind::While || ast[n].kind == Kind::Do || ast[n].kind == Kind::For;
        bool sw = ast[n].kind == Kind::Switch;
        ScopeId control = make_scope(ScopeKind::Block, s);
        facts[n].scope = control;
        if (loop) ++loop_depth;
        if (sw) ++switch_depth;
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::Condition) resolve_condition(c, control, sw);
            else if (ast[c].kind == Kind::ForInit || ast[c].kind == Kind::Iteration || ast[c].kind == Kind::Then || ast[c].kind == Kind::Else)
                resolve_statement(c, control);
            else resolve_statement(c, ast[c].kind == Kind::Compound ? control : make_scope(ScopeKind::Block, control));
        }
        if (sw) --switch_depth;
        if (loop) --loop_depth;
        return;
    }
    case Kind::Then: case Kind::Else: {
        NodeId c = ast[n].first;
        resolve_statement(c, ast[c].kind == Kind::Compound ? s : make_scope(ScopeKind::Block, s)); return;
    }
    case Kind::SimpleDeclaration: case Kind::Alias: case Kind::UsingDirective: case Kind::UsingDeclaration:
    case Kind::NamespaceAlias: case Kind::StaticAssert: case Kind::Class: case Kind::ClassForward: case Kind::Enum:
        declaration(n, s); return;
    case Kind::Return:
        if (ast[n].first) {
            Expression value = expression(ast[n].first, s);
            if (fundamental(return_type, FT_VOID) && !fundamental(value.type, FT_VOID)) throw std::runtime_error("value returned from void");
            require_conversion(ast[n].first, return_type);
        } else if (!fundamental(return_type, FT_VOID)) throw std::runtime_error("missing return value");
        return;
    case Kind::Break:
        if (!loop_depth && !switch_depth) throw std::runtime_error("break outside loop/switch");
        return;
    case Kind::Continue:
        if (!loop_depth) throw std::runtime_error("continue outside loop");
        return;
    case Kind::Case: {
        if (!switch_depth) throw std::runtime_error("case outside switch");
        NodeId c = ast[n].first;
        Expression x = expression(c, s);
        if (!integral(x.type) || !evaluate(c, s).valid) throw std::runtime_error("nonconstant case label");
        resolve_statement(ast[c].next, s); return;
    }
    case Kind::Default:
        if (!switch_depth) throw std::runtime_error("default outside switch");
        resolve_statement(ast[n].first, s); return;
    case Kind::ExpressionStatement: case Kind::ForInit: case Kind::Iteration:
        for (NodeId c = ast[n].first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::SimpleDeclaration) declaration(c, s);
            else if (expression(c, s).form == ExpressionForm::Overload) throw std::runtime_error("unresolved discarded overload");
        }
        return;
    default: throw std::runtime_error("unsupported statement");
    }
}
} }
