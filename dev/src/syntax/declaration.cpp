#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

NodeId Parser::declaration()
{
    attributes();
    if (in.eat(";")) return make(Kind::EmptyDeclaration);
    if (in.is("namespace") || (in.is("inline") && in.is("namespace", 1))) return namespace_declaration();
    if (in.is("using")) return using_declaration();
    if (in.is("template")) return template_decl();
    if (in.is("extern") && in.is("template", 1)) {
        in.take();
        in.take();
        return wrap(Kind::ExplicitInstantiation, declaration());
    }
    if (in.is("extern") && in.peek(1).kind == PostTokenKind::literal) {
        in.take();
        NodeId node = leaf(Kind::Linkage);
        if (in.eat("{")) {
            while (!in.is("}")) ast.append(node, declaration());
            in.take();
        } else ast.append(node, declaration());
        return node;
    }
    if (in.is("static_assert")) return static_assertion();
    if (in.is("class") || in.is("struct") || in.is("union")) {
        NodeId node = class_specifier();
        in.require(";");
        return node;
    }
    if (in.is("enum")) {
        NodeId node = enum_specifier();
        in.require(";");
        return node;
    }
    if (in.is("~") || in.is("operator") ||
        (current_class && in.peek().text == current_class && in.is("(", 1))) return special_member();
    return simple_declaration();
}

NodeId Parser::simple_declaration(bool require_semicolon)
{
    NodeId specs = specifiers();
    ScopeId owner = scope;
    ScopeId parameters_scope = names.enter(scope);
    scope = parameters_scope;
    NodeId decl = declarator();
    bool alias = false;
    for (NodeId s = ast[specs].first; s; s = ast[s].next) alias |= ast[s].op == KW_TYPEDEF;
    Category category = alias ? Category::Type : Category::Value;
    if (template_declaration && !alias) category = Category::TemplateValue;
    bind_declarator(decl, category, owner);
    if (decl && (in.is("{") || in.is("try")) && ast[ast[decl].last].kind != Kind::Identifier) {
        NodeId result = wrap(Kind::Function, specs);
        ast.append(result, decl);
        ast.append(result, in.is("try") ? try_block(true) : compound());
        scope = owner;
        return result;
    }
    scope = owner;
    NodeId result = wrap(Kind::SimpleDeclaration, specs);
    if (in.is(":")) {
        ast[result].kind = Kind::BitField;
        do {
            NodeId field = wrap(Kind::BitFieldDeclarator, decl);
            in.require(":");
            ast.append(field, expression(2));
            ast.append(result, field);
            if (!in.eat(",")) break;
            decl = declarator();
        } while (true);
    } else if (decl) {
        NodeId list = make(Kind::InitDeclarators);
        do {
            NodeId item = wrap(Kind::InitDeclarator, decl);
            ast.append(item, initializer());
            ast.append(list, item);
            if (!in.eat(",")) break;
            decl = declarator();
            if (!decl) throw std::runtime_error("expected declarator after comma");
            bind_declarator(decl, category, owner);
        } while (true);
        ast.append(result, list);
    }
    if (require_semicolon) in.require(";");
    return result;
}

NodeId Parser::namespace_declaration()
{
    bool is_inline = in.eat("inline");
    in.require("namespace");
    Token token;
    if (identifier()) token = in.take();
    NodeId result = ast.make(Kind::Namespace, token);
    if (in.eat("=")) {
        ast[result].kind = Kind::NamespaceAlias;
        NodeId n = name();
        ast.append(result, named(Kind::Target, n));
        Binding binding = name_binding(n);
        names.bind(scope, token.text, Category::Namespace, binding.target);
        in.require(";");
        return result;
    }
    if (is_inline) ast.append(result, make(Kind::Inline));
    Binding previous = names.local(scope, token.text);
    ScopeId child = previous.target ? previous.target : names.enter(scope);
    names.bind(scope, token.text, Category::Namespace, child);
    ScopeId saved = scope;
    scope = child;
    in.require("{");
    while (!in.is("}")) ast.append(result, declaration());
    in.take();
    scope = saved;
    if (is_inline || !token.text) names.import(scope, child);
    return result;
}

NodeId Parser::using_declaration()
{
    in.require("using");
    if (identifier() && in.is("=", 1)) {
        NodeId result = leaf(Kind::Alias);
        in.require("=");
        ast.append(result, type_id());
        names.bind(scope, ast[result].text, template_declaration ? Category::TemplateType : Category::Type);
        in.require(";");
        return result;
    }
    bool directive = in.eat("namespace");
    in.eat("typename");
    NodeId n = name();
    NodeId result = wrap(directive ? Kind::UsingDirective : Kind::UsingDeclaration, named(Kind::Target, n));
    Binding binding = name_binding(n);
    if (directive && binding.target) names.import(scope, binding.target);
    else if (!directive) names.bind(scope, final_name(n), binding.category, binding.target);
    in.require(";");
    return result;
}

NodeId Parser::static_assertion()
{
    in.require("static_assert");
    in.require("(");
    NodeId result = wrap(Kind::StaticAssert, expression(2));
    if (in.eat(",")) ast.append(result, leaf(Kind::Message));
    in.require(")");
    in.require(";");
    return result;
}

} }
