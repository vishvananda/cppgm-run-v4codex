#include "syntax/parser.h"
#include "support/type_traits.h"
#include <stdexcept>

namespace cppgm { namespace syntax {
int expression_precedence(ETokenType op)
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

bool Parser::contextual_coroutine(IdentifierId name)
{
    // In the hosted C++11 extension these are contextual spellings, not
    // keywords. An existing lexical declaration keeps its ordinary meaning.
    // Declarations, qualified names and member names use their own grammar.
    if (in.peek().text != name || names.lookup(scope,name).category != Category::Unknown) return false;
    if (!retained_template_depth)
        throw std::runtime_error("coroutine syntax requires a retained template body");
    return true;
}

NodeId Parser::expression(int minimum)
{
    NodeId left;
    if (minimum <= 2 && contextual_coroutine(yield_name)) {
        left = leaf(Kind::Yield);
        ast.append(left, in.is("{") ? primary() : expression(2));
    } else if (minimum <= 2 && in.is("throw")) {
        left = leaf(Kind::Throw);
        if (!in.is(";") && !in.is(")") && !in.is(":") && !in.is(",") && !in.is("}"))
            ast.append(left, expression(2));
    } else left = unary();
    for (;;) {
        Token op = in.peek();
        int p = expression_precedence(op.op);
        if (p < minimum || (angle_expression && (op.op == OP_GT || op.op == OP_RSHIFT))) break;
        if (in.is("...",1)) break; // The enclosing primary owns a fold operator.
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
            NodeId arg;
            if (kind == Kind::BracedInit && in.eat(".")) {
                if (!identifier()) throw std::runtime_error("expected designated member name");
                arg = leaf(Kind::DesignatedInit);
                if (in.eat("=")) ast.append(arg,expression(2));
                else if (in.is("{")) ast.append(arg,primary());
                else throw std::runtime_error("expected designated member initializer");
            } else arg = expression(2);
            if (in.eat("...")) arg = wrap(Kind::PackExpression, arg);
            ast.append(result, arg);
        } while (in.eat(","));
    }
    in.require(close);
    angle_expression = saved;
    return result;
}

ScopeId Parser::expression_scope(NodeId node)
{
    // The receiver subtree is complete before postfix lookup. Cache even an
    // unknown result so a chain of calls/members visits each node once.
    if (node >= expression_scopes.size()) expression_scopes.resize(node+1);
    if (expression_scopes[node]) return expression_scopes[node]-1;
    auto n = ast[node];
    ScopeId result = 0;
    if (n.kind == Kind::IdExpression) result = name_binding(n.detail).target;
    else if (n.kind == Kind::Parenthesized || n.kind == Kind::Call || n.kind == Kind::Unary || n.kind == Kind::Subscript)
        result = expression_scope(n.first);
    else if (n.kind == Kind::Member) {
        auto owner = expression_scope(n.first);
        auto name = ast[ast[n.first].next].detail;
        if (owner) result = names.qualified(owner,final_name(name)).target;
    }
    expression_scopes[node] = result+1;
    return result;
}

NodeId Parser::postfix(NodeId base)
{
    for (;;) {
        NodeId result;
        if (in.eat("(")) {
            result = wrap(Kind::Call, base);
            ast.append(result, arguments(ast[base].kind == Kind::IdExpression && ast[base].op != TOK_INVALID && ast[base].op != KW_TYPENAME ?
                                          Kind::ParenArguments : Kind::Arguments, ")"));
        } else if (in.is("{") && ast[base].kind == Kind::IdExpression) {
            result = wrap(Kind::Call, base);
            ast.append(result, primary());
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
            bool saved_member = member_name;
            member_name = true;
            auto receiver = expression_scope(base);
            bool known_template = receiver && template_category(names.qualified(receiver,in.peek().text).category);
            NodeId member = name(known_template);
            member_name = saved_member;
            ast.append(result, named(Kind::Identifier, member));
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
    if (in.is("__null")) {
        // GNU's null constant has the target pointer-sized integer type.
        // Retain its source spelling/location with an ordinary zero literal.
        auto result = leaf(Kind::Literal);
        LiteralValue value{}; value.kind = LiteralKind::integer; value.type = FT_LONG_INT;
        ast[result].literal = ast.literals.size(); ast.literals.push_back(value);
        return result;
    }
    if (in.is("__func__") || in.is("__FUNCTION__") || in.is("__PRETTY_FUNCTION__")) return leaf(Kind::FunctionName);
    if (in.is("__builtin_addressof")) {
        auto result = leaf(Kind::Unary); ast[result].op = OP_AMP; ast[result].flags = 1;
        in.require("("); ast.append(result,expression(2)); in.require(")"); return result;
    }
    if (in.is("__builtin_va_arg")) {
        auto result = leaf(Kind::VaArg); in.require("(");
        ast.append(result,expression(2)); in.require(",");
        ast.append(result,type_id()); in.require(")"); return result;
    }
    if (in.is("::") && builtin_trait(ids.spelling(in.peek(1).text)) == BuiltinTrait::Offsetof && in.is("(",2)) {
        in.take(); return type_trait();
    }
    auto trait = builtin_trait(ids.spelling(in.peek(in.is("::") ? 1 : 0).text));
    if (template_type_transform(trait) && in.is("<",in.is("::") ? 2 : 1)) {
        auto result = make(Kind::IdExpression);
        ast[result].detail = wrap(Kind::TypeId,specifiers(true));
        return result;
    }
    if (builtin_trait(ids.spelling(token.text)) != BuiltinTrait::None &&
        !template_type_transform(trait) && in.is("(",1)) return type_trait();
    if (token.kind == PostTokenKind::literal || token.kind == PostTokenKind::user_literal)
        return leaf(Kind::Literal);
    if (in.is("true") || in.is("false") || in.is("nullptr") || in.is("this"))
        return leaf(Kind::KeywordLiteral);
    if (in.eat("(")) {
        unsigned saved = angle_expression;
        angle_expression = 0;
        NodeId result;
        if (in.eat("...")) {
            auto op = in.take();
            if (!expression_precedence(op.op) || op.op == OP_QMARK)
                throw std::runtime_error("expected fold operator");
            result = ast.make(Kind::Fold,op); ast[result].flags = 1;
            ast.append(result,unary());
        } else if (in.is("{")) result = wrap(Kind::StatementExpression,compound());
        else {
            auto operand = expression();
            if (expression_precedence(in.peek().op) && in.is("...",1)) {
                auto kind = ast[operand].kind;
                if (kind == Kind::Binary || kind == Kind::Assignment || kind == Kind::Conditional || kind == Kind::Throw)
                    throw std::runtime_error("fold operand must be a cast-expression");
                auto op = in.take(); in.require("...");
                if (op.op == OP_QMARK) throw std::runtime_error("invalid fold operator");
                result = ast.make(Kind::Fold,op); ast.append(result,operand);
                if (!in.is(")")) {
                    if (in.take().op != op.op) throw std::runtime_error("fold operators must match");
                    ast.append(result,unary());
                }
            } else result = wrap(Kind::Parenthesized,operand);
        }
        in.require(")");
        angle_expression = saved;
        return result;
    }
    if (in.eat("{")) return arguments(Kind::BracedInit, "}");
    if (in.is("[")) return lambda();
    if (in.eat("typename")) {
        auto result = named(Kind::IdExpression,name(true));
        ast[result].op = KW_TYPENAME; return result;
    }
    if (in.is("_BitInt") || builtin()) {
        if (in.is("_BitInt") || in.is("_BitInt",1) || builtin(1)) {
            NodeId result = make(Kind::IdExpression);
            ast[result].detail = wrap(Kind::TypeId, specifiers(true));
            return result;
        }
        return leaf(Kind::IdExpression);
    }
    if (identifier() || in.is("::") || in.is("operator") || in.is("decltype"))
        return named(Kind::IdExpression, name());
    TextView text = ids.spelling(token.text);
    throw std::runtime_error("expected expression, found '" + std::string(text.data, text.size) + "'");
}

NodeId Parser::unary()
{
    if (contextual_coroutine(await_name)) {
        NodeId node = leaf(Kind::Await);
        ast.append(node,unary());
        return node;
    }
    if (in.is("++") || in.is("--") || in.is("*") || in.is("&") ||
        in.is("+") || in.is("-") || in.is("!") || in.is("~") || in.peek().op == KW_REAL || in.peek().op == KW_IMAG) {
        NodeId node = leaf(Kind::Unary);
        ast.append(node, unary());
        return node;
    }
    if (in.is("sizeof") || in.is("typeid") || in.is("alignof") || in.is("noexcept") ||
        in.is("__alignof") || in.is("__alignof__"))
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
    auto cast_type_end = in.is("(") && type_start(1) ? probe_type(1) : 0;
    // A brace after the type starts a functional construction inside the
    // parentheses; it cannot continue the type-id of a C-style cast.
    if (cast_type_end && !in.is("{",cast_type_end) && (!in.is("(", cast_type_end) ||
        in.is("*",cast_type_end+1) || in.is("^",cast_type_end+1) || in.is("&",cast_type_end+1) || in.is("&&",cast_type_end+1))) {
        in.take();
        NodeId node = make(Kind::Cast);
        ast[node].op = OP_LPAREN;
        ast.append(node, type_id());
        in.require(")");
        if (in.is("{")) {
            // GNU C++ compound literals are list-initialized temporaries.
            // Keep the actual type and list on the cast node, including its
            // postfix suffixes; no synthetic functional-cast name is needed.
            ast.append(node,primary());
            return postfix(node);
        }
        ast.append(node, unary());
        return node;
    }
    return postfix(primary());
}

NodeId Parser::type_trait()
{
    bool gnu_alignment = in.is("__alignof") || in.is("__alignof__");
    Token keyword = in.take();
    if (gnu_alignment) keyword.op = KW_ALIGNOF;
    bool size = keyword.op == KW_SIZEOF;
    NodeId result = size ? make(Kind::Sizeof) : ast.make(Kind::TypeTrait, keyword);
    auto trait = builtin_trait(ids.spelling(keyword.text));
    if (trait != BuiltinTrait::None) {
        ast[result].flags = unsigned(trait);
        if (template_type_transform(trait)) {
            auto arguments = template_arguments();
            while (ast[arguments].first) ast.append(result,ast.take_first(arguments));
            return result;
        }
        in.require("("); unsigned saved = angle_expression; angle_expression = 0;
        if (trait == BuiltinTrait::Offsetof) {
            ast.append(result,type_id()); in.require(",");
            if (!identifier()) throw std::runtime_error("offsetof requires a member designator");
            ast.append(result,leaf(Kind::Identifier));
            for (;;) {
                if (in.eat(".")) {
                    if (!identifier()) throw std::runtime_error("offsetof requires a member name");
                    ast.append(result,leaf(Kind::Identifier));
                } else if (in.eat("[")) {
                    ast.append(result,wrap(Kind::Subscript,expression())); in.require("]");
                } else break;
            }
        } else if (!in.is(")")) do {
            auto operand = type_id();
            if (in.eat("...")) operand = wrap(Kind::PackExpression,operand);
            ast.append(result,operand);
        } while (in.eat(","));
        in.require(")"); angle_expression = saved;
    } else if (size && in.eat("...")) {
        ast[result].kind = Kind::SizeofPack;
        in.require("(");
        ast[result].text = in.take().text;
        in.require(")");
    } else if (in.eat("(")) {
        unsigned saved = angle_expression;
        angle_expression = 0;
        ast.append(result, keyword.op != KW_NOEXCEPT && type_operand(keyword.op == KW_TYPEID) ? type_id() : expression());
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
    bool placement = in.is("(") && !type_start(1);
    if (in.is("(") && !placement) {
        auto type_end = probe_type(1);
        if (in.is("(",type_end) || in.is("{",type_end)) {
            auto after = in.matching(0)+1;
            placement = type_start(after) || (in.is("(",after) && type_start(after+1));
        }
    }
    if (placement) {
        in.take();
        ast.append(result, wrap(Kind::Placement, arguments(Kind::ParenArguments, ")")));
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
    bool templated = in.is("<");
    if (templated) { ++retained_template_depth; ast.append(result, template_parameters()); }
    if (in.is("(")) {
        ScopeId parameter_scope;
        NodeId decl = wrap(Kind::LambdaDeclarator, parameters(parameter_scope));
        scope = parameter_scope;
        if (in.is("mutable")) ast.append(decl, leaf(Kind::LambdaSpecifier));
        function_suffix(decl);
        ast.append(result, decl);
    }
    ast.append(result, compound());
    if (templated) --retained_template_depth;
    scope = saved;
    return result;
}

} }
