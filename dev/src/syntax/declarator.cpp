#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

NodeId Parser::specifiers(bool type_only)
{
    NodeId result = make(type_only ? Kind::TypeSpecifiers : Kind::DeclSpecifiers);
    bool have_type = false;
    for (;;) {
        attributes();
        Kind kind = type_only ? Kind::TypeSpecifier : Kind::DeclSpecifier;
        if (builtin()) {
            ast.append(result, leaf(kind));
            have_type = true;
        } else if (in.is("const") || in.is("volatile")) ast.append(result, leaf(type_only ? Kind::CvQualifier : kind));
        else if (!type_only && (in.is("typedef") || in.is("extern") || in.is("static") ||
                 in.is("inline") || in.is("virtual") || in.is("constexpr") ||
                 in.is("thread_local") || in.is("friend") || in.is("explicit") || in.is("mutable")))
            ast.append(result, leaf(kind));
        else if (!have_type && in.is("decltype")) {
            in.take();
            in.require("(");
            unsigned saved = angle_expression;
            angle_expression = 0;
            NodeId decltype_node = wrap(type_only ? Kind::Decltype : Kind::DeclSpecifier, expression());
            ast[decltype_node].op = KW_DECLTYPE;
            ast.append(result, decltype_node);
            angle_expression = saved;
            in.require(")");
            have_type = true;
        } else if (!have_type && (in.is("class") || in.is("struct") || in.is("union"))) {
            ast.append(result, class_specifier());
            have_type = true;
        } else if (!have_type && in.is("enum")) {
            ast.append(result, enum_specifier());
            have_type = true;
        } else if (!have_type && (type_start() || in.is("typename") || (type_only && identifier()) || in.is("::", 1))) {
            bool dependent = in.eat("typename");
            NodeId n = name(dependent);
            NodeId spec = named(type_only ? Kind::TypeName : kind, n);
            ast[spec].flags = dependent ? 1 : 0;
            if (!type_only && !dependent && ast[n].first == ast[n].last && !ast[ast[n].first].first)
                ast[spec].text = final_name(n);
            ast.append(result, spec);
            have_type = true;
        } else break;
    }
    if (!have_type) throw std::runtime_error("expected type specifier");
    return result;
}

bool Parser::nested_declarator_ahead()
{
    if (!in.is("(")) return false;
    return in.is("*", 1) || in.is("&", 1) || in.is("&&", 1) || in.is("(", 1) ||
           (identifier(1) && (!type_start(1) || in.is("::", 2)));
}

bool Parser::parameter_clause_ahead()
{
    ++decisions;
    if (!in.is("(")) return false;
    std::size_t end = in.matching(0);
    for (std::size_t i = 1; i < end; ++i) {
        if (in.is("...", i)) return true;
        if (!type_start(i)) return false;
        i = probe_type(i);
        // Inspect each parameter prefix; nested suffixes and default arguments
        // cannot introduce a parameter at this delimiter level.
        while (i < end && !in.is(",", i)) {
            if (in.is("(", i) || in.is("[", i) || in.is("{", i)) i = in.matching(i);
            ++i;
        }
    }
    return true;
}

NodeId Parser::declarator(bool abstract, bool new_type)
{
    NodeId result = make(abstract ? Kind::AbstractDeclarator : Kind::Declarator);
    while (in.is("*") || in.is("&") || in.is("&&") ||
           (identifier() && in.is("::", probe_name().end) && in.is("*", probe_name().end + 1))) {
        NodeId pointer;
        if (identifier()) {
            NodeId n = name();
            in.require("::");
            in.require("*");
            pointer = named(Kind::Pointer, n);
        } else pointer = leaf(Kind::Pointer);
        ast.append(result, pointer);
        while (in.is("const") || in.is("volatile")) ast.append(result, leaf(Kind::CvQualifier));
    }
    if (in.eat("...")) ast.append(result, make(Kind::ParameterPack));
    if (nested_declarator_ahead()) {
        in.require("(");
        ast.append(result, wrap(Kind::NestedDeclarator, declarator(abstract)));
        in.require(")");
    } else if (identifier() || in.is("::") || in.is("operator") || in.is("~")) {
        ast.append(result, named(Kind::Identifier, name()));
    }
    if (in.eat("...")) ast.append(result, make(Kind::ParameterPack));
    for (;;) {
        attributes();
        if (in.eat("[")) {
            unsigned saved = angle_expression;
            angle_expression = 0;
            NodeId array = make(Kind::Array);
            if (!in.is("]")) ast.append(array, expression());
            in.require("]");
            angle_expression = saved;
            ast.append(result, array);
        } else if (!new_type && parameter_clause_ahead()) {
            ast.append(result, parameters());
            function_suffix(result);
        } else break;
    }
    return ast[result].first ? result : 0;
}

