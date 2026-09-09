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
        if (scopes[s].kind != ScopeKind::Namespace && scopes[s].kind != ScopeKind::Class) { break; }
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
    if (e.incoming && !override_type) {
        const Conversion& c = conversions[e.incoming];
        if ((c.derived && c.kind != Conversion::Kind::Explicit) || c.temporary) {
            indent(out, depth++); out << "cast-expression " << category(c.derived ? e.category : ValueCategory::Prvalue) << ' ';
            write_type(out, c.reference ? types[c.target].child : c.target); out << '\n';
        }
    }
    if (node.kind == Kind::Parenthesized || node.kind == Kind::Initializer || node.kind == Kind::ParenInitializer) {
        write_expression(out, first, depth); return;
    }
    TypeId display = override_type ? override_type : facts[n].type;
    if (e.form == ExpressionForm::Cast && types[display].kind == TypeKind::MemberPointer) {
        write_expression(out, ast[first].next, depth); return;
    }
    if (node.kind == Kind::IdExpression && e.entity && scopes[entities[e.entity].owner].kind == ScopeKind::Class) {
        EntityId cls = scopes[entities[e.entity].owner].entity;
        EntityId storage = class_facts[entities[cls].class_info].storage;
        if (storage) {
            indent(out, depth); out << "member-expression " << category(e.category) << ' '; write_type(out, display);
            out << ' '; spelling(out, entities[e.entity].name); out << '\n';
            indent(out, depth + 1); out << "id-expression lvalue "; write_type(out, entities[storage].type);
            out << ' '; spelling(out, entities[storage].name); out << '\n'; return;
        }
    }
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
        EntityId selected = facts[n].entity;
        if (selected || e.form == ExpressionForm::Abort) {
            indent(out, depth + 1); out << "callee ";
            if (e.form == ExpressionForm::Abort) out << "__builtin_abort function of () returning void";
            else { write_entity_name(out, selected); out << ' '; write_type(out, entities[selected].type); }
            out << '\n';
        } else write_expression(out, first, depth + 1);
        for (NodeId a = ast[ast[first].next].first; a; a = ast[a].next) write_expression(out, a, depth + 1);
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
    write_object(out, e, init, depth);
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
        write_function(out, facts[n].entity, ast[ast[node.first].next].next, facts[n].scope, depth);
        return;
    }
    case Kind::SimpleDeclaration: {
        bool local = facts[node.first].scope && scopes[facts[node.first].scope].kind != ScopeKind::Namespace &&
            scopes[facts[node.first].scope].kind != ScopeKind::Class;
        if (local) { indent(out, depth); out << "simple-declaration\n"; ++depth; }
        NodeId list = child(n, Kind::InitDeclarators);
        if (!list) {
            for (NodeId c = ast[node.first].first; c; c = ast[c].next) {
                if (ast[c].kind != Kind::Class) continue;
                EntityId cls = facts[c].entity, storage = class_facts[entities[cls].class_info].storage;
                if (storage) write_object(out, storage, 0, depth);
            }
        }
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
    case Kind::SpecialMember: case Kind::SpecialDefinition:
        if (child(child(n, Kind::Initializer), Kind::SpecialInitializer) || node.kind == Kind::SpecialDefinition)
            write_function(out, facts[n].entity, child(n, Kind::Compound), facts[n].scope, depth);
        return;
    case Kind::Class: {
        EntityId cls = facts[n].entity;
        EntityId storage = class_facts[entities[cls].class_info].storage;
        if (storage) {
            bool local = scopes[entities[storage].owner].kind != ScopeKind::Namespace;
            if (local) { indent(out, depth++); out << "simple-declaration\n"; }
            write_object(out, storage, 0, depth);
        }
        return;
    }
    case Kind::Enum:
        if (scopes[facts[n].scope].kind == ScopeKind::Block) { indent(out, depth); out << "simple-declaration\n"; }
        return;
    case Kind::ClassForward: case Kind::UsingDirective: case Kind::UsingDeclaration:
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
void Analyzer::write_semantics(std::ostream& out, NodeId root) const
{
    write_resolved(out, root, 0);
    for (EntityId e : specialization_demand) {
        write_function(out, e, 0, 0, 1, false);
    }
    for (EntityId e : demand_queue) {
        const MemberFacts& m = members[entities[e].member_info];
        if ((m.destructor || (m.synthetic && m.constructor)) && !m.source_demand) continue;
        write_function(out, e, m.body, entities[e].scope, 1);
    }
}
} }
