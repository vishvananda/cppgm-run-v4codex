#include "semantic/analyzer.h"
namespace cppgm { namespace semantic {
using syntax::Kind;
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
    if (c.reference && c.kind == Conversion::Kind::Construction) temporary = conversion_objects[c.materialization].temporary;
    if (c.reference && c.kind == Conversion::Kind::User) temporary = user_conversions[c.materialization].temporary;
    if (temporary) {
        if (static_temporaries.get(temporary)) return;
        entities[temporary].definition = n; entities[temporary].is_static = true;
        ReferenceStorage storage; storage.object = temporary; storage.reference = reference; storage.conditional = conditional;
        static_temporaries.put(temporary,reference_storage.size()); reference_storage.push_back(storage);
        return;
    }
    Kind kind = ast[n].kind;
    if (kind == Kind::Parenthesized || (kind == Kind::Member && ast[n].op == OP_DOT) || kind == Kind::Subscript)
        retain_reference_object(ast[n].first,reference,conditional);
    else if (kind == Kind::Cast) retain_reference_object(ast[ast[n].first].next,reference,conditional);
    else if (kind == Kind::Binary && ast[n].op == OP_COMMA) retain_reference_object(ast[ast[n].first].next,reference,conditional);
    else if (kind == Kind::Conditional && expressions[n].category != ValueCategory::Prvalue) {
        NodeId b = ast[ast[n].first].next;
        retain_reference_object(b,reference,true); retain_reference_object(ast[b].next,reference,true);
    }
}
} }
