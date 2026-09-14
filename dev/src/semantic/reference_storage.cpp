#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
using syntax::Kind;
void Analyzer::retain_initializer_references(EntityId e)
{
    auto entity = entities[e];
    if ((!entity.is_static && scopes[entity.owner].kind != ScopeKind::Namespace) || entity.thread_local_storage) return;
    auto bound_plan = [&](NodeId n) {
        while (n) {
            auto x = expressions[n];
            auto c = conversions[x.incoming];
            if (c.kind != Conversion::Kind::List && x.form == ExpressionForm::ListValue) c = conversions[x.conversions];
            if (c.kind == Conversion::Kind::List) {
                auto object = list_objects[c.materialization];
                if (list_plans[object.plan].aggregate) return object.initializer;
            }
            n = reference_operand(n);
        }
        return std::uint32_t(0);
    };
    auto kind = types[entity.type].kind;
    auto plan = initializer_plan(entity.initializer,entity.type);
    if (kind == TypeKind::LRef || kind == TypeKind::RRef) {
        // Class materializations already have a selected semantic identity.
        // Publish the required lifetime before constexpr validation reads it.
        retain_reference_object(entity.initializer,e,false);
        plan = bound_plan(entity.initializer);
    }
    if (!plan) return;
    std::vector<std::uint32_t> work(1,plan);
    while (!work.empty()) {
        auto action = initializers[work.back()]; work.pop_back();
        if (action.kind == InitKind::Group) {
            for (auto child = action.first; child; child = initializers[child].next) work.push_back(child);
            continue;
        }
        auto type = types[action.type];
        if (type.kind != TypeKind::LRef && type.kind != TypeKind::RRef) continue;
        auto n = action.source;
        while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) n = ast[n].first;
        if (!n) continue;
        auto c = conversions[expressions[n].incoming];
        if (class_value(type.child)) {
            retain_reference_object(n,e,false);
            if (auto nested = bound_plan(n)) work.push_back(nested);
            continue;
        }
        if (expressions[n].category != ValueCategory::Prvalue && !c.temporary) continue;
        auto k = key(n,action.type);
        if (retained_scalar_bindings.get(k)) continue;
        auto temporary = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,entity.owner),0,n);
        entities[temporary].type = type.child; entities[temporary].definition = n; entities[temporary].is_static = true;
        ReferenceStorage storage; storage.object = temporary; storage.reference = e; storage.scalar = true;
        static_temporaries.put(temporary,reference_storage.size()); reference_storage.push_back(storage);
        retained_scalar_bindings.put(k,temporary);
        // The reference owns the temporary even when its value needs dynamic
        // initialization. Evaluate just the selected scalar conversion here.
        c.target = type.child; c.reference = c.temporary = false;
        auto value = constant_node_conversion(n,c,entity.owner);
        if (value.valid && constant_persistent(value)) entities[temporary].constant = value;
    }
}
NodeId Analyzer::reference_operand(NodeId n) const
{
    auto c = conversions[expressions[n].incoming];
    if (c.kind == Conversion::Kind::List && list_plans[list_objects[c.materialization].plan].direct_binding)
        return call_argument(list_objects[c.materialization].call);
    // A scalar conversion/bit-field binding creates its own temporary; a
    // conversion function returning a reference does not extend its receiver.
    if (c.reference && (c.temporary || c.kind == Conversion::Kind::User)) return 0;
    Kind kind = ast[n].kind;
    if (kind == Kind::Parenthesized || kind == Kind::Initializer || kind == Kind::ParenInitializer || kind == Kind::ParenArguments)
        return ast[n].first;
    auto x = expressions[n];
    if (x.form == ExpressionForm::Cast && x.category != ValueCategory::Prvalue) {
        auto conversion = conversions[x.conversions];
        if (conversion.kind == Conversion::Kind::Standard || conversion.kind == Conversion::Kind::Explicit)
            return kind == Kind::Cast ? ast[ast[n].first].next : ast[ast[ast[n].first].next].first;
    }
    if (x.form != ExpressionForm::Ordinary) return 0;
    if (kind == Kind::Member && ast[n].op == OP_DOT && nonstatic_field(x.entity)) {
        auto type = types[entities[x.entity].type].kind;
        if (type != TypeKind::LRef && type != TypeKind::RRef) return ast[n].first;
    }
    if (kind == Kind::Subscript) {
        for (NodeId operand = ast[n].first; operand; operand = ast[operand].next)
            if (types[expressions[operand].type].kind == TypeKind::Array) return operand;
    }
    if (kind == Kind::Binary && ast[n].op == OP_DOTSTAR) return ast[n].first;
    if (kind == Kind::Binary && ast[n].op == OP_COMMA) return ast[ast[n].first].next;
    return 0;
}
void Analyzer::local_reference(NodeId n, EntityId reference, bool conditional)
{
    if (!n) return;
    ++reference_binding_work;
    auto c = conversions[expressions[n].incoming];
    EntityId temporary = c.reference ? converted_temporary(c) : 0;
    if (!temporary && c.reference && (c.temporary || c.kind == Conversion::Kind::User)) return;
    if (!temporary) temporary = object_fact(n).temporary;
    if (temporary) {
        if (conditional) {
            auto next = reference_choices(reference);
            conditional_references.put(reference,reference_alternatives.size());
            reference_alternatives.push_back({temporary,next});
        } else {
            reference_temporaries.put(reference,temporary);
            object_destructors.put(reference,object_destructor(temporary));
        }
        return;
    }
    if (auto operand = reference_operand(n)) local_reference(operand,reference,conditional);
    else if (ast[n].kind == Kind::Conditional && expressions[n].category != ValueCategory::Prvalue) {
        NodeId b = ast[ast[n].first].next;
        local_reference(b,reference,true); local_reference(ast[b].next,reference,true);
    }
}
void Analyzer::static_reference(EntityId e)
{
    NodeId n = entities[e].initializer;
    while (ast[n].kind == Kind::Initializer || ast[n].kind == Kind::ParenInitializer || ast[n].kind == Kind::ParenArguments) n = ast[n].first;
    if (!n) return;
    // Constant backing storage already belongs to the static initializer plan.
    if (static_value(n,entities[e].type).kind != StaticValue::Invalid) return;
    Conversion c = conversions[expressions[n].incoming];
    bool scalar = !class_value(types[entities[e].type].child) &&
        (expressions[n].category == ValueCategory::Prvalue || c.temporary);
    if (c.kind == Conversion::Kind::User) {
        TypeId returned = types[entities[c.function].type].child;
        scalar = !class_value(types[entities[e].type].child) && (user_conversions[c.materialization].result.temporary ||
            (types[returned].kind != TypeKind::LRef && types[returned].kind != TypeKind::RRef && !class_value(returned)));
    }
    if (c.kind == Conversion::Kind::List) scalar = false;
    if (scalar) {
        EntityId object = make_entity(EntityKind::Variable,make_scope(ScopeKind::Block,entities[e].owner),0,n);
        entities[object].type = types[entities[e].type].child;
        entities[object].definition = n; entities[object].is_static = true;
        ReferenceStorage storage; storage.object = object; storage.reference = e; storage.scalar = true;
        static_temporaries.put(object,reference_storage.size()); reference_storage.push_back(storage);
        reference_scalars.put(e,object);
    } else retain_reference_object(n,e,false);
}
void Analyzer::retain_reference_object(NodeId n, EntityId reference, bool conditional)
{
    if (!n) return;
    EntityId temporary = object_fact(n).temporary;
    Conversion c = conversions[expressions[n].incoming];
    if (c.reference) temporary = bound_temporary(n);
    if (temporary) {
        if (static_temporaries.get(temporary)) return;
        entities[temporary].definition = n; entities[temporary].is_static = true;
        ReferenceStorage storage; storage.object = temporary; storage.reference = reference; storage.conditional = conditional;
        static_temporaries.put(temporary,reference_storage.size()); reference_storage.push_back(storage);
        return;
    }
    Kind kind = ast[n].kind;
    if (auto operand = reference_operand(n)) retain_reference_object(operand,reference,conditional);
    else if (kind == Kind::Conditional && expressions[n].category != ValueCategory::Prvalue) {
        NodeId b = ast[ast[n].first].next;
        retain_reference_object(b,reference,true); retain_reference_object(ast[b].next,reference,true);
    }
}
} }
