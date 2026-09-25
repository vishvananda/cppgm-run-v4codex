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
        auto block = make_scope(ScopeKind::Block,s,0,0,false); template_pattern_scopes.put(block,1); facts.edit(n).scope = block;
        for (auto c = node.first; c; c = ast[c].next) bind_template_statement(c,block);
        return;
    }
    case Kind::If: case Kind::Switch: case Kind::While: case Kind::Do: case Kind::For: {
        bool loop = node.kind == Kind::While || node.kind == Kind::Do || node.kind == Kind::For;
        bool sw = node.kind == Kind::Switch;
        if (loop) ++loop_depth;
        if (sw) { ++switch_depth; switches.emplace_back(); }
        auto control = make_scope(ScopeKind::Control,s,0,0,false); template_pattern_scopes.put(control,1); facts.edit(n).scope = control;
        for (auto c = node.first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::Condition) bind_template_condition(c,control,sw);
            else if (ast[c].kind == Kind::ForInit || ast[c].kind == Kind::Iteration ||
                ast[c].kind == Kind::Then || ast[c].kind == Kind::Else || ast[c].kind == Kind::Compound)
                bind_template_statement(c,control);
            else bind_template_statement(c,make_scope(ScopeKind::Block,control,0,0,false));
        }
        if (sw) { --switch_depth; switches.pop_back(); }
        if (loop) --loop_depth;
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
    case Kind::Return: bind_template_return(n,s); return;
    case Kind::Break:
        if (!loop_depth && !switch_depth) throw std::runtime_error("break outside loop/switch");
        return;
    case Kind::Continue:
        if (!loop_depth) throw std::runtime_error("continue outside loop");
        return;
    case Kind::ExpressionStatement: case Kind::Iteration:
        for (auto c = node.first; c; c = ast[c].next) {
            bind_template_expression(c,s); bind_template_discarded(c,s);
        }
        return;
    case Kind::Case: {
        if (!switch_depth) throw std::runtime_error("case outside switch");
        bool dependent = bind_template_expression(node.first,s);
        if (!dependent && switches.back().type) {
            auto value = template_statement_value(node.first,s);
            auto constant = evaluate(node.first,s);
            if (value.type) {
                if (!integral(value.type) || !constant.valid) throw std::runtime_error("nonconstant case label");
                auto converted = convert(constant,switches.back().type);
                if (switches.back().labels.get(converted.bits)) throw std::runtime_error("duplicate case label");
                switches.back().labels.put(converted.bits,n);
            }
        }
        bind_template_statement(ast[node.first].next,s); return;
    }
    case Kind::Default:
        if (!switch_depth) throw std::runtime_error("default outside switch");
        if (switches.back().has_default) throw std::runtime_error("duplicate default label");
        switches.back().has_default = true;
        bind_template_statement(node.first,s); return;
    case Kind::ForInit:
        for (auto c = node.first; c; c = ast[c].next) {
            if (ast[c].kind == Kind::SimpleDeclaration) bind_template_declaration(c,s);
            else { bind_template_expression(c,s); bind_template_discarded(c,s); }
        }
        return;
    case Kind::Label: case Kind::Condition:
        for (auto c = node.first; c; c = ast[c].next) {
            if (node.kind == Kind::Condition && ast[c].kind != Kind::ConditionDeclaration) bind_template_expression(c,s);
            else {
                bind_template_statement(c,s);
                if (node.kind == Kind::Condition && ast[c].kind == Kind::ConditionDeclaration) {
                    auto d = ast[ast[c].first].next; facts.edit(n).entity = facts[d].entity;
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
