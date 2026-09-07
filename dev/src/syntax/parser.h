#pragma once
#include "syntax/cursor.h"
#include "syntax/names.h"

namespace cppgm { namespace syntax {

class Parser {
public:
    Parser(Cursor& cursor, Ast& ast, IdentifierTable& ids);
    NodeId translation_unit();
    std::size_t decisions = 0;
private:
    Cursor& in;
    Ast& ast;
    IdentifierTable& ids;
    Names names;
    ScopeId scope = 0;
    IdentifierId current_class = 0;
    bool template_declaration = false;
    unsigned angle_expression = 0;

    NodeId make(Kind kind);
    NodeId leaf(Kind kind);
    NodeId wrap(Kind kind, NodeId child);
    NodeId named(Kind kind, NodeId name);
    bool identifier(std::size_t ahead = 0);
    bool builtin(std::size_t ahead = 0);
    bool type_start(std::size_t ahead = 0);
    bool declaration_start();
    Binding name_binding(NodeId name);
    IdentifierId final_name(NodeId name) const;
    NodeId name(bool force_template = false);
    NodeId name_part(bool force_template, ScopeId owner, bool qualified);
    NodeId template_arguments();
    NodeId operator_name();
    void attributes();
    void balanced(const char* open, const char* close);

    NodeId declaration();
    NodeId namespace_declaration();
    NodeId using_declaration();
    NodeId template_decl();
    NodeId template_parameters();
    NodeId class_specifier();
    NodeId enum_specifier();
    NodeId static_assertion();
    NodeId simple_declaration(bool require_semicolon = true);
    NodeId special_member(NodeId specs = 0);
    NodeId ctor_initializer();
    NodeId specifiers(bool type_only = false);
    NodeId type_id(bool new_type = false);
    NodeId declarator(bool abstract = false, bool new_type = false);
    NodeId parameters();
    NodeId parameter(Kind kind = Kind::Parameter);
    void function_suffix(NodeId owner);
    bool parameter_clause_ahead();
    bool nested_declarator_ahead();
    bool declaration_ahead();
    NodeId initializer();
    void bind_declarator(NodeId declarator, Category category, ScopeId owner);
    NodeId declarator_name(NodeId declarator) const;

    NodeId expression(int minimum = 1);
    NodeId unary();
    NodeId primary();
    NodeId postfix(NodeId base);
    NodeId arguments(Kind kind, const char* close);
    NodeId type_trait();
    NodeId new_expression();
    NodeId lambda();
    NodeId compound();
    NodeId statement();
    NodeId condition();
    NodeId selection();
    NodeId iteration();
    NodeId for_statement();
    NodeId try_block(bool function = false);
    NodeId handler();
};

} }
