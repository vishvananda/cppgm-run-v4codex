#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::schedule_parameter_bodies(EntityId& cursor)
{
    // New bodies can introduce function declarations. Advance once over each
    // entity, interleaved with body demand; never retry completed declarations.
    while (cursor < entities.size()) {
        EntityId e = cursor++;
        if (entities[e].kind != EntityKind::Function || entities[e].template_info) continue;
        Type f = types[entities[e].type];
        for (unsigned j = 0; j < f.count; ++j) query_parameter_representation(types.parameters[f.offset+j]);
    }
}
void Analyzer::query_parameter_representation(TypeId type)
{
    if (!class_value(type) || !entities[types[type].entity].complete) return;
    EntityId cls = types[type].entity;
    auto info = entities[cls].class_info;
    if (class_facts[info].parameter_state) return;
    class_facts[info].parameter_state = 1; ++parameter_queries;
    TypeId target = entities[cls].type;
    prepare_value_boundary(target);
    auto facts = class_facts[info];
    if (facts.parameter_abi == 1 || !trivial_destructor(target) || !facts.first_base ||
        bases[facts.first_base].next || (facts.declared_transfers & unsigned(TransferKind::MoveConstructor))) return;
    TypeId base = entities[bases[facts.first_base].base].type;
    if (size(base) != size(target) || !copy_storage_type(base)) return;
    for (auto d = scopes[entities[cls].scope].first_decl; d; d = declarations[d].next) {
        ++parameter_query_work;
        EntityId field = declarations[d].entity;
        if (nonstatic_field(field) || field_fact(field).bit_field) return;
    }
    EntityId copy = select_transfer(target,types.qualify(target,1),ValueCategory::Lvalue,false);
    if (!copy || deleted_transfer(copy)) return;
    auto member = members[entities[copy].member_info];
    Type signature = types[entities[copy].type];
    if (member.transfer != TransferKind::CopyConstructor || member.synthetic || !member.in_class_body ||
        !member.body || ast[member.body].kind != syntax::Kind::Compound || ast[member.body].first || signature.count != 1) return;
    Type parameter = types[types.parameters[signature.offset]];
    if (parameter.kind != TypeKind::LRef || (types[parameter.child].cv & 2)) return;
    class_facts[info].parameter_state = 2;
    class_facts[info].parameter_transfer = copy;
    require_member_body(copy);
}
void Analyzer::finish_parameter_representation(TypeId type)
{
    if (!class_value(type)) return;
    auto info = entities[types[type].entity].class_info;
    if (class_facts[info].parameter_state != 2) return;
    class_facts[info].parameter_state = 1;
    EntityId copy = class_facts[info].parameter_transfer;
    auto member = members[entities[copy].member_info];
    if (member.demand != DemandState::Complete || member.delegated_constructor || member.action_count != 1) return;
    ++parameter_query_work;
    auto action = subobject_actions[member.action_begin];
    if (action.field || !action.initializer) return;
    EntityId selected = facts[action.initializer].entity;
    if (!trivial_transfer(selected)) return;
    auto call = expressions[action.initializer];
    if (call.argument_count != 1) return;
    auto conversion = conversions[call.conversions];
    if (conversion.kind != Conversion::Kind::Standard || !conversion.reference || !conversion.derived ||
        conversion.temporary || conversion.function) return;
    NodeId argument = call_arguments[call.arguments];
    while (ast[argument].kind == syntax::Kind::Parenthesized) {
        ++parameter_query_work; argument = ast[argument].first;
    }
    if (ast[argument].kind != syntax::Kind::IdExpression) return;
    auto source = expressions[argument];
    if (source.form != ExpressionForm::Ordinary || source.category != ValueCategory::Lvalue || !source.entity) return;
    auto parameter = entities[source.entity];
    if (parameter.kind != EntityKind::Parameter || parameter.owner != entities[copy].scope) return;
    // Only the complete base representation moves. No destination identity is
    // observed or stored by this body; language triviality/return ABI stay intact.
    class_facts[info].parameter_state = 3;
    class_facts[info].parameter_abi = 1;
}
} }
