#include "syntax/parser.h"
#include <stdexcept>

namespace cppgm { namespace syntax {

IdentifierId Parser::final_name(NodeId name) const
{
    if (!name) return 0;
    if (ast[name].kind == Kind::Name) return final_name(ast[name].last);
    return ast[name].text;
}

Binding Parser::name_binding(NodeId name)
{
    ScopeId owner = scope;
    bool qualified = ast[name].op == OP_COLON2;
    if (qualified) owner = 0;
    Binding result;
    for (NodeId p = ast[name].first; p; p = ast[p].next) {
        result = qualified ? names.local(owner, ast[p].text) : names.lookup(owner, ast[p].text);
        if (ast[p].next) {
            if (!result.target) return Binding();
            owner = result.target;
            qualified = true;
        }
    }
    return result;
}

NodeId Parser::operator_name()
{
    Token token = in.require("operator");
    NodeId part = ast.make(Kind::NamePart, token);
    if (in.is("new") || in.is("delete")) {
        ast.append(part, leaf(Kind::Identifier));
        if (in.eat("[")) {
            in.require("]");
            ast.append(part, make(Kind::Array));
        }
    } else if (in.is("(") || in.is("[")) {
        bool call = in.eat("(");
        if (!call) in.require("[");
        in.require(call ? ")" : "]");
        ast.append(part, make(call ? Kind::Parameters : Kind::Array));
    } else if (in.peek().kind == PostTokenKind::literal) {
        ast.append(part, leaf(Kind::Literal));
        if (!identifier()) throw std::runtime_error("literal operator needs suffix");
        ast.append(part, leaf(Kind::Identifier));
    } else if (type_start()) {
        NodeId type = type_id(true);
        ast[part].detail = type;
    } else if (in.peek().op >= OP_LBRACE) {
        ast.append(part, leaf(Kind::Identifier));
    } else throw std::runtime_error("invalid operator name");
    return part;
}

NodeId Parser::name_part(bool force_template, ScopeId owner, bool qualified)
{
    force_template = in.eat("template") || force_template;
    NodeId part;
    if (in.is("operator")) part = operator_name();
    else if (in.eat("~")) {
        part = make(Kind::NamePart);
        ast[part].op = OP_COMPL;
        ast.append(part, in.is("decltype") ? type_id() : leaf(Kind::Identifier));
    } else if (in.is("decltype")) {
        part = make(Kind::NamePart);
        in.take();
        in.require("(");
        unsigned saved = angle_expression;
        angle_expression = 0;
        NodeId type = wrap(Kind::Decltype, expression());
        ast[part].detail = type;
        angle_expression = saved;
        in.require(")");
    } else {
        if (!identifier()) throw std::runtime_error("expected identifier");
        part = leaf(Kind::NamePart);
    }
    Binding binding = qualified ? names.local(owner, ast[part].text) : names.lookup(owner, ast[part].text);
    bool potential = template_category(binding.category) || force_template || ast[part].op == KW_OPERATOR;
    if (binding.category == Category::Unknown) {
        TextView spelling = ids.spelling(ast[part].text);
        for (std::size_t i = 0; i < spelling.size; ++i) potential |= spelling.data[i] == 'T';
        // An unresolved name with an explicit builtin type argument is unambiguous.
        potential |= builtin(1) || in.is("typename", 1);
    }
    if (in.is("<") && potential) ast.append(part, template_arguments());
    return part;
}

NodeId Parser::name(bool force_template)
{
    NodeId result = make(Kind::Name);
    ScopeId owner = scope;
    bool qualified = in.eat("::");
    if (qualified) {
        ast[result].op = OP_COLON2;
        owner = 0;
    }
    for (;;) {
        NodeId part = name_part(force_template, owner, qualified);
        ast.append(result, part);
        if (!in.is("::") || in.is("*", 1)) break;
        in.take();
        Binding b = qualified ? names.local(owner, ast[part].text) : names.lookup(owner, ast[part].text);
        owner = b.target;
        qualified = true;
        force_template = force_template || template_category(b.category);
    }
    return result;
}

NodeId Parser::template_arguments()
{
    in.require("<");
    NodeId args = make(Kind::TemplateArguments);
    unsigned saved = angle_expression;
    angle_expression = 1;
    if (!in.is(">") && !in.is(">>")) {
        do {
            NodeId arg = type_start() ? type_id() : expression(2);
            if (in.eat("...")) arg = wrap(Kind::PackExpression, arg);
            ast.append(args, arg);
        } while (in.eat(","));
    }
    in.close_angle();
    angle_expression = saved;
    return args;
}

} }
