#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::x86_lane_call(NodeId n, const X86Builtin& d)
{
    auto fact = sem.expression_fact(n);
    auto arg = [&](unsigned j) { return converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j)); };
    auto a = arg(0), b = arg(1);
    auto index = [&](std::uint64_t x) { return Value(Operand::integer(x),IRType::I64); };
    if (d.form == X86Form::Extract) return convert(vector_read(a,coerce(b,IRType::I64)),fact.type);
    Value result(Operand::slot(builder->add_slot(0,type(fact.type))),type(fact.type),fact.type,true);
    if (d.form == X86Form::Insert) {
        auto i = arg(2); store(a,result);
        vector_write(result,coerce(i,IRType::I64),convert(b,sem.types[fact.type].child));
    } else if (d.form == X86Form::ByteShift) {
        // GCC's byte-shift builtins express the immediate in bits. Discard
        // the bottom three bits, as the target byte instruction does.
        auto count = coerce(b,IRType::I64,true);
        auto safe = emit(Opcode::Binary,IRType::I64,{count.operand,Operand::integer(120)},Operation::And);
        auto fits = emit(Opcode::Compare,IRType::I64,{count.operand,Operand::integer(128)},Operation::Ult);
        auto mask = coerce(fits,IRType::I128,true);
        mask = emit(Opcode::Unary,IRType::I128,{mask.operand},Operation::Neg);
        a.address = true;
        auto bits = emit(Opcode::Load,IRType::I128,{address(a).operand});
        bits = emit(Opcode::Binary,IRType::I128,{bits.operand,safe.operand},d.predicate ? Operation::Ushr : Operation::Shl);
        bits = emit(Opcode::Binary,IRType::I128,{bits.operand,mask.operand},Operation::And);
        emit(Opcode::Store,IRType::I128,{bits.operand,address(result).operand});
    } else {
        auto control = d.form == X86Form::Shuffle ? arg(2) : b;
        control = coerce(control,IRType::I64,true);
        auto lane = sem.types[fact.type].child;
        unsigned count = sem.object_size(fact.type)/sem.object_size(lane);
        for (unsigned i=0; i<count; ++i) {
            auto source = d.form == X86Form::Shuffle && i >= count/2 ? b : a;
            bool retained = d.form == X86Form::ShuffleOne && count == 8 &&
                ((d.predicate == 1 && i < 4) || (d.predicate == 2 && i >= 4));
            Value select = index(i);
            if (!retained) {
                unsigned shift = d.form == X86Form::Shuffle && d.predicate == 1 ? i : (i%4)*2;
                select = emit(Opcode::Binary,IRType::I64,{control.operand,Operand::integer(shift)},Operation::Ushr);
                select = emit(Opcode::Binary,IRType::I64,{select.operand,Operand::integer(d.form == X86Form::Shuffle && d.predicate == 1 ? 1 : 3)},Operation::And);
                if (d.form == X86Form::ShuffleOne && d.predicate == 1)
                    select = emit(Opcode::Binary,IRType::I64,{select.operand,Operand::integer(4)},Operation::Add);
            }
            vector_write(result,i,vector_read(source,select));
        }
    }
    result.address = false; return result;
}
} }
