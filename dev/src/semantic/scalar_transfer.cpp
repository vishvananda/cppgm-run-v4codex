#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
using syntax::Kind;
bool Analyzer::scalar_transfer_node(NodeId n)
{
    if (!n) return true;
    if (scalar_transfer_nodes.empty()) scalar_transfer_nodes.resize(ast.nodes.size());
    if (scalar_transfer_nodes[n]) return scalar_transfer_nodes[n] == 2;
    ++scalar_transfer_work;
    bool safe = false;
    switch (ast[n].kind) {
    case Kind::Initializer: case Kind::ParenInitializer: case Kind::ParenArguments:
    case Kind::Identifier: case Kind::IdExpression: case Kind::Literal: case Kind::KeywordLiteral:
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: case Kind::Unary: case Kind::Postfix:
    case Kind::Parenthesized: case Kind::Subscript: case Kind::Member:
    case Kind::Compound: case Kind::ExpressionStatement: case Kind::Return:
    case Kind::If: case Kind::Condition: case Kind::Then: case Kind::Else:
        safe = true; break;
    default: break;
    }
    auto x = expressions[n];
    safe &= x.form == ExpressionForm::Ordinary;
    auto scalar_conversion = [&](std::uint32_t id) {
        if (!id) return true;
        auto c = conversions[id];
        return c.kind != Conversion::Kind::Construction && c.kind != Conversion::Kind::User &&
            c.kind != Conversion::Kind::List && c.kind != Conversion::Kind::ListPlan;
    };
    safe &= scalar_conversion(x.incoming);
    for (unsigned j = 0; safe && j < x.count; ++j) safe &= scalar_conversion(x.conversions+j);
    for (NodeId child = ast[n].first; safe && child; child = ast[child].next) safe &= scalar_transfer_node(child);
    scalar_transfer_nodes[n] = safe ? 2 : 1; return safe;
}
void Analyzer::prepare_scalar_transfer(EntityId e)
{
    if (!calls || !transfer_member(e) || !constructor_member(e)) return;
    auto m = entities[e].member_info;
    if (members[m].synthetic || !entities[e].body || members[m].delegated_constructor) return;
    bool safe = scalar_transfer_node(entities[e].body);
    for (unsigned j = 0; safe && j < members[m].action_count; ++j) {
        auto action = subobject_actions[members[m].action_begin+j];
        safe = !action.constructor && !class_value(action.type) && types[action.type].kind != TypeKind::Array &&
            scalar_transfer_node(action.initializer);
    }
    members[m].scalar_transfer_body = safe;
}
bool Analyzer::scalar_transfer_source(EntityId transfer, NodeId n) const
{
    if (!member_fact(transfer).scalar_transfer_body) return false;
    auto x = expression_fact(n);
    if (x.form != ExpressionForm::Ordinary || x.category == ValueCategory::Prvalue || !x.entity) return false;
    auto e = entities[x.entity];
    if (e.kind != EntityKind::Variable && e.kind != EntityKind::Parameter) return false;
    auto scope = scopes[e.owner].kind;
    return !e.is_static && !e.external_decl && !e.thread_local_storage &&
        (scope == ScopeKind::Block || scope == ScopeKind::Control || scope == ScopeKind::Function);
}
} }
