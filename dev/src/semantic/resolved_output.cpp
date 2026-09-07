#include "semantic/analyzer.h"
#include <ostream>
#include <stdexcept>

namespace cppgm { namespace semantic {
using syntax::Kind;
namespace {
void indent(std::ostream& out, unsigned depth) { for (unsigned i = 0; i < depth; ++i) out << "  "; }
const char* category(ValueCategory c) { return c == ValueCategory::Lvalue ? "lvalue" : c == ValueCategory::Xvalue ? "xvalue" : "prvalue"; }
}
void Analyzer::write_entity_name(std::ostream& out, EntityId e) const
{
    std::vector<ScopeId> owners;
    for (ScopeId s = entities[e].owner; s && s != global; s = scopes[s].parent) {
        if (scopes[s].kind != ScopeKind::Namespace && scopes[s].kind != ScopeKind::Class) { owners.clear(); break; }
        if (scopes[s].name) owners.push_back(s);
    }
    for (auto i = owners.rbegin(); i != owners.rend(); ++i) { spelling(out, scopes[*i].name); out << "::"; }
    spelling(out, entities[e].name);
}
void Analyzer::write_expression(std::ostream& out, NodeId n, unsigned depth, TypeId override_type, ValueCategory override_category) const
{
    if (!n) return;
    const syntax::Node& node = ast[n];
    const Expression& e = expressions[n];
    NodeId first = node.first;
    if (node.kind == Kind::Parenthesized || node.kind == Kind::Initializer || node.kind == Kind::ParenInitializer) {
        write_expression(out, first, depth); return;
    }
    TypeId display = override_type ? override_type : facts[n].type;
    if (e.form == ExpressionForm::Cast && (types[display].kind == TypeKind::LRef || types[display].kind == TypeKind::RRef)) {
        write_expression(out, ast[first].next, depth, display, e.category); return;
    }
    Kind kind = node.kind;
    if (kind == Kind::KeywordLiteral || e.form == ExpressionForm::ConstantQuery ||
        (kind == Kind::IdExpression && e.entity && entities[e.entity].kind == EntityKind::Enumerator)) kind = Kind::Literal;
    if (e.form == ExpressionForm::Cast) kind = Kind::Cast;
    bool zero_cast = kind == Kind::Cast && node.kind == Kind::Call && !ast[ast[first].next].first;
    if (zero_cast) kind = Kind::Literal;
    indent(out, depth);
    out << syntax::kind_name(kind) << ' ' << category(override_type ? override_category : e.category) << ' ';
    write_type(out, display);
    if (kind == Kind::Literal) {
        out << ' ';
        if (zero_cast) out << 0;
        else if (e.form == ExpressionForm::ConstantQuery) out << constants[facts[n].value].bits;
        else if (e.entity && entities[e.entity].kind == EntityKind::Enumerator) {
            Constant c = entities[e.entity].constant;
            if (is_unsigned(c.type)) out << c.bits; else out << static_cast<std::int64_t>(c.bits);
        } else {
            if (node.kind == Kind::KeywordLiteral) out << simple_name(node.op) << ':';
            spelling(out, node.text);
        }
    } else if (kind == Kind::IdExpression) {
        out << ' '; syntax::write_inline(out, ast, node.detail, ids);
    } else if (kind == Kind::Binary || kind == Kind::Assignment || kind == Kind::Unary || kind == Kind::Postfix ||
               (kind == Kind::Cast && node.kind == Kind::Cast)) {
        out << ' ' << simple_name(node.op) << ':';
        if (node.op != OP_LPAREN) spelling(out, node.text);
    } else if (kind == Kind::Member) {
        out << ' ' << simple_name(node.op) << ':'; spelling(out, entities[e.entity].name);
    }
    out << '\n';
    if (kind == Kind::Literal || kind == Kind::IdExpression || kind == Kind::Sizeof) return;
    if (kind == Kind::Call) {
        if (e.entity || e.form == ExpressionForm::Abort) {
            indent(out, depth + 1); out << "callee ";
            if (e.form == ExpressionForm::Abort) out << "__builtin_abort function of () returning void";
            else { write_entity_name(out, e.entity); out << ' '; write_type(out, entities[e.entity].type); }
            out << '\n';
        } else write_expression(out, first, depth + 1);
        unsigned index = 0;
        for (NodeId a = ast[ast[first].next].first; a; a = ast[a].next, ++index) {
            const Conversion& c = conversions[e.conversions + index];
            if (c.temporary) {
                indent(out, depth + 1); out << "cast-expression prvalue "; write_type(out, types[c.target].child); out << '\n';
            }
            write_expression(out, a, depth + 1 + c.temporary);
        }
        return;
    }
    if (kind == Kind::Cast) {
        NodeId operand = node.kind == Kind::Cast ? ast[first].next : ast[ast[first].next].first;
        write_expression(out, operand, depth + 1); return;
    }
    if (kind == Kind::Member) { write_expression(out, first, depth + 1); return; }
    if (kind == Kind::Subscript && types[expressions[first].type].kind != TypeKind::Array && types[expressions[first].type].kind != TypeKind::Pointer) {
        write_expression(out, ast[first].next, depth + 1); write_expression(out, first, depth + 1); return;
    }
    for (NodeId c = first; c; c = ast[c].next) write_expression(out, c, depth + 1);
}
void Analyzer::write_variable(std::ostream& out, NodeId d, NodeId init, unsigned depth) const
{
    EntityId e = facts[d].entity;
    if (!e) return;
    indent(out, depth);
    out << (entities[e].kind == EntityKind::Alias ? "type-alias " : entities[e].kind == EntityKind::Function ? "function-declaration " : "variable ");
    if (entities[e].kind == EntityKind::Function) write_entity_name(out, e); else spelling(out, entities[e].name);
    out << ' '; write_type(out, entities[e].type); out << '\n';
    if (init) write_expression(out, init, depth + 1);
}
void Analyzer::write_resolved(std::ostream& out, NodeId n, unsigned depth) const
{
    if (!n) return;
    const syntax::Node& node = ast[n];
    switch (node.kind) {
    case Kind::Linkage:
        for (NodeId c = node.first; c; c = ast[c].next) write_resolved(out, c, depth);
        return;
    case Kind::TranslationUnit:
        out << "translation-unit\n";
        for (NodeId c = node.first; c; c = ast[c].next) write_resolved(out, c, 1);
        return;
    case Kind::Namespace:
        indent(out, depth); out << "namespace-definition ";
        if (node.text) spelling(out, node.text); else out << "<unnamed>";
        out << '\n';
        for (NodeId c = node.first; c; c = ast[c].next) write_resolved(out, c, depth + 1);
        return;
    case Kind::Alias:
        indent(out, depth); out << "type-alias "; spelling(out, node.text); out << ' ';
        write_type(out, facts[n].type); out << '\n'; return;
    case Kind::Function: {
        EntityId e = facts[n].entity;
        indent(out, depth); out << "function-definition "; write_entity_name(out, e); out << ' ';
        write_type(out, entities[e].type); out << '\n';
        ScopeId fs = facts[n].scope;
        Type function = types[entities[e].type];
        unsigned parameter = 0;
        for (std::uint32_t d = scopes[fs].first_decl; d; d = declarations[d].next) {
            EntityId p = declarations[d].entity;
            if (entities[p].kind != EntityKind::Parameter) continue;
            indent(out, depth + 1); out << "parameter "; spelling(out, entities[p].name); out << ' ';
            write_type(out, types.parameters[function.offset + parameter++]); out << '\n';
        }
        write_resolved(out, ast[ast[node.first].next].next, depth + 1);
        return;
    }
    case Kind::SimpleDeclaration: {
        bool local = facts[node.first].scope && scopes[facts[node.first].scope].kind != ScopeKind::Namespace &&
            scopes[facts[node.first].scope].kind != ScopeKind::Class;
        if (local) { indent(out, depth); out << "simple-declaration\n"; ++depth; }
        NodeId list = child(n, Kind::InitDeclarators);
        for (NodeId c = ast[list].first; c; c = ast[c].next) {
            NodeId d = ast[c].first; write_variable(out, d, ast[d].next, depth);
        }
        return;
    }
    case Kind::ConditionDeclaration: {
        indent(out, depth); out << "condition-declaration\n";
        NodeId d = ast[node.first].next;
        write_variable(out, d, ast[d].next, depth + 1); return;
    }
    case Kind::Class: case Kind::ClassForward: case Kind::Enum: case Kind::UsingDirective: case Kind::UsingDeclaration:
    case Kind::NamespaceAlias: case Kind::EmptyDeclaration: case Kind::Inline: case Kind::StaticAssert: case Kind::Template:
        return;
    case Kind::Compound: case Kind::Return: case Kind::ExpressionStatement: case Kind::If: case Kind::Then: case Kind::Else:
    case Kind::While: case Kind::Do: case Kind::For: case Kind::ForInit: case Kind::Iteration: case Kind::Switch:
    case Kind::Case: case Kind::Default: case Kind::Break: case Kind::Continue: case Kind::Condition:
        indent(out, depth); out << syntax::kind_name(node.kind) << '\n';
        for (NodeId c = node.first; c; c = ast[c].next) write_resolved(out, c, depth + 1);
        return;
    default: write_expression(out, n, depth); return;
    }
}
void Analyzer::write_semantics(std::ostream& out, NodeId root) const { write_resolved(out, root, 0); }
} }
