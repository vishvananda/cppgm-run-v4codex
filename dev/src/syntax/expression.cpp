#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {
namespace {
int precedence(ETokenType op)
{
    switch (op) {
    case OP_COMMA: return 1;
    case OP_ASS: case OP_PLUSASS: case OP_MINUSASS: case OP_STARASS:
    case OP_DIVASS: case OP_MODASS: case OP_XORASS: case OP_BANDASS:
    case OP_BORASS: case OP_LSHIFTASS: case OP_RSHIFTASS: return 2;
    case OP_QMARK: return 3;
    case OP_LOR: return 4;
    case OP_LAND: return 5;
    case OP_BOR: return 6;
    case OP_XOR: return 7;
    case OP_AMP: return 8;
    case OP_EQ: case OP_NE: return 9;
    case OP_LT: case OP_GT: case OP_LE: case OP_GE: return 10;
    case OP_LSHIFT: case OP_RSHIFT: return 11;
    case OP_PLUS: case OP_MINUS: return 12;
    case OP_STAR: case OP_DIV: case OP_MOD: return 13;
    case OP_DOTSTAR: case OP_ARROWSTAR: return 14;
    default: return 0;
    }
}
}

NodeId Parser::expression(int minimum)
{
    NodeId left = unary();
    for (;;) {
        Token op = in.peek();
        int p = precedence(op.op);
        if (p < minimum || (angle_expression && (op.op == OP_GT || op.op == OP_RSHIFT))) break;
        in.take();
        NodeId node = ast.make(p == 2 ? Kind::Assignment : Kind::Binary, op);
        ast.append(node, left);
        if (op.op == OP_QMARK) {
            ast[node].kind = Kind::Conditional;
            ast[node].text = 0;
            ast[node].op = TOK_INVALID;
            ast.append(node, expression());
            in.require(":");
            ast.append(node, expression(2));
        } else ast.append(node, expression(p == 2 ? p : p + 1));
        left = node;
    }
    return left;
}

NodeId Parser::arguments(Kind kind, const char* close)
{
    unsigned saved = angle_expression;
    angle_expression = 0;
    NodeId result = make(kind);
    if (!in.is(close)) {
        do {
            if (in.is(close)) break;
            NodeId arg = expression(2);
            if (in.eat("...")) arg = wrap(Kind::PackExpression, arg);
            ast.append(result, arg);
        } while (in.eat(","));
    }
    in.require(close);
    angle_expression = saved;
    return result;
}

NodeId Parser::postfix(NodeId base)
{
    for (;;) {
        NodeId result;
        if (in.eat("(")) {
            result = wrap(Kind::Call, base);
            ast.append(result, arguments(ast[base].kind == Kind::IdExpression && ast[base].op != TOK_INVALID ?
                                          Kind::ParenArguments : Kind::Arguments, ")"));
        } else if (in.eat("[")) {
            result = wrap(Kind::Subscript, base);
            unsigned saved = angle_expression;
            angle_expression = 0;
            ast.append(result, expression());
            in.require("]");
            angle_expression = saved;
        } else if (in.is(".") || in.is("->")) {
            result = leaf(Kind::Member);
            ast.append(result, base);
            ast.append(result, named(Kind::Identifier, name()));
        } else if (in.is("++") || in.is("--")) {
            result = leaf(Kind::Postfix);
            ast.append(result, base);
        } else break;
        base = result;
    }
    return base;
}

NodeId Parser::primary()
{
    Token token = in.peek();
    if (token.kind == PostTokenKind::literal || token.kind == PostTokenKind::user_literal)
        return leaf(Kind::Literal);
    if (in.is("true") || in.is("false") || in.is("nullptr") || in.is("this"))
        return leaf(Kind::KeywordLiteral);
    if (in.eat("(")) {
        unsigned saved = angle_expression;
        angle_expression = 0;
        NodeId result = wrap(Kind::Parenthesized, expression());
        in.require(")");
        angle_expression = saved;
        return result;
    }
    if (in.eat("{")) return arguments(Kind::BracedInit, "}");
    if (in.is("[")) return lambda();
    if (builtin()) return leaf(Kind::IdExpression);
    if (identifier() || in.is("::") || in.is("operator") || in.is("decltype"))
        return named(Kind::IdExpression, name());
    TextView text = ids.spelling(token.text);
    throw std::runtime_error("expected expression, found '" + std::string(text.data, text.size) + "'");
}