NodeId Parser::type_id(bool new_type)
{
    NodeId result = make(Kind::TypeId);
    ast.append(result, specifiers(true));
    if (in.is("*") || in.is("&") || in.is("&&") || in.is("[") ||
        (!new_type && (nested_declarator_ahead() || parameter_clause_ahead())))
        ast.append(result, declarator(true, new_type));
    return result;
}

NodeId Parser::parameter(Kind kind)
{
    NodeId result = make(kind);
    if (in.eat("...")) {
        ast.append(result, make(Kind::Ellipsis));
        return result;
    }
    ast.append(result, specifiers());
    NodeId decl = declarator();
    if (decl && !declarator_name(decl) && ast[ast[decl].first].kind == Kind::Parameters &&
        (!ast[ast[decl].first].first || ast[ast[ast[decl].first].first].kind == Kind::ParameterPack))
        ast[decl].kind = Kind::AbstractDeclarator;
    ast.append(result, decl);
    bind_declarator(decl, Category::Value, scope);
    if (in.is("=")) ast.append(result, wrap(Kind::DefaultArgument, initializer()));
    return result;
}

NodeId Parser::parameters()
{
    in.require("(");
    unsigned saved = angle_expression;
    angle_expression = 0;
    NodeId result = make(Kind::Parameters);
    if (!in.is(")")) {
        do {
            if (in.eat("...")) {
                ast.append(result, make(Kind::ParameterPack));
                break;
            }
            ast.append(result, parameter());
        } while (in.eat(","));
    }
    in.require(")");
    angle_expression = saved;
    return result;
}

void Parser::function_suffix(NodeId owner)
{
    for (;;) {
        attributes();
        if (in.is("const") || in.is("volatile")) ast.append(owner, leaf(Kind::CvQualifier));
        else if (in.is("&") || in.is("&&")) ast.append(owner, leaf(Kind::FunctionQualifier));
        else if (in.is("override") || in.is("final")) ast.append(owner, leaf(Kind::VirtSpecifier));
        else if (in.is("throw")) {
            NodeId node = leaf(Kind::FunctionQualifier);
            in.require("(");
            NodeId types = make(Kind::Parameters);
            if (!in.is(")")) {
                do { ast.append(types, type_id()); } while (in.eat(","));
            }
            in.require(")");
            ast[node].detail = types;
            ast.append(owner, node);
        } else if (in.is("noexcept")) {
            NodeId node = leaf(Kind::FunctionQualifier);
            if (in.eat("(")) {
                if (ast[owner].kind == Kind::LambdaDeclarator) ast[node].kind = Kind::Noexcept;
                ast.append(node, expression());
                in.require(")");
            }
            ast.append(owner, node);
        } else if (in.eat("->")) {
            NodeId type = type_id();
            NodeId node = wrap(Kind::TrailingReturn, type);
            NodeId spec = ast[ast[type].first].first;
            if (spec && ast[spec].kind == Kind::TypeName) ast[node].detail = ast[spec].detail;
            ast.append(owner, node);
        } else break;
    }
}

NodeId Parser::initializer()
{
    NodeId result = make(Kind::Initializer);
    if (in.eat("=")) {
        if (in.is("default") || in.is("delete")) ast.append(result, leaf(Kind::SpecialInitializer));
        else ast.append(result, in.is("{") ? primary() : expression(2));
    } else if (in.eat("(")) ast.append(result, arguments(Kind::ParenInitializer, ")"));
    else if (in.is("{")) ast.append(result, primary());
    else return 0;
    return result;
}

NodeId Parser::declarator_name(NodeId decl) const
{
    for (NodeId child = ast[decl].first; child; child = ast[child].next) {
        if (ast[child].kind == Kind::Identifier) return ast[child].detail;
        if (ast[child].kind == Kind::NestedDeclarator) return declarator_name(ast[child].first);
    }
    return 0;
}

void Parser::bind_declarator(NodeId decl, Category category, ScopeId owner)
{
    if (!decl) return;
    NodeId n = declarator_name(decl);
    if (n) names.bind(ast[n].first != ast[n].last || ast[n].op == OP_COLON2 ? qualified_owner(n) : owner,
                      final_name(n), category);
}

} }
