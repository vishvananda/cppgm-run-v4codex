#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::prepare_conversion_result(EntityId e)
{
    auto entity = entities[e];
    if (!entity.member_info || !members[entity.member_info].conversion_target ||
        members[entity.member_info].virtual_member || entity.body_state != FactState::Success) return;
    auto target = types[entity.type].child;
    if (!integral(target) && !floating_type(target)) return;
    // O0's named-constant forwarding rule consumes an already established
    // scalar fact. It neither evaluates arbitrary bodies nor demands a body.
    // One completed function owns this proof; bounded wrappers are the only
    // syntax inspected. More elaborate bodies keep their ordinary call.
    ++conversion_result_work;
    NodeId body = entity.body;
    if (ast[body].kind != syntax::Kind::Compound) return;
    NodeId statement = ast[body].first;
    if (!statement || ast[statement].next || ast[statement].kind != syntax::Kind::Return) return;
    NodeId source = ast[statement].first;
    auto conversion = conversions[expressions[source].incoming];
    if (conversion.kind != Conversion::Kind::Standard || conversion.reference || conversion.function) return;
    unsigned budget = 8;
    while (ast[source].kind == syntax::Kind::Parenthesized && budget) {
        --budget; ++conversion_result_work; source = ast[source].first;
    }
    if (ast[source].kind != syntax::Kind::IdExpression ||
        expressions[source].form != ExpressionForm::Ordinary || (types[expressions[source].type].cv & 2)) return;
    EntityId named = expressions[source].entity;
    if (!named || (entities[named].kind != EntityKind::Enumerator &&
        (entities[named].kind != EntityKind::Variable || !entities[named].constant.valid ||
         (!entities[named].is_static && scopes[entities[named].owner].kind != ScopeKind::Namespace)))) return;
    auto value = entities[named].constant;
    if (!integral(value.type) && !floating_type(value.type)) return;
    value = convert(value,target);
    if (!value.valid) return;
    conversion_results.put(e,constants.size()); constants.push_back(value);
}
} }