NodeId Parser::unary()
{
    if (in.is("++") || in.is("--") || in.is("*") || in.is("&") ||
        in.is("+") || in.is("-") || in.is("!") || in.is("~")) {
        NodeId node = leaf(Kind::Unary);
        ast.append(node, unary());
        return node;
    }
    if (in.is("sizeof") || in.is("typeid") || in.is("alignof") || in.is("noexcept"))
        return postfix(type_trait());
    if (in.is("static_cast") || in.is("dynamic_cast") || in.is("reinterpret_cast") || in.is("const_cast")) {
        NodeId node = leaf(Kind::Cast);
        in.require("<");
        ast.append(node, type_id());
        in.close_angle();
        in.require("(");
        unsigned saved = angle_expression;
        angle_expression = 0;
        ast.append(node, expression());
        in.require(")");
        angle_expression = saved;
        return postfix(node);
    }
    if (in.is("new") || in.is("delete") || (in.is("::") && (in.is("new", 1) || in.is("delete", 1))))
        return new_expression();
    if (in.is("(") && type_start(1)) {
        in.take();
        NodeId node = make(Kind::Cast);
        ast[node].op = OP_LPAREN;
        ast.append(node, type_id());
        in.require(")");
        ast.append(node, unary());
        return node;
    }
    return postfix(primary());
}

NodeId Parser::type_trait()
{
    Token keyword = in.take();
    bool size = keyword.op == KW_SIZEOF;
    NodeId result = size ? make(Kind::Sizeof) : ast.make(Kind::TypeTrait, keyword);
    if (size && in.eat("...")) {
        ast[result].kind = Kind::SizeofPack;
        in.require("(");
        ast[result].text = in.take().text;
        in.require(")");
    } else if (in.eat("(")) {
        unsigned saved = angle_expression;
        angle_expression = 0;
        ast.append(result, keyword.op != KW_NOEXCEPT && type_start() ? type_id() : expression());
        in.require(")");
        angle_expression = saved;
    } else {
        if (!size) throw std::runtime_error("type trait requires parentheses");
        ast.append(result, unary());
    }
    return result;
}

NodeId Parser::new_expression()
{
    bool global = in.eat("::");
    bool deletion = in.eat("delete");
    if (!deletion) in.require("new");
    NodeId result = make(deletion ? Kind::Delete : Kind::New);
    if (global) ast.append(result, make(Kind::Global));
    if (deletion) {
        if (in.eat("[")) {
            in.require("]");
            ast.append(result, make(Kind::ArrayDelete));
        }
        ast.append(result, unary());
        return result;
    }
    if (in.is("(") && !type_start(1)) {
        in.take();
        ast.append(result, arguments(Kind::Placement, ")"));
    }
    bool paren = in.eat("(");
    ast.append(result, type_id(!paren));
    if (paren) in.require(")");
    if (in.is("(") || in.is("{")) ast.append(result, initializer());
    return result;
}

NodeId Parser::lambda()
{
    NodeId result = make(Kind::Lambda);
    NodeId captures = make(Kind::LambdaIntroducer);
    in.require("[");
    while (!in.is("]")) {
        NodeId capture = leaf(Kind::Capture);
        if (ast[capture].op == OP_AMP && identifier()) {
            NodeId id = leaf(Kind::Identifier);
            ast[capture].detail = id;
        }
        if (in.eat("...")) ast.append(capture, make(Kind::ParameterPack));
        ast.append(captures, capture);
        if (!in.eat(",")) break;
    }
    in.require("]");
    ast.append(result, captures);
    ScopeId saved = scope;
    scope = names.enter(scope);
    if (in.is("(")) {
        NodeId decl = wrap(Kind::LambdaDeclarator, parameters());
        if (in.is("mutable")) ast.append(decl, leaf(Kind::LambdaSpecifier));
        function_suffix(decl);
        ast.append(result, decl);
    }
    ast.append(result, compound());
    scope = saved;
    return result;
}

} }
