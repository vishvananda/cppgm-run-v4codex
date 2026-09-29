#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::record_member_pointer_write(NodeId destination, NodeId source)
{
    auto object = expressions[destination].entity;
    if (!object || unevaluated_depth || types[entities[object].type].kind != TypeKind::MemberPointer) return;
    auto next = member_pointer_write_heads.get(object);
    member_pointer_write_heads.put(object,member_pointer_writes.size());
    member_pointer_writes.push_back({source,next});
}
bool Analyzer::member_pointer_zero_adjustment(EntityId object) const
{
    return member_pointer_value_states.get(object) == 2;
}
bool Analyzer::prove_member_pointer_value(NodeId source, TypeId target, unsigned& budget)
{
    if (!source || !budget) return false;
    --budget; ++member_pointer_proof_work;
    auto node = ast[source];
    if ((node.kind == syntax::Kind::Initializer || node.kind == syntax::Kind::ParenInitializer ||
         node.kind == syntax::Kind::BracedInit || node.kind == syntax::Kind::Parenthesized) && node.first == node.last)
        return node.first ? prove_member_pointer_value(node.first,target,budget) : true;
    auto expression = expressions[source];
    if (node.kind == syntax::Kind::KeywordLiteral || node.kind == syntax::Kind::Literal ||
        (node.kind == syntax::Kind::Unary && node.op == OP_AMP) ||
        (node.kind == syntax::Kind::IdExpression && entities[expression.entity].constant.valid)) {
        auto value = static_value(source,target);
        return value.kind == StaticValue::MemberFunction && !value.addend;
    }
    if (node.kind != syntax::Kind::IdExpression || !expression.entity) return false;
    auto from = expression.type;
    if (types[from].kind != TypeKind::MemberPointer) return false;
    auto conversion = conversions[expression.incoming];
    if (conversion.derived && base_adjustments[conversion.adjustment].total) return false;
    prepare_member_pointer_value(expression.entity,&budget);
    return member_pointer_zero_adjustment(expression.entity);
}
void Analyzer::prepare_member_pointer_value(EntityId object, unsigned* remaining)
{
    if (member_pointer_value_states.get(object)) { ++member_pointer_proof_hits; return; }
    member_pointer_value_states.put(object,1); // Active is conservatively unknown.
    auto entity = entities[object];
    bool proven = local_scalar(object) && !(types[entity.type].cv & 2) && !member_pointer_exposed.get(object);
    unsigned local_budget = 64;
    unsigned& budget = remaining ? *remaining : local_budget;
    if (proven) proven = prove_member_pointer_value(entity.initializer,entity.type,budget);
    // Every explicit write participates, independently of CFG order. A single
    // unknown write or escaped reference invalidates the entire owner's proof.
    for (auto at = member_pointer_write_heads.get(object); proven && at; at = member_pointer_writes[at].next)
        proven = prove_member_pointer_value(member_pointer_writes[at].source,entity.type,budget);
    member_pointer_value_states.put(object,proven ? 2 : 3);
}
} }
