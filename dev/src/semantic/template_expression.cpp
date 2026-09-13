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
    case Kind::Member:
        if (!check_fixed_member(n,s)) return;
        template_fixed_expressions.put(source,n); ++template_fixed_work;
        return;
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
        // Prototype parameters provide type-query ordinals without runtime
        // objects. Body parameters also have ordinals, but still require
        // ordinary fixed-expression validation and concrete object identities.
        if (signature_parameters.get(e) && scopes[entities[e].owner].kind != ScopeKind::Function) return;
        bool object = class_value(value) || types[value].kind == TypeKind::Pointer || types[value].kind == TypeKind::Function;
        if (!e || binding.dependent || (entities[e].kind != EntityKind::Variable && entities[e].kind != EntityKind::Parameter) ||
            !type || scopes[entities[e].owner].kind == ScopeKind::Class || (types[value].kind != TypeKind::Fundamental && !object))
            return;
        break;
    }
    case Kind::Cast: {
        auto target = type_id(first,s);
        first = ast[first].next;
        if (!first) return;
        if (!fixed(first)) return;
        auto source_type = expressions[fixed(first)].type;
        auto kind = types[target].kind;
        bool reference = kind == TypeKind::LRef || kind == TypeKind::RRef;
        bool related = reference && (types.unqualified(source_type) == types.unqualified(types[target].child) ||
            derived_from(source_type,types[target].child) || derived_from(types[target].child,source_type));
        // Object-preserving reference casts have no source-owned temporary.
        // Other class conversions must publish their own materialization recipe.
        if (reference ? !related : class_value(source_type) ? !fundamental(target,FT_VOID) :
            kind != TypeKind::Fundamental && kind != TypeKind::Pointer) return;
        if (reference && class_value(source_type))
            for (auto e = types[source_type].entity; e; e = direct_base(e))
                if (class_facts[entities[e].class_info].first_conversion) return;
        break;
    }
    case Kind::Sizeof: case Kind::TypeTrait:
        if (ast[first].kind == Kind::TypeId) {
            if (types[type_id(first,s)].kind != TypeKind::Fundamental) return;
        } else if (!fixed(first)) return;
        break;
    case Kind::Parenthesized:
        if (!fixed(first)) return;
        break;
    case Kind::Unary: case Kind::Postfix:
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
    if (reuse_template_value(n,s,result)) return true;
    if (ast[n].kind == Kind::Call) { reuse_fixed_call(n,source,s,result); return true; }
    result = expressions[source]; result.incoming = 0;
    if (reuse_template_field(n,s,result)) {
        facts[n].type = facts[source].type; facts[n].entity = result.entity;
        return true;
    }
    auto first = ast[n].first;
    if (ast[n].kind == Kind::Member) {
        expression(first,s); // The shared receiver edge is projected by object_fact.
        facts[n].type = facts[source].type; facts[n].entity = result.entity; facts[n].value = facts[source].value;
        return true;
    }
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
    reuse_value_conversions(n,source,result);
    if (result.entity && entities[result.entity].template_pattern) {
        if (nonstatic_field(result.entity)) result.entity = template_field_use(result.entity,s).entity;
        else {
        auto declaration = ast.projected(entities[result.entity].source,occurrence.context);
        auto entity = facts[declaration].entity;
        if (!entity || entities[entity].template_pattern) throw std::logic_error("missing concrete fixed-expression declaration");
        if (result.type != value_type(entities[entity].type)) throw std::logic_error("fixed expression declaration type changed");
        result.entity = entity;
        }
    }
    if (ast[n].kind == Kind::Assignment || ast[n].op == OP_INC || ast[n].op == OP_DEC ||
        (ast[n].kind == Kind::Unary && ast[n].op == OP_AMP)) observe_scalar(first);
    facts[n].type = facts[source].type;
    if (!template_value_dependence.get(occurrence.source)) facts[n].value = facts[source].value;
    facts[n].entity = result.entity;
    return true;
}
} }
