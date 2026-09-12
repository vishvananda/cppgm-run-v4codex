#include "semantic/analyzer.h"
#include <stdexcept>
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::check_fixed_expression(NodeId n, ScopeId s)
{
    auto source = ast.nodes.occurrences[n].source;
    if (template_fixed_expressions.get(source)) return;
    auto fixed = [&](NodeId child) { return template_fixed_expressions.get(ast.nodes.occurrences[child].source); };
    auto node = ast[n];
    auto first = node.first;
    switch (node.kind) {
    case Kind::Call:
        if (!check_fixed_call(n,s)) return;
        template_fixed_expressions.put(source,n); ++template_fixed_work;
        return;
    case Kind::Literal:
        if (ast.literals[node.literal].suffix || ast.literals[node.literal].kind == LiteralKind::string) return;
        break;
    case Kind::KeywordLiteral:
        if (node.op == KW_THIS) return;
        break;
    case Kind::IdExpression: {
        auto binding = template_bindings[template_binding_index.get(ast.nodes.occurrences[node.detail].source)];
        auto e = binding.entity;
        auto type = entities[e].type;
        auto value = value_type(type);
        bool function_pointer = types[value].kind == TypeKind::Function ||
            (types[value].kind == TypeKind::Pointer && types[types[value].child].kind == TypeKind::Function);
        if (!e || binding.dependent || (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter) ||
            !type || scopes[entities[e].owner].kind == ScopeKind::Class || (types[value].kind != TypeKind::Fundamental && !function_pointer))
            return;
        break;
    }
    case Kind::Cast:
        // Type/layout-dependent and class conversions have their own owners.
        // This slice contains no materialization, access or overload decisions.
        if (types[type_id(first,s)].kind != TypeKind::Fundamental) return;
        first = ast[first].next;
        if (!first) return;
        if (!fixed(first)) return;
        break;
    case Kind::Sizeof: case Kind::TypeTrait:
        if (ast[first].kind == Kind::TypeId) {
            if (types[type_id(first,s)].kind != TypeKind::Fundamental) return;
        } else if (!fixed(first)) return;
        break;
    case Kind::Parenthesized: case Kind::Unary: case Kind::Postfix:
    case Kind::Binary: case Kind::Assignment: case Kind::Conditional: case Kind::Subscript:
        for (auto c = first; c; c = ast[c].next)
            if (!fixed(c) || class_value(expressions[fixed(c)].type)) return;
        break;
    default: return;
    }
    // Validate with the ordinary semantic rules, without observing a concrete
    // local object or demanding storage. The source graph owns the completed
    // type/category and immutable operand-conversion slice for all occurrences.
    ++unevaluated_depth;
    try { expression(n,s); }
    catch (...) { --unevaluated_depth; throw; }
    --unevaluated_depth;
    template_fixed_expressions.put(source,n); ++template_fixed_work;
}
bool Analyzer::reuse_fixed_expression(NodeId n, ScopeId s, Expression& result)
{
    auto occurrence = ast.nodes.occurrences[n];
    if (!occurrence.context) return false;
    auto source = template_fixed_expressions.get(occurrence.source);
    if (!source) return false;
    ++template_fixed_uses;
    if (ast[n].kind == Kind::Call) { reuse_fixed_call(n,source,s,result); return true; }
    result = expressions[source]; result.incoming = 0;
    auto first = ast[n].first;
    bool unevaluated = ast[n].kind == Kind::Sizeof || ast[n].kind == Kind::TypeTrait;
    if (ast[n].kind == Kind::Cast || (unevaluated && ast[first].kind == Kind::TypeId)) {
        facts[first].type = facts[ast[source].first].type;
        first = ast[first].next;
    }
    if (unevaluated) ++unevaluated_depth;
    try {
    for (auto c = first; c; c = ast[c].next) {
        expression(c,s);
        auto incoming = expressions[template_fixed_expressions.get(ast.nodes.occurrences[c].source)].incoming;
        if (incoming >= result.conversions && incoming-result.conversions < result.count) {
            expressions[c].incoming = incoming;
            if (conversions[incoming].reference && !conversions[incoming].temporary) observe_scalar(c);
        }
    }
    } catch (...) { if (unevaluated) --unevaluated_depth; throw; }
    if (unevaluated) --unevaluated_depth;
    if (result.entity && entities[result.entity].template_pattern) {
        auto declaration = ast.projected(entities[result.entity].source,occurrence.context);
        auto entity = facts[declaration].entity;
        if (!entity || entities[entity].template_pattern) throw std::logic_error("missing concrete fixed-expression declaration");
        if (result.type != value_type(entities[entity].type)) throw std::logic_error("fixed expression declaration type changed");
        result.entity = entity;
    }
    if (ast[n].kind == Kind::Assignment || ast[n].op == OP_INC || ast[n].op == OP_DEC ||
        (ast[n].kind == Kind::Unary && ast[n].op == OP_AMP)) observe_scalar(first);
    facts[n].type = facts[source].type;
    facts[n].value = facts[source].value;
    facts[n].entity = result.entity;
    return true;
}
} }
