#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

Parser::Parser(Cursor& cursor, Ast& tree, IdentifierTable& identifiers)
    : in(cursor), ast(tree), ids(identifiers) {}

NodeId Parser::make(Kind kind)
{
    NodeId node = ast.make(kind);
    ast[node].location = in.peek().location;
    return node;
}

NodeId Parser::leaf(Kind kind) { return ast.make(kind, in.take()); }

NodeId Parser::wrap(Kind kind, NodeId child)
{
    NodeId node = make(kind);
    ast.append(node, child);
    if (child) ast[node].location = ast[child].location;
    return node;
}

NodeId Parser::named(Kind kind, NodeId name)
{
    NodeId node = make(kind);
    ast[node].detail = name;
    if (name) ast[node].location = ast[name].location;
    return node;
}

bool Parser::identifier(std::size_t ahead)
{
    return in.peek(ahead).kind == PostTokenKind::identifier;
}

bool Parser::builtin(std::size_t ahead)
{
    switch (in.peek(ahead).op) {
    case KW_VOID: case KW_BOOL: case KW_CHAR: case KW_WCHAR_T:
    case KW_CHAR16_T: case KW_CHAR32_T: case KW_SHORT: case KW_INT:
    case KW_LONG: case KW_SIGNED: case KW_UNSIGNED: case KW_FLOAT:
    case KW_DOUBLE: case KW_AUTO: return true;
    default: return false;
    }
}

bool Parser::type_start(std::size_t ahead)
{
    if (builtin(ahead)) return true;
    switch (in.peek(ahead).op) {
    case KW_CONST: case KW_VOLATILE: case KW_TYPENAME: case KW_DECLTYPE:
    case KW_STRUCT: case KW_CLASS: case KW_UNION: case KW_ENUM: return true;
    default: break;
    }
    NameProbe probe = probe_name(ahead);
    if (!probe.valid || !probe.terminal || probe.special) return false;
    if (probe.binding.category != Category::Unknown) return type_category(probe.binding.category);
    return lexical_hint(probe.terminal) & 1;
}

unsigned char Parser::lexical_hint(IdentifierId id)
{
    if (!id) return 0;
    if (lexical_hints.size() <= id) lexical_hints.resize(id + 1);
    if (lexical_hints[id] & 4) return lexical_hints[id];
    unsigned char hint = 4;
    TextView spelling = ids.spelling(id);
    if (ast.telemetry) hint_bytes += spelling.size;
    for (std::size_t i = 0; i < spelling.size; ++i) {
        char ch = spelling.data[i];
        if (ch == 'T') hint |= 3;
        else if (ch == 'C' || ch == 'Y' || ch == 'E') hint |= 1;
    }
    lexical_hints[id] = hint;
    return hint;
}

bool Parser::declaration_start()
{
    switch (in.peek().op) {
    case KW_TYPEDEF: case KW_EXTERN: case KW_STATIC: case KW_INLINE:
    case KW_VIRTUAL: case KW_CONSTEXPR: case KW_THREAD_LOCAL: case KW_FRIEND:
    case KW_EXPLICIT: case KW_NAMESPACE: case KW_USING: case KW_TEMPLATE:
    case KW_STATIC_ASSERT: return true;
    default: return type_start();
    }
}

NodeId Parser::translation_unit()
{
    NodeId root = make(Kind::TranslationUnit);
    while (in.peek().kind != PostTokenKind::eof) {
        std::size_t before = in.consumed;
        ast.append(root, declaration());
        if (before == in.consumed) throw std::logic_error("declaration made no progress");
    }
    return root;
}

void Parser::balanced(const char* open, const char* close)
{
    in.require(open);
    while (!in.is(close)) {
        if (in.peek().kind == PostTokenKind::eof) throw std::runtime_error("unterminated attribute");
        if (in.is("(")) balanced("(", ")");
        else if (in.is("[")) balanced("[", "]");
        else if (in.is("{")) balanced("{", "}");
        else in.take();
    }
    in.take();
}

void Parser::attributes()
{
    for (;;) {
        if (in.is("[") && in.is("[", 1)) balanced("[", "]");
        else if (in.is("alignas") || in.is("__attribute__") || in.is("__attribute")) {
            in.take();
            balanced("(", ")");
        } else break;
    }
}

} }
