#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

namespace {
void condition_specifiers(const Ast& ast, NodeId specs)
{
    for (auto s = ast[specs].first; s; s = ast[s].next) {
        switch (ast[s].op) {
        case KW_TYPEDEF: case KW_EXTERN: case KW_STATIC: case KW_THREAD_LOCAL:
        case KW_REGISTER: case KW_MUTABLE: case KW_INLINE: case KW_FRIEND:
        case KW_VIRTUAL: case KW_EXPLICIT:
            throw std::runtime_error("invalid condition declaration specifier");
        default: break;
        }
        if (ast[s].kind == Kind::Class || (ast[s].kind == Kind::Enum && (ast[s].flags & 1)))
            throw std::runtime_error("class or enum definition in condition");
    }
}
}

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
        condition_specifiers(ast,ast[decl].first);
        NodeId d = declarator();
        ast.append(decl, d);
        bind_declarator(d, Category::Value, scope);
        NodeId init = initializer();
        if (!init) throw std::runtime_error("condition declaration requires initializer");
        if (ast[ast[init].first].kind == Kind::ParenInitializer)
            throw std::runtime_error("condition requires brace or equal initializer");
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
    if (is_if && in.eat("constexpr")) ast[result].flags |= 1;
    in.require("(");
    // Parse the shared declaration/expression prefix once. The following
    // delimiter decides whether it initializes the selection or is its test.
    NodeId initial = 0;
    bool initialized = false;
    if (in.eat(";")) initialized = true;
    else if (in.is("using")) {
        initial = using_declaration();
        if (ast[initial].kind != Kind::Alias) throw std::runtime_error("selection initializer requires an alias declaration");
        initialized = true;
    } else if (declaration_start() && declaration_ahead()) {
        initial = simple_declaration(false);
        initialized = in.eat(";");
        if (!initialized) {
            auto specs = ast[initial].first, list = ast[specs].next;
            auto item = ast[list].first, d = ast[item].first, init = ast[d].next;
            if (ast[initial].kind != Kind::SimpleDeclaration || !item || ast[item].next || !init ||
                ast[ast[init].first].kind == Kind::ParenInitializer)
                throw std::runtime_error("invalid condition declaration");
            ast[initial].kind = Kind::ConditionDeclaration;
            condition_specifiers(ast,specs);
            ast[specs].next = d; ast[initial].last = init;
        }
    } else {
        initial = expression();
        initialized = in.eat(";");
        if (initialized) initial = wrap(Kind::ExpressionStatement,initial);
    }
    if (initialized) {
        ast.append(result,wrap(Kind::SelectionInit,initial));
        ast.append(result,condition());
    } else ast.append(result,wrap(Kind::Condition,initial));
    in.require(")");
    ast.append(result, is_if ? wrap(Kind::Then, substatement()) : substatement());
    if (is_if && in.eat("else")) ast.append(result, wrap(Kind::Else, substatement()));
    scope = saved;
    return result;
}

NodeId Parser::iteration()
{
    if (in.is("for")) return for_statement();
    ScopeId saved = scope;
    scope = names.enter(scope);
    bool is_do = in.eat("do");
    NodeId result = make(is_do ? Kind::Do : Kind::While);
    if (is_do) ast.append(result, substatement());
    in.require("while");
    in.require("(");
    ast.append(result, condition());
    in.require(")");
    if (is_do) in.require(";");
    else ast.append(result, substatement());
    scope = saved;
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
    if (declaration_start() && declaration_ahead()) ast.append(init, simple_declaration(false));
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
        NodeId test = in.is(";") ? make(Kind::Condition) : condition();
        in.require(";");
        ast.append(result, test);
        NodeId step = make(Kind::Iteration);
        if (!in.is(")")) ast.append(step, expression());
        ast.append(result, step);
    }
    in.require(")");
    ast.append(result, substatement());
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
    std::uint32_t alignment = 0;
    NativeAttributes native;
    unsigned flags = attributes(&alignment,&native);
    if (alignment || native.section || native.weak) {
        auto result = declaration();
        native_attributes(result,native);
        ast.alignment_owners.put(result,alignment); ast[result].flags |= flags;
        return result;
    }
    if (in.is("{")) return compound();
    if (in.is("asm") || in.is("__asm") || in.is("__asm__")) return assembly();
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
    if (in.is("throw")) {
        auto value = expression();
        in.require(";");
        if (ast[value].kind != Kind::Throw) return wrap(Kind::ExpressionStatement,value);
        // Keep the established standalone-statement view. A comma belongs to
        // the surrounding expression, not to the throw's assignment operand.
        ast[value].text = 0; ast[value].op = TOK_INVALID;
        return value;
    }
    Kind jump;
    if (in.is("return")) jump = Kind::Return;
    else if (contextual_coroutine("co_return")) jump = Kind::CoroutineReturn;
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
    auto token = in.take();
    NodeId node = jump == Kind::CoroutineReturn ? ast.make(jump,token) : make(jump);
    if (jump == Kind::Goto) ast[node].text = in.take().text;
    else if ((jump == Kind::Return || jump == Kind::CoroutineReturn) && !in.is(";"))
        ast.append(node, expression());
    in.require(";");
    return node;
}

NodeId Parser::substatement()
{
    // Even an unbraced controlled statement has its own block scope.
    if (in.is("{")) return compound();
    ScopeId saved = scope;
    scope = names.enter(scope);
    NodeId result = statement();
    scope = saved;
    return result;
}

bool Parser::declaration_ahead()
{
    if (ast.telemetry) ++decisions;
    std::size_t prefix = probe_type(0);
    if (in.is("{", prefix)) return false;
    if (!in.is("(", prefix)) return true;
    // A parenthesized declarator needs a declarator-id or pointer/nesting
    // prefix. Empty parentheses and expression-only operands construct a
    // value; they cannot declare an unnamed function in a statement.
    if (!identifier(prefix+1) && !in.is("*",prefix+1) && !in.is("&",prefix+1) &&
        !in.is("&&",prefix+1) && !in.is("(",prefix+1) && !in.is("::",prefix+1)) return false;
    // Pointer operators are shared with unary expressions. Their presence
    // alone does not establish a declarator: T(*this)() constructs and calls
    // an object. Inspect the terminal prefix without replaying either grammar.
    auto terminal = prefix+1;
    while (in.is("(",terminal) || in.is("*",terminal) || in.is("&",terminal) ||
        in.is("&&",terminal) || in.is("const",terminal) || in.is("volatile",terminal)) ++terminal;
    if (!identifier(terminal) && !in.is("::",terminal)) return false;
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
