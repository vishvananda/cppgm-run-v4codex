#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
void Analyzer::prepare_context_reference(EntityId e)
{
    if (!evaluation_context_present) return;
    auto entity = entities[e]; auto type = types[entity.type];
    if ((type.kind != TypeKind::LRef && type.kind != TypeKind::RRef) ||
        entity.is_static || entity.thread_local_storage || scopes[entity.owner].kind == ScopeKind::Namespace ||
        !entity.initializer) return;
    EvaluationScope mode(*this,true);
    auto uses = evaluation_mode_uses;
    auto value = constant_initialize(entity.initializer,entity.type,entity.owner);
    if (!value.valid || evaluation_mode_uses == uses) return;
    auto address = constant_static_value(value);
    if (address.kind == StaticValue::Address || address.kind == StaticValue::String) {
        entities[e].constant = value; mode_sensitive_objects.put(e,1); return;
    }
    ContextReference plan;
    if (value.bits) {
        auto storage = constant_storage[constant_addresses[value.bits].storage].entity;
        if (storage) {
            plan.storage = storage; plan.offset = constant_offset(value.bits);
            context_reference_index.put(e,context_references.size()); context_references.push_back(plan); return;
        }
    }
    auto scalar = constant_indirect(value);
    if (!scalar.valid || !constant_persistent(scalar)) return;
    auto n = entity.initializer;
    while (ast[n].kind == syntax::Kind::Initializer || ast[n].kind == syntax::Kind::ParenInitializer ||
        ast[n].kind == syntax::Kind::ParenArguments) n = ast[n].first;
    auto conversion = conversions[expressions[n].incoming];
    auto temporary = converted_temporary(conversion);
    if (!temporary) temporary = object_fact(n).temporary;
    if (!temporary) {
        temporary = make_entity(EntityKind::Variable,entity.owner,0,n);
        entities[temporary].type = type.child; entities[temporary].definition = n;
    }
    entities[temporary].constant = scalar;
    mode_sensitive_objects.put(temporary,1);
    plan.storage = temporary; plan.initialize = true;
    context_reference_index.put(e,context_references.size()); context_references.push_back(plan);
}
} }
