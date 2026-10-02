#include "semantic/analyzer.h"
#include <stdexcept>
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
        kind != TypeKind::Array && kind != TypeKind::Function && kind != TypeKind::LRef && kind != TypeKind::RRef;
}
bool Analyzer::private_scalar(EntityId object) const
{ return local_scalar(object) && !(types[entities[object].type].cv & 2) && !scalar_observations.get(object); }
void Analyzer::observe_scalar(NodeId n, bool write)
{
    EntityId e = expressions[n].entity;
    if (auto capture = e ? capture_object(e) : 0) {
        auto value = expressions[n];
        if (!value.object_use) record_object(value,0,0,0);
        object_uses[value.object_use].capture = capture;
        expressions.set(n,value);
    }
    if (unevaluated_depth || discarded_statement()) return;
    if (!write && e && types[entities[e].type].kind == TypeKind::MemberPointer) member_pointer_exposed.put(e,1);
    if (e && entities[e].kind == EntityKind::Variable && entities[e].specialization) entities[e].emission |= Entity::Used;
    if (private_scalar(e)) { scalar_observations.put(e,1); ++scalar_observation_count; }
}
bool Analyzer::direct_class_call(NodeId n)
{
    if (!n || (expressions[n].ready && !expressions[n].evaluated)) return false;
    ++scalar_consumption_work;
    auto x = expressions[n]; EntityId callee = facts[n].entity;
    // A scalar member result can consume an observable class temporary just
    // as a direct class-returning call does. Retain the existing proof that
    // the selector is a private, unmodified constant before retiring an arm.
    auto temporary = object_fact(n).temporary;
    if (temporary_cleanup(temporary) && destructor_needed(object_destructor(temporary))) return true;
    if ((ast.kind(n) == Kind::Call || x.form == ExpressionForm::OperatorCall) &&
        x.form != ExpressionForm::Construction && x.form != ExpressionForm::Cast && callee &&
        entities[callee].kind == EntityKind::Function) {
        TypeId result = types[entities[callee].type].child;
        if (class_value(result) && !indirect_value(result)) return true;
    }
    for (NodeId child = ast.first(n); child; child = ast.next(child))
        if (direct_class_call(child)) return true;
    return false;
}
unsigned char Analyzer::scalar_truth(NodeId n)
{
    if (types[expressions[n].type].cv & 2) return 0;
    auto literal = [&](NodeId source) {
        Constant value = runtime_constant_fact(source);
        bool boolean = ast.kind(source) == Kind::KeywordLiteral && (ast.op(source) == KW_TRUE || ast.op(source) == KW_FALSE);
        if (!value.valid && (ast.kind(source) == Kind::Literal || boolean)) value = evaluate(source,facts[source].scope);
        return value;
    };
    Constant value = literal(n);
    EntityId object = expressions[n].entity;
    if (!value.valid && ast.kind(n) == Kind::IdExpression && object) value = entities[object].constant;
    if (!value.valid && private_scalar(object) && integral(entities[object].type)) {
        NodeId source = entities[object].initializer;
        while (source && (ast.kind(source) == Kind::Initializer || ast.kind(source) == Kind::ParenInitializer ||
            ast.kind(source) == Kind::ParenArguments || ast.kind(source) == Kind::BracedInit || ast.kind(source) == Kind::Parenthesized) &&
            ast.first(source) == ast.last(source)) { ++scalar_consumption_work; source = ast.first(source); }
        value = convert(literal(source),entities[object].type);
    }
    return value.valid && integral(value.type) ? (value.bits ? 2 : 1) : 0;
}
void Analyzer::prepare_scalar_consumption(EntityId object)
{
    if (types[entities[object].type].kind == TypeKind::MemberPointer) { prepare_member_pointer_value(object); return; }
    NodeId source = entities[object].initializer;
    std::uint32_t conversion_id = 0;
    while (source && (ast.kind(source) == Kind::Initializer || ast.kind(source) == Kind::ParenInitializer ||
        ast.kind(source) == Kind::ParenArguments || ast.kind(source) == Kind::BracedInit) && ast.first(source) == ast.last(source)) {
        if (!conversion_id) conversion_id = expressions[source].incoming;
        source = ast.first(source);
    }
    if (!conversion_id) conversion_id = expressions[source].incoming;
    NodeId root = source;
    while (ast.kind(root) == Kind::Parenthesized) root = ast.first(root);
    if (ast.kind(root) != Kind::Conditional || class_value(expressions[root].type)) return;
    // The constant branch has the required terminal materialization boundary.
    // Unknown branches retain the shared cleanup path: extending this policy
    // to them reduced LowIR size but regressed measured O0 native execution.
    unsigned char truth = scalar_truth(ast.first(root));
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
