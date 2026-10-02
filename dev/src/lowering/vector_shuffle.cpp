#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::vector_shuffle(NodeId n)
{
    auto fact = sem.expression_fact(n);
    auto arg = [&](unsigned j) { return converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j)); };
    auto a = arg(0), b = fact.argument_count == 3 ? arg(1) : a;
    auto mask = arg(fact.argument_count-1);
    auto t = sem.types[fact.type];
    auto count = t.kind == TypeKind::ExtVector ? t.bound : t.bound/sem.object_size(t.child);
    Value result(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
    zero_object(fact.type,address(result));
    vector_each(count,[&](Value lane) {
        auto index = vector_read(mask,lane);
        index = coerce(index,IRType::I64,true);
        index = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(count*(fact.argument_count-1))},Operation::Umod);
        if (fact.argument_count == 2) vector_write(result,lane,vector_read(a,index));
        else {
            auto first = block(), second = block(), done = block();
            auto low = emit(Opcode::Compare,IRType::I64,{index.operand,Operand::integer(count)},Operation::Ult);
            emit(Opcode::Branch,IRType(),{low.operand,Operand::label(first),Operand::label(second)});
            start(first); vector_write(result,lane,vector_read(a,index)); jump(done);
            start(second);
            index = emit(Opcode::Binary,IRType::I64,{index.operand,Operand::integer(count)},Operation::Sub);
            vector_write(result,lane,vector_read(b,index)); jump(done); start(done);
        }
    });
    result.address = false; return result;
}
} }
