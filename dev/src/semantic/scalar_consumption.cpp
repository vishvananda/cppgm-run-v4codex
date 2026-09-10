#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
using syntax::Kind;
const ScalarConsumption& Analyzer::scalar_consumption(EntityId object) const
{ return scalar_consumptions[scalar_consumption_index.get(object)]; }
bool Analyzer::local_scalar(EntityId object) const
{
    if (!object) return false;
    auto e = entities[object]; auto kind = types[e.type].kind;
    auto scope = scopes[e.owner].kind;
    return e.kind == EntityKind::Variable && !e.is_static && !e.external_decl && !e.thread_local_storage &&
        (scope == ScopeKind::Block || scope == ScopeKind::Control) && !class_value(e.type) &&
        kind != TypeKind::Array && kind != TypeKind::Function && kind != TypeKind::LRef && kind != TypeKind::RRef &&
        kind != TypeKind::MemberPointer;
}
bool Analyzer::private_scalar(EntityId object) const
{ return local_scalar(object) && !(types[entities[object].type].cv & 2) && !scalar_observations.get(object); }
void Analyzer::observe_scalar(NodeId n)
{
    if (unevaluated_depth) return;
    EntityId e = expressions[n].entity;
    if (private_scalar(e)) { scalar_observations.put(e,1); ++scalar_observation_count; }
}
bool Analyzer::direct_class_call(NodeId n)
{
    if (!n || (expressions[n].ready && !expressions[n].evaluated)) return false;
    ++scalar_consumption_work;
    auto x = expressions[n]; EntityId callee = facts[n].entity;
    if ((ast[n].kind == Kind::Call || x.form == ExpressionForm::OperatorCall) &&
        x.form != ExpressionForm::Construction && x.form != ExpressionForm::Cast && callee &&
        entities[callee].kind == EntityKind::Function) {
        TypeId result = types[entities[callee].type].child;
        if (class_value(result) && !indirect_value(result)) return true;
    }
    for (NodeId child = ast[n].first; child; child = ast[child].next)
        if (direct_class_call(child)) return true;
    return false;
}
unsigned char Analyzer::scalar_truth(NodeId n)
{
    if (types[expressions[n].type].cv & 2) return 0;
    auto literal = [&](NodeId source) {
        Constant value = constant_fact(source);
        bool boolean = ast[source].kind == Kind::KeywordLiteral && (ast[source].op == KW_TRUE || ast[source].op == KW_FALSE);
        if (!value.valid && (ast[source].kind == Kind::Literal || boolean)) value = evaluate(source,facts[source].scope);
        return value;
    };
    Constant value = literal(n);
    EntityId object = expressions[n].entity;
    if (!value.valid && private_scalar(object) && integral(entities[object].type)) {
        NodeId source = entities[object].initializer;
        while (source && (ast[source].kind == Kind::Initializer || ast[source].kind == Kind::ParenInitializer ||
            ast[source].kind == Kind::ParenArguments || ast[source].kind == Kind::BracedInit || ast[source].kind == Kind::Parenthesized) &&
            ast[source].first == ast[source].last) { ++scalar_consumption_work; source = ast[source].first; }
        value = convert(literal(source),entities[object].type);
    }
    return value.valid && integral(value.type) ? (value.bits ? 2 : 1) : 0;
}
void Analyzer::prepare_scalar_consumption(EntityId object)
{
    NodeId source = entities[object].initializer;
    std::uint32_t conversion_id = 0;
    while (source && (ast[source].kind == Kind::Initializer || ast[source].kind == Kind::ParenInitializer ||
        ast[source].kind == Kind::ParenArguments || ast[source].kind == Kind::BracedInit) && ast[source].first == ast[source].last) {
        if (!conversion_id) conversion_id = expressions[source].incoming;
        source = ast[source].first;
    }
    if (!conversion_id) conversion_id = expressions[source].incoming;
    NodeId root = source;
    while (ast[root].kind == Kind::Parenthesized) root = ast[root].first;
    if (ast[root].kind != Kind::Conditional || class_value(expressions[root].type)) return;
    // The constant branch has the required terminal materialization boundary.
    // Unknown branches retain the shared cleanup path: extending this policy
    // to them reduced LowIR size but regressed measured O0 native execution.
    unsigned char truth = scalar_truth(ast[root].first);
    if (!truth || !direct_class_call(root)) return;
    TypeId target = entities[object].type;
    Conversion selected = conversion_id ? conversions[conversion_id] : conversion(source,target);
    if (!selected.valid() || selected.reference || (selected.kind != Conversion::Kind::Standard &&
        selected.kind != Conversion::Kind::Explicit && selected.kind != Conversion::Kind::Contextual)) return;
    if (!conversion_id) { conversion_id = conversions.size(); conversions.push_back(selected); }
    ScalarConsumption record; record.expression = root; record.target = target; record.conversion = conversion_id;
    record.private_destination = private_scalar(object); record.truth = truth;
    scalar_consumption_index.put(object,scalar_consumptions.size()); scalar_consumptions.push_back(record);
}
} }
