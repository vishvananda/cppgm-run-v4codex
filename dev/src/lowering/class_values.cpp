#include "lowering/procedural.h"
#include <stdexcept>
namespace cppgm { namespace lowering {
using syntax::Kind;
IRType Procedural::result_type() const
{ return p.signatures[p.functions[function.index-1].signature.index-1].result; }
Value Procedural::class_temporary(EntityId object, TypeId t)
{
    if (!object) throw std::logic_error("missing class value storage identity");
    if (!objects[object] || p.slots[objects[object].index-1].owner.index != function.index)
        objects[object] = builder->add_slot(0,type(t));
    return Value(Operand::slot(objects[object]),type(t),t,true);
}
void Procedural::construct_value(NodeId n, const semantic::Conversion& c, Value destination)
{
    while (ast[n].kind == Kind::Parenthesized) n = ast[n].first;
    auto fact = sem.expression_fact(n);
    bool construction = c.kind == semantic::Conversion::Kind::Construction;
    auto materialized = construction ? sem.conversion_objects[c.materialization] : semantic::ConversionObject();
    bool elided = construction ? materialized.elided : c.empty_copy && fact.category == ValueCategory::Prvalue;
    if (elided && fact.form == semantic::ExpressionForm::Construction) {
        construct(sem.facts[n].entity,n,destination); return;
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
    guarded_call(Instruction(Opcode::Call,IRType::Void),call_work.data()+begin,call_work.size()-begin);
    call_work.resize(begin);
}
} }
