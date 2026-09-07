#include "syntax/ast.h"
#include <ostream>
#include <string>

namespace cppgm { namespace syntax {
namespace {
void spelling(std::ostream& out, const IdentifierTable& ids, IdentifierId id)
{
    if (!id) return;
    TextView view = ids.spelling(id);
    out.write(view.data, view.size);
}

void children_inline(std::ostream& out, const Ast& ast, NodeId id,
                     const IdentifierTable& ids, const char* separator = "")
{
    bool first = true;
    for (NodeId child = ast[id].first; child; child = ast[child].next) {
        if (!first) out << separator;
        write_inline(out, ast, child, ids);
        first = false;
    }
}

bool token_label(Kind kind)
{
    switch (kind) {
    case Kind::DeclSpecifier: case Kind::TypeSpecifier: case Kind::Pointer:
    case Kind::KeywordLiteral: case Kind::Binary: case Kind::Assignment:
    case Kind::Unary: case Kind::Postfix: case Kind::Member: case Kind::Cast:
    case Kind::TypeTrait: case Kind::ClassKey: case Kind::EnumKey:
    case Kind::Access: case Kind::CvQualifier: case Kind::ParameterKey:
    case Kind::Specifier: case Kind::LambdaSpecifier: case Kind::Virtual:
    case Kind::VirtSpecifier: case Kind::PackExpansion: return true;
    default: return false;
    }
}

void payload(std::ostream& out, const Ast& ast, NodeId id, const IdentifierTable& ids)
{
    const Node& node = ast[id];
    if (node.op == KW_DECLTYPE) {
        out << ' ';
        write_inline(out, ast, id, ids);
    } else if (node.kind == Kind::FunctionQualifier && (node.op == KW_NOEXCEPT || node.op == KW_THROW)) {
        out << ' ';
        spelling(out, ids, node.text);
        if (node.detail) write_inline(out, ast, node.detail, ids);
        else if (node.first) {
            out << '(';
            children_inline(out, ast, id, ids);
            out << ')';
        }
    } else if (node.kind == Kind::Noexcept) {
        // Lambda noexcept owns the operand directly.
    } else if (node.kind == Kind::Placement) {
        out << ' ';
        children_inline(out, ast, id, ids);
    } else if (node.kind == Kind::LambdaIntroducer) {
        out << ' ';
        write_inline(out, ast, id, ids);
    } else if (node.kind == Kind::ParameterPack || node.kind == Kind::Ellipsis) out << " ...";
    else if (node.detail) {
        out << ' ';
        if (node.kind == Kind::DeclSpecifier && node.text) out << "TT_IDENTIFIER:";
        write_inline(out, ast, node.detail, ids);
        if (node.kind == Kind::Pointer) out << "::*";
    } else if (node.text || node.op != TOK_INVALID) {
        out << ' ';
        if (node.kind == Kind::Literal && (node.flags & 2)) out << "TT_LITERAL:";
        if (token_label(node.kind) && !(node.kind == Kind::Specifier && node.op == KW_EXPLICIT)) out << (node.op == TOK_INVALID ? "TT_IDENTIFIER" : simple_name(node.op)) << ':';
        if (node.kind == Kind::Linkage) {
            TextView text = ids.spelling(node.text);
            out.write(text.data + 1, text.size - 2);
        } else spelling(out, ids, node.text);
    } else if (node.kind == Kind::Namespace) out << " <unnamed>";
}
}

void write_inline(std::ostream& out, const Ast& ast, NodeId id, const IdentifierTable& ids)
{
    if (!id) return;
    const Node& n = ast[id];
    if (n.op == KW_DECLTYPE && n.kind != Kind::NamePart) {
        out << "decltype(";
        children_inline(out, ast, id, ids);
        out << ')';
    } else if (n.kind == Kind::Capture) {
        spelling(out, ids, n.text);
        write_inline(out, ast, n.detail, ids);
        children_inline(out, ast, id, ids);
    } else if (n.kind == Kind::ParameterPack || n.kind == Kind::Ellipsis) out << "...";
    else if (n.kind == Kind::Name) {
        if (n.op == OP_COLON2) out << "::";
        children_inline(out, ast, id, ids, "::");
    } else if (n.kind == Kind::NamePart) {
        if (n.flags & 1) out << "template ";
        if (n.op == OP_COMPL) out << '~';
        spelling(out, ids, n.text);
        if (n.flags & 2) out << ' ';
        write_inline(out, ast, n.detail, ids);
        children_inline(out, ast, id, ids);
    } else if (n.kind == Kind::TemplateArguments) {
        out << '<';
        children_inline(out, ast, id, ids, ",");
        out << '>';
    } else if (n.kind == Kind::Binary || n.kind == Kind::Assignment) {
        write_inline(out, ast, n.first, ids);
        spelling(out, ids, n.text);
        write_inline(out, ast, n.last, ids);
    } else if (n.kind == Kind::Conditional) {
        write_inline(out, ast, n.first, ids);
        out << '?';
        write_inline(out, ast, ast[n.first].next, ids);
        out << ':';
        write_inline(out, ast, n.last, ids);
    } else if (n.kind == Kind::Unary) {
        spelling(out, ids, n.text);
        children_inline(out, ast, id, ids);
    } else if (n.kind == Kind::Postfix) {
        children_inline(out, ast, id, ids);
        spelling(out, ids, n.text);
    } else if (n.kind == Kind::Parenthesized || n.kind == Kind::Parameters ||
               n.kind == Kind::Arguments || n.kind == Kind::ParenArguments || n.kind == Kind::ParenInitializer || n.kind == Kind::NestedDeclarator) {
        out << '(';
        children_inline(out, ast, id, ids, n.kind == Kind::NestedDeclarator ? "" : ",");
        out << ')';
    } else if (n.kind == Kind::Array || n.kind == Kind::LambdaIntroducer) {
        out << '[';
        children_inline(out, ast, id, ids, ",");
        out << ']';
    } else if (n.kind == Kind::BracedInit) {
        out << '{';
        children_inline(out, ast, id, ids, ",");
        out << '}';
    } else if (n.kind == Kind::Member || n.kind == Kind::Subscript) {
        write_inline(out, ast, n.first, ids);
        if (n.kind == Kind::Member) spelling(out, ids, n.text);
        else out << '[';
        write_inline(out, ast, n.last, ids);
        if (n.kind == Kind::Subscript) out << ']';
    } else if (n.kind == Kind::Cast) {
        if (n.op != OP_LPAREN) spelling(out, ids, n.text);
        out << (n.op == OP_LPAREN ? '(' : '<');
        write_inline(out, ast, n.first, ids);
        out << (n.op == OP_LPAREN ? ')' : '>');
        if (n.op != OP_LPAREN) out << '(';
        write_inline(out, ast, n.last, ids);
        if (n.op != OP_LPAREN) out << ')';
    } else if (n.kind == Kind::New) {
        NodeId child = n.first;
        if (child && ast[child].kind == Kind::Global) {
            out << "::";
            child = ast[child].next;
        }
        out << "new";
        for (; child; child = ast[child].next) write_inline(out, ast, child, ids);
    } else if (n.kind == Kind::TypeSpecifiers || n.kind == Kind::DeclSpecifiers)
        children_inline(out, ast, id, ids, " ");
    else if (n.kind == Kind::PackExpression) {
        children_inline(out, ast, id, ids);
        out << "...";
    } else if (n.kind == Kind::Sizeof || n.kind == Kind::TypeTrait || n.kind == Kind::Decltype) {
        if (n.kind == Kind::Sizeof) out << "sizeof";
        else if (n.kind == Kind::Decltype) out << "decltype";
        else spelling(out, ids, n.text);
        out << '(';
        children_inline(out, ast, id, ids);
        out << ')';
    } else {
        if (n.flags & 1) out << "typename ";
        if (n.detail) write_inline(out, ast, n.detail, ids);
        else spelling(out, ids, n.text);
        if (n.kind == Kind::Pointer && n.detail) out << "::*";
        children_inline(out, ast, id, ids);
    }
}

void write_ast(std::ostream& out, const Ast& ast, NodeId root, const IdentifierTable& ids)
{
    // An explicit traversal stack avoids recursive tree destruction/rendering.
    struct Work { NodeId node; unsigned depth; };
    std::vector<Work> stack(1, Work{root, 0});
    while (!stack.empty()) {
        Work work = stack.back();
        stack.pop_back();
        const Node& node = ast[work.node];
        out << std::string(work.depth * 2, ' ') << kind_name(node.kind);
        payload(out, ast, work.node, ids);
        out << '\n';
        if (node.next) stack.push_back(Work{node.next, work.depth});
        if (node.first && node.kind != Kind::LambdaIntroducer)
            stack.push_back(Work{node.first, work.depth + 1});
    }
}

} }
