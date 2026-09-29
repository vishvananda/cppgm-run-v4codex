#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
Constant Analyzer::member_address_constant(TypeId type, EntityId member)
{
    if (!member) return Constant(type,0);
    auto owner = scopes[entities[member].owner].entity;
    auto declared = types.member_pointer(owner,entities[member].type);
    return convert(member_constant(declared,member,0),type);
}
Constant Analyzer::member_constant(TypeId type, EntityId member, std::int64_t adjustment)
{
    if (!member) return Constant(type,0);
    auto bits = std::uint64_t(adjustment);
    auto key = intern_arguments({member,std::uint32_t(bits),std::uint32_t(bits >> 32)});
    auto id = member_constant_index.get(key);
    if (!id) {
        id = member_constants.size();
        MemberConstant value; value.member = member; value.adjustment = adjustment;
        member_constants.push_back(value); member_constant_index.put(key,id);
    }
    return Constant(type,id);
}
std::uint32_t Analyzer::constant_member_receiver(std::uint32_t object, Constant member)
{
    if (!object || !member.bits) return 0;
    auto identity = key(object,member.bits);
    if (auto known = member_receiver_index.get(identity)) return known == ~std::uint32_t(0) ? 0 : known;
    auto value = member_constant_value(member);
    auto owner = scopes[entities[value.member].owner].entity;
    // The receiver is already projected to the member-pointer's owner.
    // Resolve the preserved displacement within this complete object's bases,
    // including inverse conversions and distinct repeated base subobjects.
    auto offset = constant_offset(object) + std::uint64_t(value.adjustment);
    auto root = object;
    while (constant_addresses[root].parent && class_value(constant_addresses[constant_addresses[root].parent].type) &&
        constant_addresses[root].selector != ~std::uint64_t(0) && (constant_addresses[root].selector & 0x80000000U))
        root = constant_addresses[root].parent;
    std::vector<std::uint32_t> work(1,root);
    while (!work.empty()) {
        auto address = work.back(); work.pop_back(); ++member_receiver_work;
        auto type = constant_addresses[address].type;
        auto cls = types[type].entity;
        if (cls == owner && constant_offset(address) == offset) {
            member_receiver_index.put(identity,address); return address;
        }
        for (auto b = class_facts[entities[cls].class_info].first_base; b; b = bases[b].next) {
            auto base = bases[b].base;
            work.push_back(constant_subobject(address,entities[base].type,0x80000000U | base));
        }
    }
    if (entities[types[constant_addresses[root].type].entity].complete)
        member_receiver_index.put(identity,~std::uint32_t(0));
    return 0;
}
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
    auto expression = expressions[source];
    // A wrapper can own the conversion even when its child denotes an
    // unadjusted local. Consume that fact before following the value edge.
    auto conversion = conversions[expression.incoming];
    if (conversion.derived && base_adjustments[conversion.adjustment].total) return false;
    if ((node.kind == syntax::Kind::Initializer || node.kind == syntax::Kind::ParenInitializer ||
         node.kind == syntax::Kind::BracedInit || node.kind == syntax::Kind::Parenthesized) && node.first == node.last)
        return node.first ? prove_member_pointer_value(node.first,target,budget) : true;
    if (node.kind == syntax::Kind::KeywordLiteral || node.kind == syntax::Kind::Literal ||
        (node.kind == syntax::Kind::Unary && node.op == OP_AMP && expression.form == ExpressionForm::Ordinary) ||
        (node.kind == syntax::Kind::IdExpression && entities[expression.entity].constant.valid)) {
        auto value = static_value(source,target);
        return value.kind == StaticValue::MemberFunction && !value.addend;
    }
    if (node.kind != syntax::Kind::IdExpression || !expression.entity) return false;
    auto from = expression.type;
    if (types[from].kind != TypeKind::MemberPointer) return false;
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
