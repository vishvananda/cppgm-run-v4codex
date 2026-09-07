#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

NodeId Parser::compound()
{
    in.require("{");
    ScopeId saved = scope;
    scope = names.enter(scope);
    NodeId result = make(Kind::Compound);
    while (!in.is("}")) {
        if (in.peek().kind == PostTokenKind::eof) throw std::runtime_error("unterminated function body");
        ast.append(result, statement());
    }
    in.take();
    scope = saved;
    return result;
}

NodeId Parser::condition()
{
    NodeId result = make(Kind::Condition);
    if (declaration_start() && !in.is("(", probe_type(0)) &&
        (identifier(probe_type(0)) || in.is("*", probe_type(0)) || in.is("&", probe_type(0)))) {
        NodeId decl = make(Kind::ConditionDeclaration);
        ast.append(decl, specifiers());
        NodeId d = declarator();
        ast.append(decl, d);
        bind_declarator(d, Category::Value, scope);
        NodeId init = initializer();
        if (!init) throw std::runtime_error("condition declaration requires initializer");
        ast.append(decl, init);
        ast.append(result, decl);
    } else ast.append(result, expression());
    return result;
}

NodeId Parser::selection()
{
    bool is_if = in.eat("if");
    if (!is_if) in.require("switch");
    ScopeId saved = scope;
    scope = names.enter(scope);
    NodeId result = make(is_if ? Kind::If : Kind::Switch);
    in.require("(");
    ast.append(result, condition());
    in.require(")");
    ast.append(result, is_if ? wrap(Kind::Then, statement()) : statement());
    if (is_if && in.eat("else")) ast.append(result, wrap(Kind::Else, statement()));
    scope = saved;
    return result;
}

NodeId Parser::iteration()
{
    if (in.is("for")) return for_statement();
    bool is_do = in.eat("do");
    NodeId result = make(is_do ? Kind::Do : Kind::While);
    if (is_do) ast.append(result, statement());
    in.require("while");
    in.require("(");
    ast.append(result, condition());
    in.require(")");
    if (is_do) in.require(";");
    else ast.append(result, statement());
    return result;
}

NodeId Parser::for_statement()
{
    in.require("for");
    in.require("(");
    ScopeId saved = scope;
    scope = names.enter(scope);
    NodeId result = make(Kind::For);
    NodeId init = make(Kind::ForInit);
    if (declaration_start()) ast.append(init, simple_declaration(false));
    else if (!in.is(";")) ast.append(init, expression());
    if (in.eat(":")) {
        ast[result].kind = Kind::RangeFor;
        ast[init].kind = Kind::RangeDeclaration;
        NodeId decl = ast[init].first;
        NodeId specs = ast[decl].first;
        NodeId list = ast[specs].next;
        NodeId declarator = ast[ast[list].first].first;
        ast[init].first = specs;
        ast[specs].next = declarator;
        ast[init].last = declarator;
        ast.append(result, init);
        ast.append(result, wrap(Kind::RangeInitializer, expression()));
    } else {
        in.require(";");
        ast.append(result, init);
        NodeId test = make(Kind::Condition);
        if (!in.is(";")) ast.append(test, expression());
        in.require(";");
        ast.append(result, test);
        NodeId step = make(Kind::Iteration);
        if (!in.is(")")) ast.append(step, expression());
        ast.append(result, step);
    }
    in.require(")");
    ast.append(result, statement());
    scope = saved;
    return result;
}

NodeId Parser::handler()
{
    in.require("catch");
    in.require("(");
    ScopeId saved = scope;
    scope = names.enter(scope);
    NodeId result = wrap(Kind::Handler, parameter(Kind::ExceptionDeclaration));
    in.require(")");
    ast.append(result, compound());
    scope = saved;
    return result;
}

NodeId Parser::try_block(bool function)
{
    in.require("try");
    NodeId result = make(function ? Kind::FunctionTry : Kind::Try);
    if (function && in.is(":")) ast.append(result, ctor_initializer());
    ast.append(result, compound());
    if (!in.is("catch")) throw std::runtime_error("try requires handler");
    while (in.is("catch")) ast.append(result, handler());
    return result;
}

NodeId Parser::statement()
{
    attributes();
    if (in.is("{")) return compound();
    if (in.is("if") || in.is("switch")) return selection();
    if (in.is("while") || in.is("do") || in.is("for")) return iteration();
    if (in.is("try")) return try_block();
    if (in.eat("case")) {
        NodeId node = wrap(Kind::Case, expression());
        in.require(":");
        ast.append(node, statement());
        return node;
    }
    if (in.eat("default")) {
        in.require(":");
        return wrap(Kind::Default, statement());
    }
    if (identifier() && in.is(":", 1)) {
        NodeId node = leaf(Kind::Label);
        in.take();
        ast.append(node, statement());
        return node;
    }
    Kind jump;
    if (in.is("return")) jump = Kind::Return;
    else if (in.is("throw")) jump = Kind::Throw;
    else if (in.is("break")) jump = Kind::Break;
    else if (in.is("continue")) jump = Kind::Continue;
    else if (in.is("goto")) jump = Kind::Goto;
    else {
        if (declaration_start() && declaration_ahead()) return declaration();
        NodeId node = make(Kind::ExpressionStatement);
        if (!in.is(";")) ast.append(node, expression());
        in.require(";");
        return node;
    }
    in.take();
    NodeId node = make(jump);
    if (jump == Kind::Goto) ast[node].text = in.take().text;
    else if ((jump == Kind::Return || jump == Kind::Throw) && !in.is(";"))
        ast.append(node, expression(jump == Kind::Throw ? 2 : 1));
    in.require(";");
    return node;
}

bool Parser::declaration_ahead()
{
    if (ast.telemetry) ++decisions;
    if (in.is("typename")) return false;
    std::size_t prefix = probe_type(0);
    if (in.is("{", prefix)) return false;
    if (!in.is("(", prefix)) return true;
    // Only this shared type/parenthesis prefix requires declaration preference.
    // Scan its balanced suffix without constructing or abandoning any AST.
    unsigned depth = 0;
    std::size_t i = prefix;
    for (;; ++i) {
        if (in.peek(i).kind == PostTokenKind::eof) return false;
        if (in.is("(", i)) ++depth;
        if (in.is(")", i) && !--depth) break;
    }
    return in.is(";", i + 1) || in.is("[", i + 1) || in.is("(", i + 1) ||
           in.is("=", i + 1) || in.is(",", i + 1);
}

} }
