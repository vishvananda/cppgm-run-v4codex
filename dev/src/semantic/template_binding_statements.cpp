#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::bind_template_statement(NodeId n, ScopeId s)
{
    if (!n) return;
    ++template_binding_work;
    auto node = ast[n];
    switch (node.kind) {
    case Kind::Compound: {
        auto block = make_scope(ScopeKind::Block,s,0,0,false); template_pattern_scopes.put(block,1); facts[n].scope = block;
        for (auto c = node.first; c; c = ast[c].next) bind_template_statement(c,block);
        return;
    }
    case Kind::If: case Kind::Switch: case Kind::While: case Kind::Do: case Kind::For: {
        auto control = make_scope(ScopeKind::Control,s,0,0,false); template_pattern_scopes.put(control,1); facts[n].scope = control;
        for (auto c = node.first; c; c = ast[c].next) bind_template_statement(c,control);
        return;
    }
    case Kind::Then: case Kind::Else: {
        auto block = ast[node.first].kind == Kind::Compound ? s : make_scope(ScopeKind::Block,s,0,0,false);
        bind_template_statement(node.first,block); return;
    }
    case Kind::SimpleDeclaration: case Kind::Alias: case Kind::UsingDirective: case Kind::UsingDeclaration:
    case Kind::NamespaceAlias: case Kind::StaticAssert: case Kind::Class: case Kind::ClassForward: case Kind::Enum:
    case Kind::ConditionDeclaration:
        bind_template_declaration(n,s); return;
    case Kind::Return: case Kind::ExpressionStatement: case Kind::Iteration:
        for (auto c = node.first; c; c = ast[c].next) bind_template_expression(c,s);
        return;
    case Kind::Case:
        bind_template_expression(node.first,s); bind_template_statement(ast[node.first].next,s); return;
    case Kind::Default: case Kind::Label: case Kind::Condition: case Kind::ForInit:
        for (auto c = node.first; c; c = ast[c].next) {
            if (node.kind == Kind::Condition && ast[c].kind != Kind::ConditionDeclaration) bind_template_expression(c,s);
            else {
                bind_template_statement(c,s);
                if (node.kind == Kind::Condition && ast[c].kind == Kind::ConditionDeclaration) {
                    auto d = ast[ast[c].first].next; facts[n].entity = facts[d].entity;
                }
            }
        }
        return;
    default:
        for (auto c = node.first; c; c = ast[c].next) bind_template_expression(c,s);
        return;
    }
}
} }
