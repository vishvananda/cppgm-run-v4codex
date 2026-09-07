#include "syntax/parser.h"

namespace cppgm { namespace syntax {

NodeId Parser::template_decl()
{
    in.require("template");
    ScopeId saved = scope;
    bool saved_template = template_declaration;
    scope = names.enter(scope);
    NodeId params = template_parameters();
    template_declaration = true;
    NodeId result = wrap(Kind::Template, params);
    NodeId declaration_node = declaration();
    ast.append(result, declaration_node);
    NodeId n = ast[declaration_node].detail;
    if (ast[declaration_node].kind == Kind::SimpleDeclaration) {
        NodeId list = ast[ast[declaration_node].first].next;
        n = declarator_name(ast[ast[list].first].first);
    }
    if (ast[declaration_node].kind == Kind::Function)
        n = declarator_name(ast[ast[declaration_node].first].next);
    IdentifierId id = n ? final_name(n) : ast[declaration_node].text;
    Binding binding = names.local(scope, id);
    if (binding.category != Category::Unknown) names.bind(saved, id, binding.category, binding.target);
    scope = saved;
    template_declaration = saved_template;
    return result;
}

NodeId Parser::template_parameters()
{
    in.require("<");
    NodeId result = make(Kind::TemplateParameters);
    if (in.is(">") || in.is(">>")) {
        in.close_angle();
        return result;
    }
    NodeId list = make(Kind::TemplateParameterList);
    do {
        NodeId param = make(Kind::TypeParameter);
        bool template_parameter = in.eat("template");
        if (template_parameter) {
            ast.append(param, make(Kind::TemplateTemplate));
            ScopeId saved = scope;
            scope = names.enter(scope);
            ast.append(param, template_parameters());
            scope = saved;
        }
        if (in.is("class") || in.is("typename")) {
            ast.append(param, leaf(Kind::ParameterKey));
            if (in.eat("...")) ast.append(param, make(Kind::ParameterPack));
            if (identifier()) {
                NodeId id = leaf(Kind::Identifier);
                ast.append(param, id);
                names.bind(scope, ast[id].text, template_parameter ? Category::TemplateType : Category::Type);
            }
            if (in.eat("=")) ast.append(param, wrap(Kind::DefaultTemplateArgument, type_id()));
        } else {
            ast[param].kind = Kind::NonTypeParameter;
            ast.append(param, specifiers());
            NodeId decl = declarator();
            ast.append(param, decl);
            bind_declarator(decl, Category::Value, scope);
            if (in.eat("=")) {
                unsigned saved = angle_expression;
                angle_expression = 1;
                ast.append(param, wrap(Kind::DefaultTemplateArgument, expression(2)));
                angle_expression = saved;
            }
        }
        ast.append(list, param);
    } while (in.eat(","));
    in.close_angle();
    ast.append(result, list);
    return result;
}

} }
