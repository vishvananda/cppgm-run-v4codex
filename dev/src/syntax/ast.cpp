#include "syntax/ast.h"
#include <limits>
#include <stdexcept>

namespace cppgm { namespace syntax {

Ast::Ast(bool stats) : telemetry(stats), nodes(1), locations(1), literals(1) {}

NodeId Ast::make(Kind kind, Token token)
{
    if (nodes.size() >= std::numeric_limits<NodeId>::max())
        throw std::runtime_error("syntax node capacity exceeded");
    Node node;
    node.kind = kind;
    node.op = token.op;
    node.text = token.text;
    node.location = token.location;
    node.literal = token.literal;
    if (telemetry && nodes.size() == nodes.capacity()) ++node_growths;
    nodes.push_back(node);
    return static_cast<NodeId>(nodes.size() - 1);
}

void Ast::append(NodeId parent, NodeId child)
{
    if (!child) return;
    if (nodes[parent].last) nodes[nodes[parent].last].next = child;
    else nodes[parent].first = child;
    nodes[parent].last = child;
}

std::uint32_t Ast::save_literal(const PostToken& token, IdentifierId prefix)
{
    LiteralValue value;
    value.kind = token.literal;
    value.type = token.type;
    value.suffix = token.suffix;
    value.prefix = prefix;
    value.scalar = token.scalar;
    value.offset = literal_bytes.size();
    value.bytes = token.data.size;
    value.elements = token.elements;
    literal_bytes.insert(literal_bytes.end(), token.data.data, token.data.data + token.data.size);
    if (telemetry && literals.size() == literals.capacity()) ++literal_growths;
    literals.push_back(value);
    return literals.size() - 1;
}

const char* kind_name(Kind kind)
{
    static const char* const names[] = {
        "translation-unit",
        "empty-declaration",
        "simple-declaration",
        "decl-specifier-seq",
        "decl-specifier",
        "type-specifier-seq",
        "type-specifier",
        "type-name",
        "type-id",
        "declarator",
        "abstract-declarator",
        "nested-declarator",
        "ptr-operator",
        "array-suffix",
        "parameter-clause",
        "parameter-declaration",
        "default-argument",
        "init-declarator-list",
        "init-declarator",
        "initializer",
        "paren-initializer",
        "function-definition",
        "identifier",
        "id-expression",
        "literal",
        "keyword-literal",
        "binary-expression",
        "assignment-expression",
        "conditional-expression",
        "unary-expression",
        "postfix-expression",
        "parenthesized-expression",
        "call-expression",
        "argument-list",
        "subscript-expression",
        "member-expression",
        "braced-init-list",
        "cast-expression",
        "sizeof-expression",
        "sizeof-pack-expression",
        "type-trait-expression",
        "new-expression",
        "delete-expression",
        "global-scope",
        "array-delete",
        "placement",
        "paren-argument-list",
        "lambda-expression",
        "lambda-introducer",
        "lambda-declarator",
        "lambda-specifier",
        "compound-statement",
        "expression-statement",
        "return-statement",
        "throw-statement",
        "break-statement",
        "continue-statement",
        "goto-statement",
        "labeled-statement",
        "case-statement",
        "default-statement",
        "if-statement",
        "switch-statement",
        "while-statement",
        "do-statement",
        "for-statement",
        "for-init-statement",
        "range-for-statement",
        "range-declaration",
        "range-initializer",
        "condition",
        "condition-declaration",
        "iteration",
        "then",
        "else",
        "try-block",
        "function-try-block",
        "handler",
        "exception-declaration",
        "ellipsis",
        "namespace-definition",
        "namespace-alias-definition",
        "using-directive",
        "using-declaration",
        "alias-declaration",
        "target",
        "inline",
        "linkage-specification",
        "static-assert-declaration",
        "message",
        "class-specifier",
        "class-forward-declaration",
        "class-key",
        "access-specifier",
        "base-clause",
        "base-specifier",
        "base-name",
        "virtual",
        "enum-specifier",
        "enum-key",
        "enumerator",
        "bit-field-declaration",
        "bit-field-declarator",
        "special-member-declaration",
        "special-member-definition",
        "member-specifiers",
        "specifier",
        "special-initializer",
        "ctor-initializer",
        "mem-initializer",
        "mem-initializer-id",
        "cv-qualifier",
        "function-qualifier",
        "virt-specifier",
        "noexcept-specification",
        "trailing-return-type",
        "decltype-specifier",
        "template-declaration",
        "template-parameter-clause",
        "template-parameter-list",
        "type-parameter",
        "non-type-template-parameter",
        "template-template-parameter",
        "parameter-key",
        "parameter-pack",
        "default-template-argument",
        "explicit-instantiation-declaration",
        "pack-expansion",
        "pack-expansion-expression",
        "name",
        "name-part",
        "template-arguments",
        "capture",
    };
    return names[static_cast<unsigned>(kind)];
}

} }
