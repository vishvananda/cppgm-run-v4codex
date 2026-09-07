#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

NodeId Parser::class_specifier()
{
    NodeId key = leaf(Kind::ClassKey);
    attributes();
    NodeId n = identifier() ? name(true) : 0;
    NodeId result = named(Kind::Class, n);
    ast.append(result, key);
    ScopeId owner = scope;
    Binding previous = names.local(owner, final_name(n));
    ScopeId child = previous.target ? previous.target : names.enter(owner);
    names.bind(owner, final_name(n), template_declaration ? Category::TemplateType : Category::Type, child);
    in.eat("final");
    if (in.eat(":")) {
        NodeId bases = make(Kind::Bases);
        do {
            NodeId base = make(Kind::Base);
            while (in.is("virtual") || in.is("public") || in.is("private") || in.is("protected")) {
                if (in.is("virtual")) ast.append(base, leaf(Kind::Virtual));
                else ast.append(base, leaf(Kind::Access));
            }
            NodeId base_name = name(true);
            ast.append(base, named(Kind::BaseName, base_name));
            Binding binding = name_binding(base_name);
            if (binding.target) names.import(child, binding.target);
            if (in.is("...")) ast.append(base, leaf(Kind::PackExpansion));
            ast.append(bases, base);
        } while (in.eat(","));
        ast.append(result, bases);
    }
    if (!in.eat("{")) {
        ast[result].kind = Kind::ClassForward;
        return result;
    }
    IdentifierId saved_class = current_class;
    bool saved_template = template_declaration;
    template_declaration = false;
    current_class = final_name(n);
    scope = child;
    predeclare_class();
    while (!in.is("}")) {
        if (in.peek().kind == PostTokenKind::eof) throw std::runtime_error("unterminated class");
        if ((in.is("public") || in.is("private") || in.is("protected")) && in.is(":", 1)) {
            ast.append(result, leaf(Kind::Access));
            in.take();
        } else ast.append(result, declaration());
    }
    in.take();
    scope = owner;
    current_class = saved_class;
    template_declaration = saved_template;
    return result;
}

NodeId Parser::enum_specifier()
{
    in.require("enum");
    NodeId result = make(Kind::Enum);
    if (in.is("class") || in.is("struct")) ast.append(result, leaf(Kind::EnumKey));
    if (identifier()) ast[result].text = in.take().text;
    ScopeId child = names.enter(scope);
    names.bind(scope, ast[result].text, Category::Type, child);
    if (in.eat(":")) ast.append(result, type_id());
    if (!in.eat("{")) return result;
    while (!in.is("}")) {
        if (!identifier()) throw std::runtime_error("expected enumerator");
        NodeId value = leaf(Kind::Enumerator);
        names.bind(scope, ast[value].text, Category::Value);
        names.bind(child, ast[value].text, Category::Value);
        if (in.eat("=")) ast.append(value, expression(2));
        ast.append(result, value);
        if (!in.eat(",")) break;
    }
    in.require("}");
    return result;
}

NodeId Parser::special_member(NodeId specs)
{
    while (in.is("inline") || in.is("virtual") || in.is("explicit") || in.is("constexpr") ||
           in.is("friend") || in.is("static")) {
        if (!specs) specs = make(Kind::MemberSpecifiers);
        ast.append(specs, leaf(Kind::Specifier));
        attributes();
    }
    NodeId n = name();
    NodeId last = ast[n].last;
    if (ast[last].op == KW_OPERATOR && !ast[last].detail)
        throw std::runtime_error("non-conversion operator needs return type");
    NodeId result = named(Kind::SpecialMember, n);
    ast.append(result, specs);
    NodeId decl = wrap(Kind::Declarator, named(Kind::Identifier, n));
    ScopeId saved = scope;
    scope = names.enter(scope);
    ast.append(decl, parameters());
    function_suffix(decl);
    ast.append(result, decl);
    if (in.is("=")) ast.append(result, initializer());
    if (in.is(":") || in.is("{") || in.is("try")) {
        ast[result].kind = Kind::SpecialDefinition;
        if (in.is(":")) ast.append(result, ctor_initializer());
        ast.append(result, in.is("try") ? try_block(true) : compound());
    } else in.require(";");
    scope = saved;
    return result;
}

NodeId Parser::ctor_initializer()
{
    in.require(":");
    NodeId result = make(Kind::CtorInitializer);
    do {
        NodeId item = wrap(Kind::MemInitializer, named(Kind::MemInitializerId, name(true)));
        if (in.eat("(")) ast.append(item, arguments(Kind::ParenArguments, ")"));
        else if (in.is("{")) ast.append(item, primary());
        else throw std::runtime_error("expected constructor initializer arguments");
        if (in.is("...")) ast.append(item, leaf(Kind::PackExpansion));
        ast.append(result, item);
    } while (in.eat(","));
    return result;
}

} }
