#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
namespace {
// An allocation owns this fixed header and its payload until the last handler
// exits. The signed handler count preserves ownership during rethrow.
const int header_bytes = 64;
enum Field { TypeInfo=0, Destructor=8, Adjusted=16, PreviousCatch=24, Handlers=32, PreviousException=40 };
FunctionId primitive(RuntimeProgram& r, SymbolRole role, Type result, bool parameter)
{
    auto symbol = r.symbol("",SR_NONE); auto f = r.function(symbol,result);
    RuntimeBody b(r,f); if (parameter) b.parameter(role == SR_ALLOCATE_MEMORY ? Type::I64 : Type::Ptr);
    r.p.functions[f.index-1].declaration = true; r.primitives.push_back({symbol,role}); return f;
}
Operand state(RuntimeProgram& r, native::RuntimeEntity e) { return Operand::symbol(r.state(e)); }

}
void build_exceptions(RuntimeProgram& r, const std::vector<RuntimeRequest>& requests)
{
    auto allocate = primitive(r,SR_ALLOCATE_MEMORY,Type::Ptr,true);
    auto release = primitive(r,SR_FREE_MEMORY,Type(),true);
    auto terminate = primitive(r,SR_TERMINATE,Type(),false);
    bool needed[5] = {}, failures = false, free_needed = false;
    for (const auto& request : requests) {
        if (request.role >= SR_EH_ALLOCATE_EXCEPTION && request.role <= SR_EH_THROW)
            needed[request.role-SR_EH_ALLOCATE_EXCEPTION] = true;
        failures |= request.role == SR_BAD_CAST || request.role == SR_BAD_TYPEID || request.role == SR_ALLOCATE_MEMORY;
        free_needed |= request.role == SR_EH_FREE_EXCEPTION;
    }
    if (failures) needed[0] = needed[4] = true;
    FunctionId match;
    if (needed[4]) match = build_exception_match(r,runtime_typeinfo(r,"v"));
    FunctionId allocation_function, throw_function;
    const char* names[] = {"__cxa_allocate_exception","__cxa_begin_catch","__cxa_end_catch","__cxa_rethrow","__cxa_throw"};
    for (unsigned k=0;k<5;++k) {
        if (!needed[k]) continue;
        auto role = SymbolRole(SR_EH_ALLOCATE_EXCEPTION+k);
        std::string name = names[k];
        for (const auto& request : requests) if (request.role == role) name = request.name;
        auto f = r.function(r.symbol(name,role),role <= SR_EH_BEGIN_CATCH ? Type::Ptr : Type());
        if (!k) allocation_function=f;
        if (k==4) throw_function=f;
        RuntimeBody b(r,f);
        auto current = state(r,native::RuntimeEntity::ExceptionValue);
        auto caught = state(r,native::RuntimeEntity::ExceptionCaught);
        if (role == SR_EH_ALLOCATE_EXCEPTION) {
            auto size = b.parameter(Type::I64); b.start(b.block());
            auto bytes = b.emit(Opcode::Binary,Type::I64,{size,Operand::integer(header_bytes)},Operation::Add);
            auto valid = b.block(), invalid = b.block(); b.branch(b.compare(bytes,size,Operation::Ult,Type::I64),invalid,valid);
            b.start(invalid); b.call(terminate,{}); b.emit(Opcode::Unreachable,Type(),{});
            b.start(valid); auto header = b.call(allocate,{bytes});
            for (unsigned i = 0; i < header_bytes; i += 8) b.store(header,i,Operand::integer(0),Type::I64);
            b.emit(Opcode::Return,Type::Ptr,{b.offset(header,header_bytes)});
        } else if (role == SR_EH_THROW) {
            auto object = b.parameter(Type::Ptr), type = b.parameter(Type::Ptr), destructor = b.parameter(Type::Ptr);
            b.start(b.block()); auto header = b.offset(object,-header_bytes);
            b.store(header,TypeInfo,type); b.store(header,Destructor,destructor); b.store(header,Adjusted,object);
            b.store(header,PreviousException,b.load(current));
            b.store(header,48,b.load(state(r,native::RuntimeEntity::ExceptionSelector),0,Type::I32),Type::I32);
            b.store(state(r,native::RuntimeEntity::ExceptionMatcher),0,b.emit(Opcode::Addr,Type(),{Operand::symbol(r.p.functions[match.index-1].symbol)}));
            b.emit(Opcode::Throw,Type::Ptr,{object});
        } else if (role == SR_EH_BEGIN_CATCH) {
            auto object = b.parameter(Type::Ptr); b.start(b.block()); auto header = b.offset(object,-header_bytes);
            auto count = b.load(header,Handlers,Type::I64);
            auto negative = b.block(), positive = b.block(), link = b.block(), push = b.block(), done = b.block();
            b.branch(b.compare(count,Operand::integer(0),Operation::Lt,Type::I64),negative,positive);
            b.start(negative); auto neg = b.emit(Opcode::Unary,Type::I64,{count},Operation::Neg);
            b.store(header,Handlers,b.emit(Opcode::Binary,Type::I64,{neg,Operand::integer(1)},Operation::Add),Type::I64); b.jump(link);
            b.start(positive); b.store(header,Handlers,b.emit(Opcode::Binary,Type::I64,{count,Operand::integer(1)},Operation::Add),Type::I64); b.jump(link);
            b.start(link); auto previous = b.load(caught); b.branch(b.compare(previous,object),done,push);
            b.start(push); b.store(header,PreviousCatch,previous); b.store(caught,0,object); b.jump(done);
            b.start(done); b.emit(Opcode::Return,Type::Ptr,{b.load(header,Adjusted)});
        } else if (role == SR_EH_RETHROW) {
            b.start(b.block()); auto object = b.load(caught);
            auto valid = b.block(), invalid = b.block(); b.branch(b.compare(object,Operand::null()),invalid,valid);
            b.start(invalid); b.call(terminate,{}); b.emit(Opcode::Unreachable,Type(),{});
            b.start(valid); auto header = b.offset(object,-header_bytes);
            b.store(header,Handlers,b.emit(Opcode::Unary,Type::I64,{b.load(header,Handlers,Type::I64)},Operation::Neg),Type::I64);
            b.emit(Opcode::Throw,Type::Ptr,{object});
        } else {
            b.start(b.block()); auto object = b.load(caught);
            auto valid = b.block(), done = b.block(); b.branch(b.compare(object,Operand::null()),done,valid);
            b.start(valid); auto header = b.offset(object,-header_bytes), count = b.load(header,Handlers,Type::I64);
            auto negative = b.block(), positive = b.block(), pop = b.block(), destroy = b.block(), free = b.block(), restore = b.block();
            b.branch(b.compare(count,Operand::integer(0),Operation::Lt,Type::I64),negative,positive);
            b.start(negative); auto remaining = b.emit(Opcode::Binary,Type::I64,{count,Operand::integer(1)},Operation::Add);
            b.store(header,Handlers,remaining,Type::I64); b.branch(b.compare(remaining,Operand::integer(0),Operation::Eq,Type::I64),pop,done);
            b.start(pop); b.store(caught,0,b.load(header,PreviousCatch)); b.jump(done);
            b.start(positive); auto left = b.emit(Opcode::Binary,Type::I64,{count,Operand::integer(1)},Operation::Sub);
            b.store(header,Handlers,left,Type::I64); b.branch(b.compare(left,Operand::integer(0),Operation::Eq,Type::I64),destroy,done);
            b.start(destroy); b.store(caught,0,b.load(header,PreviousCatch));
            auto destructor = b.load(header,Destructor);
            auto call = b.block(); b.branch(b.compare(destructor,Operand::null()),free,call);
            b.start(call); auto failed = b.block();
            b.emit(Opcode::EhTry,Type(),{Operand::label(failed)});
            auto sig = r.p.functions[release.index-1].signature;
            Instruction invoke(Opcode::Call,Type()); invoke.signature = sig;
            b.builder.append(invoke,{destructor,object}); b.emit(Opcode::EhEnd,Type(),{}); b.jump(free);
            b.start(failed); b.emit(Opcode::EhCatchAll,Type(),{Operand::integer(1)});
            b.call(terminate,{}); b.emit(Opcode::Unreachable,Type(),{});
            b.start(free); auto finish = b.block(); b.branch(b.compare(b.load(current),object),restore,finish);
            b.start(restore); b.store(current,0,b.load(header,PreviousException));
            b.store(state(r,native::RuntimeEntity::ExceptionSelector),0,b.load(header,48,Type::I32),Type::I32); b.jump(finish);
            b.start(finish); b.call(release,{header}); b.jump(done);
            b.start(done); b.emit(Opcode::Return,Type(),{});
        }
    }
    if (free_needed) {
    auto free_exception = r.function(r.symbol("__cxa_free_exception",SR_EH_FREE_EXCEPTION),Type());
    { RuntimeBody b(r,free_exception); auto object=b.parameter(Type::Ptr); b.start(b.block());
      b.call(release,{b.offset(object,-header_bytes)}); b.emit(Opcode::Return,Type(),{}); }
    }
    if (failures) build_failures(r,allocation_function,throw_function,requests);
}
} }
