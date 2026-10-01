#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::atomic_call(NodeId n, Value destination)
{
    auto fact = sem.expression_fact(n);
    auto kind = sem.atomic_kind(sem.facts[n].entity);
    auto op = kind.op;
    std::vector<Value> args;
    for (unsigned j = 0; j < fact.argument_count; ++j) {
        // always_lock_free's address is explicitly unevaluated by GNU.
        if (kind.form == AtomicForm::Always && j == 1) break;
        args.push_back(converted(sem.call_argument(fact,j),sem.conversion_fact(fact.conversions+j)));
    }
    auto none = Value(Operand(),IRType::Void,fact.type);
    if (kind.fence()) {
        emit(op == AtomicOp::ThreadFence ? Opcode::AtomicThreadFence : Opcode::AtomicSignalFence,IRType(),{Operand::integer(5)});
        return none;
    }
    if (op == AtomicOp::LockFree) {
        auto bytes = args[0];
        auto smaller = emit(Opcode::Binary,bytes.ir,{bytes.operand,Operand::integer(1)},Operation::Sub);
        auto mask = emit(Opcode::Binary,bytes.ir,{bytes.operand,smaller.operand},Operation::And);
        auto power = emit(Opcode::Compare,bytes.ir,{mask.operand,Operand::integer(0)},Operation::Eq);
        auto bound = emit(Opcode::Compare,bytes.ir,{smaller.operand,Operand::integer(kind.form == AtomicForm::C11 ? 16 : 8)},Operation::Ult);
        auto result = emit(Opcode::Binary,IRType::I64,{power.operand,bound.operand},Operation::And);
        result.type = sem.types.fundamental(FT_LONG_INT); return convert(result,fact.type);
    }
    auto target = sem.types[args[0].type].child;
    auto source = sem.expression_fact(sem.call_argument(fact,0));
    if (source.storage_type) target = sem.types[source.storage_type].child;
    auto bytes = (op == AtomicOp::TestSet || (op == AtomicOp::Clear && !kind.sync())) ? 1 : sem.object_size(target);
    bool native = bytes == 1 || inline_atomic(target);
    if ((kind.form == AtomicForm::Generic || kind.form == AtomicForm::C11) && !inline_atomic(target)) {
        auto buffer = [&](unsigned j) { return kind.form == AtomicForm::Generic ? args[j].operand : atomic_buffer(args[j]); };
        if (op == AtomicOp::Store || op == AtomicOp::Init) {
            atomic_runtime(AtomicOp::Store,bytes,args[0].operand,buffer(1)); return none;
        }
        if (op == AtomicOp::Compare) {
            auto result = atomic_runtime(op,bytes,args[0].operand,buffer(2),args[1].operand);
            result.type = fact.type; return result;
        }
        bool own = kind.form != AtomicForm::Generic && destination.ir == IRType::Void;
        if (kind.form == AtomicForm::Generic) destination = args[op == AtomicOp::Load ? 1 : 2];
        else if (own) destination = class_address(sem.object_fact(n).temporary,fact.type);
        atomic_runtime(op,bytes,args[0].operand,op == AtomicOp::Load ? destination.operand : buffer(1),destination.operand);
        if (kind.form == AtomicForm::Generic) return none;
        if (own) activate_temporary(sem.object_fact(n).temporary);
        destination.type = fact.type; destination.address = true; return destination;
    }
    // All operations consume the same integer object representation in LowIR.
    IRType raw = bytes == 1 ? IRType::U8 : bytes == 2 ? IRType::U16 : bytes == 4 ? IRType::U32 : bytes == 8 ? IRType::I64 : IRType::I128;
    auto primitive = [&](AtomicOp action, Operand object, Operand value, Operand expected) {
        return atomic_scalar(action,raw,object,value,expected,native);
    };
    auto bits = [&](Value value, bool pointer) {
        return pointer ? emit(Opcode::Load,raw,{value.operand}) : atomic_bits(value,raw);
    };
    auto result = [&](Value value) {
        if (!sem.class_value(fact.type)) return atomic_value(value,fact.type);
        bool own = destination.ir == IRType::Void;
        if (own) destination = class_address(sem.object_fact(n).temporary,fact.type);
        atomic_extract(value,destination.operand,fact.type);
        if (own) activate_temporary(sem.object_fact(n).temporary);
        destination.type = fact.type; destination.address = true; return destination;
    };
    auto address = args[0].operand;
    if (op == AtomicOp::Load) {
        auto value = primitive(AtomicOp::Load,address,Operand(),Operand());
        if (kind.form != AtomicForm::Generic) return result(value);
        emit(Opcode::Store,raw,{value.operand,args[1].operand}); return none;
    }
    if (op == AtomicOp::Clear || op == AtomicOp::Store || op == AtomicOp::Init) {
        auto value = op == AtomicOp::Clear ? Value(Operand::integer(0),raw) : bits(args[1],kind.form == AtomicForm::Generic);
        if (op == AtomicOp::Init) emit(Opcode::Store,raw,{value.operand,address});
        else primitive(AtomicOp::Store,address,value.operand,Operand());
        return none;
    }
    if (op == AtomicOp::Compare) {
        auto desired = bits(args[2],kind.form == AtomicForm::Generic);
        auto expected = args[1].operand;
        auto plain = sem.types.non_atomic(sem.types.unqualified(target));
        bool padded = kind.form == AtomicForm::C11 && sem.object_size(plain) != bytes;
        if (padded) {
            expected = Operand::slot(builder->add_slot(0,raw));
            auto initial = atomic_bits(Value(args[1].operand,type(plain),plain),raw);
            emit(Opcode::Store,raw,{initial.operand,expected});
        }
        if (kind.sync()) {
            expected = Operand::slot(builder->add_slot(0,raw));
            auto initial = bits(args[1],false); emit(Opcode::Store,raw,{initial.operand,expected});
        }
        auto expected_pointer = this->address(Value(expected,raw,0,true)).operand;
        auto success = primitive(AtomicOp::Compare,address,desired.operand,expected_pointer);
        if (padded) {
            auto done = block(), failed = block();
            emit(Opcode::Branch,IRType(),{success.operand,Operand::label(done),Operand::label(failed)});
            start(failed); atomic_extract(emit(Opcode::Load,raw,{expected}),args[1].operand,plain); jump(done); start(done);
        }
        if (kind.form == AtomicForm::SyncValue) return result(emit(Opcode::Load,raw,{expected}));
        success.type = sem.types.fundamental(FT_LONG_INT); return convert(success,fact.type);
    }
    auto value = op == AtomicOp::TestSet ? Value(Operand::integer(1),raw) : bits(args[1],kind.form == AtomicForm::Generic);
    if (op == AtomicOp::Exchange || op == AtomicOp::TestSet) {
        auto old = primitive(AtomicOp::Exchange,address,value.operand,Operand());
        if (kind.form == AtomicForm::Generic) { emit(Opcode::Store,raw,{old.operand,args[2].operand}); return none; }
        if (op == AtomicOp::TestSet) { old.type = sem.types.fundamental(FT_UNSIGNED_CHAR); return convert(old,fact.type); }
        return result(old);
    }
    if ((op == AtomicOp::Add || op == AtomicOp::Sub) && kind.form == AtomicForm::C11 && sem.types[target].kind == TypeKind::Pointer)
        value = emit(Opcode::Binary,raw,{value.operand,Operand::integer(sem.object_size(sem.types[target].child))},Operation::Mul);
    if ((op == AtomicOp::Add || op == AtomicOp::Sub) && native) {
        if (op == AtomicOp::Sub) value = emit(Opcode::Binary,raw,{Operand::integer(0),value.operand},Operation::Sub);
        auto next = emit(Opcode::AtomicAddFetch,raw,{address,value.operand,Operand::integer(5)});
        if (!kind.updated) next = emit(Opcode::Binary,raw,{next.operand,value.operand},Operation::Sub);
        return result(next);
    }
    // One strong-CAS loop implements remaining RMWs. Arguments are evaluated
    // outside the loop; failure refreshes expected. Compiler work/growth is O(1).
    auto expected = Operand::slot(builder->add_slot(0,raw));
    auto initial = primitive(AtomicOp::Load,address,Operand(),Operand());
    emit(Opcode::Store,raw,{initial.operand,expected});
    auto expected_pointer = this->address(Value(expected,raw,0,true)).operand;
    auto retry = block(), done = block(); jump(retry); start(retry);
    auto prior = emit(Opcode::Load,raw,{expected});
    auto action = op == AtomicOp::Add ? Operation::Add : op == AtomicOp::Sub ? Operation::Sub :
        op == AtomicOp::Or ? Operation::Or : op == AtomicOp::Xor ? Operation::Xor : Operation::And;
    auto next = emit(Opcode::Binary,raw,{prior.operand,value.operand},action);
    if (op == AtomicOp::Nand) next = emit(Opcode::Unary,raw,{next.operand},Operation::Bitnot);
    auto success = primitive(AtomicOp::Compare,address,next.operand,expected_pointer);
    emit(Opcode::Branch,IRType(),{success.operand,Operand::label(done),Operand::label(retry)});
    start(done); return result(kind.updated ? next : prior);
}
} }
