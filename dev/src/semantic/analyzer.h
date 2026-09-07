#pragma once
#include "semantic/model.h"
#include "syntax/parser.h"

namespace cppgm { namespace semantic {
// Semantic facts extend the parser's sole graph, indexed by its stable NodeId.
// The parser calls this boundary before proceeding to the next source region.
class Analyzer : public syntax::DeclarationConsumer {
public:
    Analyzer(syntax::Ast& ast, IdentifierTable& ids);
    void consume(NodeId declaration) override;
    void finish();
    void write(std::ostream& out) const;
    void telemetry(std::ostream& out) const;
    Types types;
    std::vector<Entity> entities;
    std::vector<Scope> scopes;
    std::vector<Declaration> declarations;
    std::vector<Fact> facts;
    ScopeId global = 0;
private:
    syntax::Ast& ast;
    IdentifierTable& ids;
    Index ordinary, tags, namespaces, qualifiers;
    std::vector<Constant> constants;
    double analysis_ms = 0;
    std::vector<Edge> edges;
    std::vector<std::uint64_t> visited;
    std::uint64_t walk = 0;
    std::size_t lookup_work = 0, analyzed = 0, constant_work = 0;
    struct Body { NodeId node, declarator; ScopeId owner; EntityId entity; };
    std::vector<Body> bodies;
    std::size_t body_cursor = 0;
    enum class Lookup { Ordinary, Tag, Namespace, Qualifier };
    std::uint64_t key(ScopeId s, IdentifierId n) const;
    EntityId local(ScopeId s, IdentifierId n, Lookup mode = Lookup::Ordinary) const;
    EntityId lookup(ScopeId s, IdentifierId n, Lookup mode = Lookup::Ordinary, bool qualified = false);
    EntityId imported(ScopeId s, IdentifierId n, Lookup mode, std::uint64_t visit);
    EntityId resolve(NodeId name, ScopeId s, Lookup mode = Lookup::Ordinary);
    ScopeId name_owner(NodeId name, ScopeId s);
    ScopeId common_ancestor(ScopeId a, ScopeId b) const;
    ScopeId target(EntityId e) const;
    IdentifierId terminal(NodeId name) const;
    NodeId decl_name(NodeId d) const;
    NodeId child(NodeId n, syntax::Kind k) const;
    bool spec_has(NodeId n, ETokenType op) const;
    bool encloses(ScopeId outer, ScopeId inner) const;
    ScopeId make_scope(ScopeKind k, ScopeId parent, IdentifierId name = 0, EntityId e = 0, bool visible = true);
    void attach_scope(ScopeId s, ScopeId parent);
    EntityId make_entity(EntityKind k, ScopeId s, IdentifierId name, NodeId source);
    void bind(ScopeId s, IdentifierId n, EntityId e);
    std::uint32_t record(ScopeId s, EntityId e, NodeId source, TypeId type, EntityKind kind);
    void add_edge(ScopeId s, ScopeId to, bool is_inline = false);
    void declaration(NodeId n, ScopeId s);
    void simple(NodeId n, ScopeId s);
    void function_body(const Body& body);
    void statements(NodeId n, ScopeId s);
    void template_declaration(NodeId n, ScopeId s);
    void namespace_declaration(NodeId n, ScopeId s);
    TypeId class_type(NodeId n, ScopeId s, IdentifierId anonymous_name = 0, bool emit = true, bool static_union = false);
    TypeId enum_type(NodeId n, ScopeId s, IdentifierId anonymous_name = 0, bool emit = true);
    TypeId specifiers(NodeId n, ScopeId s, IdentifierId anonymous_name = 0);
    TypeId type_id(NodeId n, ScopeId s);
    TypeId declarator(NodeId n, TypeId base, ScopeId s);
    TypeId parameter(NodeId n, ScopeId s);
    EntityId declare_object(NodeId d, NodeId init, TypeId t, NodeId specs, ScopeId s, NodeId source);
    Constant evaluate(NodeId n, ScopeId s);
    Constant evaluate_value(NodeId n, ScopeId s);
    Constant binary(ETokenType op, Constant a, Constant b);
    Constant convert(Constant value, TypeId to, bool explicit_cast = false);
    TypeId expression_type(NodeId n, ScopeId s, bool decltype_form = false);
    std::uint64_t size(TypeId t, bool alignment = false);
    bool integral(TypeId t) const;
    bool is_unsigned(TypeId t) const;
    bool scoped_enum(TypeId t) const;
    unsigned width(TypeId t) const;
    void write_scope(std::ostream& out, ScopeId s, unsigned depth) const;
    void write_type(std::ostream& out, TypeId t, NodeId display_name = 0, ETokenType key = TOK_INVALID) const;
    void write_name(std::ostream& out, NodeId n) const;
    void spelling(std::ostream& out, IdentifierId n) const;
};
} }
