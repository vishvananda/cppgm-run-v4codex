#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
IRType Procedural::result_type() const
{ return p.signatures[p.functions[function.index-1].signature.index-1].result; }
Value Procedural::class_temporary(EntityId object, TypeId t)
{
    if (!object) throw std::logic_error("missing class value storage identity");
    if (sem.static_temporary(object).object) return binding(object);
    if (!objects[object] || p.slots[objects[object].index-1].owner.index != function.index)
        objects[object] = builder->add_slot(0,type(t));
    return Value(Operand::slot(objects[object]),type(t),t,true);
}
Value Procedural::class_address(EntityId object, TypeId t)
{
    Value pointer = address(class_temporary(object,t));
    if (!sem.static_temporary(object).object) object_addresses[object] = lowir_model::ValueId(pointer.operand.ref);
    return pointer;
}
void Procedural::construct_value(NodeId n, const semantic::Conversion& c, Value destination)
{
    if (c.kind == semantic::Conversion::Kind::List) { list_conversion(c,destination); return; }
    if (c.kind == semantic::Conversion::Kind::User) { user_conversion(n,c,destination); return; }
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    auto fact = sem.expression_fact(n);
    bool construction = c.kind == semantic::Conversion::Kind::Construction;
    auto materialized = construction ? sem.conversion_objects[c.materialization] : semantic::ConversionObject();
    bool elided = construction ? materialized.elided : c.empty_copy && fact.category == ValueCategory::Prvalue;
    if (construction && materialized.branches) { conditional(n,false,destination,materialized.branches); return; }
    if (elided && ast[n].kind == Kind::Conditional) { conditional(n,false,destination); return; }
    if (elided && fact.form == semantic::ExpressionForm::ListValue) {
        list_conversion(sem.conversion_fact(fact.conversions),destination); return;
    }
    if (elided && fact.form == semantic::ExpressionForm::Construction) {
        construct(sem.facts[n].entity,n,destination); return;
    }
    if (elided && fact.form == semantic::ExpressionForm::Cast) {
        NodeId first = ast[n].first;
        NodeId operand = ast[n].kind == Kind::Cast ? ast[first].next : ast[ast[first].next].first;
        construct_value(operand,sem.conversion_fact(fact.conversions),destination); return;
    }
    if (elided && (ast[n].kind == Kind::Call || fact.form == semantic::ExpressionForm::OperatorCall)) {
        call(n,destination); return;
    }
    TypeId target = reference(c.target) ? sem.types[c.target].child : c.target;
    if (!construction || sem.direct_transfer(materialized.constructor)) {
        Value source = construction ? converted(sem.call_arguments[materialized.call.arguments],sem.conversion_fact(materialized.call.conversions)) : address(expression(n,true));
        if (!sem.empty_class(target)) {
            Instruction copy(Opcode::CopyObject); copy.bytes = sem.object_size(target); copy.alignment = sem.object_alignment(target);
            emit(copy,{source.operand,destination.operand});
        }
        return;
    }
    std::size_t begin = call_work.size();
    call_work.push_back(Operand::symbol(symbol(materialized.constructor))); call_work.push_back(destination.operand);
    for (unsigned j = 0; j < materialized.call.argument_count; ++j)
        call_work.push_back(converted(sem.call_arguments[materialized.call.arguments+j],sem.conversion_fact(materialized.call.conversions+j)).operand);
    Instruction transfer(Opcode::Call,IRType::Void); transfer.copy_elision = materialized.elision_permission;
    guarded_call(transfer,call_work.data()+begin,call_work.size()-begin);
    call_work.resize(begin);
}
} }
