#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
namespace {
// State is local to one cast: source type/address, target type, unique downcast
// and crosscast results with ambiguity flags, and public reachability flags.
// Repeated paths to one virtual subobject have the same address and coalesce.
void candidate(RuntimeBody& b, Operand state, unsigned field, Operand object)
{
    auto old = b.load(state,field);
    auto empty = b.block(), occupied = b.block(), different = b.block(), done = b.block();
    b.branch(b.compare(old,Operand::null()),empty,occupied);
    b.start(empty); b.store(state,field,object); b.jump(done);
    b.start(occupied); b.branch(b.compare(old,object),done,different);
    b.start(different); b.store(state,field+8,Operand::integer(1),Type::I64); b.jump(done);
    b.start(done);
}
FunctionId walk(RuntimeProgram& r)
{
    auto f = r.function(r.symbol("",SR_NONE),Type::I32);
    RuntimeBody b(r,f);
    auto type = b.parameter(Type::Ptr), object = b.parameter(Type::Ptr);
    auto public_path = b.parameter(Type::I32), state = b.parameter(Type::Ptr);
    auto found = Operand::slot(b.builder.add_slot(0,Type::I32));
    auto index = Operand::slot(b.builder.add_slot(0,Type::I32));
    auto displacement = Operand::slot(b.builder.add_slot(0,Type::I64));
    auto entry = b.block(), same = b.block(), source = b.block(), kind = b.block();
    auto single = b.block(), multiple = b.block(), header = b.block(), edge = b.block(), virt = b.block(), child = b.block();
    auto accumulated = b.block(), result = b.block(), target = b.block(), down = b.block(), cross = b.block(), public_target = b.block(), done = b.block();
    b.start(entry); b.store(found,0,Operand::integer(0),Type::I32);
    b.branch(b.compare(type,b.load(state,0)),same,kind);
    b.start(same); b.branch(b.compare(object,b.load(state,8)),source,kind);
    b.start(source); b.store(found,0,Operand::integer(3),Type::I32);
    // OR rather than overwrite: a virtual subobject can be reached along both
    // a private and a public path from the most-derived object.
    auto was_public = b.load(state,56,Type::I32);
    auto now_public = b.emit(Opcode::Binary,Type::I32,{was_public,public_path},Operation::Or);
    b.store(state,56,now_public,Type::I32); b.jump(kind);
    b.start(kind);
    auto vptr = b.load(type);
    auto si = b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[1])}),16);
    b.branch(b.compare(vptr,si),single,multiple);
    b.start(single);
    auto single_found = b.call(f,{b.load(type,16),object,public_path,state});
    b.store(found,0,b.emit(Opcode::Binary,Type::I32,{b.load(found,0,Type::I32),single_found},Operation::Or),Type::I32);
    b.jump(result);
    b.start(multiple);
    auto vmi = b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[2])}),16);
    auto init = b.block(); b.branch(b.compare(vptr,vmi),init,result);
    b.start(init); b.store(index,0,Operand::integer(0),Type::I32); b.jump(header);
    b.start(header);
    auto n = b.load(index,0,Type::I32), count = b.load(type,20,Type::I32);
    b.branch(b.compare(n,count,Operation::Ult,Type::I32),edge,result);
    b.start(edge);
    auto bytes = b.emit(Opcode::Binary,Type::I64,{n,Operand::integer(16)},Operation::Mul);
    auto record = b.emit(Opcode::Index,Type::I8,{b.offset(type,24),bytes});
    auto base_type = b.load(record), flags = b.load(record,8,Type::I64);
    auto off = b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(8)},Operation::Shr);
    b.store(displacement,0,off,Type::I64);
    auto is_virtual = b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(1)},Operation::And);
    b.branch(b.compare(is_virtual,Operand::integer(0),Operation::Ne,Type::I64),virt,child);
    b.start(virt);
    auto virtual_row = b.emit(Opcode::Index,Type::I8,{b.load(object),off});
    b.store(displacement,0,b.load(virtual_row,0,Type::I64),Type::I64); b.jump(child);
    b.start(child);
    auto address = b.emit(Opcode::Index,Type::I8,{object,b.load(displacement,0,Type::I64)});
    auto access = b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(2)},Operation::And);
    auto pub = b.compare(access,Operand::integer(0),Operation::Ne,Type::I64);
    auto path = b.emit(Opcode::Binary,Type::I32,{public_path,pub},Operation::And);
    auto child_found = b.call(f,{base_type,address,path,state});
    auto public_bit = b.emit(Opcode::Binary,Type::I32,{pub,Operand::integer(1)},Operation::Shl);
    auto mask = b.emit(Opcode::Binary,Type::I32,{public_bit,Operand::integer(1)},Operation::Or);
    auto accessible = b.emit(Opcode::Binary,Type::I32,{child_found,mask},Operation::And);
    b.store(found,0,b.emit(Opcode::Binary,Type::I32,{b.load(found,0,Type::I32),accessible},Operation::Or),Type::I32);
    b.jump(accumulated);
    b.start(accumulated);
    b.store(index,0,b.emit(Opcode::Binary,Type::I32,{b.load(index,0,Type::I32),Operand::integer(1)},Operation::Add),Type::I32);
    b.jump(header);
    b.start(result); b.branch(b.compare(type,b.load(state,16)),target,done);
    b.start(target); b.branch(b.compare(b.load(found,0,Type::I32),Operand::integer(0),Operation::Ne,Type::I32),down,cross);
    b.start(down); candidate(b,state,24,object);
    auto public_source = b.emit(Opcode::Binary,Type::I32,{b.load(found,0,Type::I32),Operand::integer(1)},Operation::Ushr);
    b.store(state,64,b.emit(Opcode::Binary,Type::I32,{b.load(state,64,Type::I32),public_source},Operation::Or),Type::I32);
    b.jump(cross);
    b.start(cross); candidate(b,state,40,object); b.jump(public_target);
    b.start(public_target);
    b.store(state,60,b.emit(Opcode::Binary,Type::I32,{b.load(state,60,Type::I32),public_path},Operation::Or),Type::I32);
    b.jump(done);
    b.start(done); b.emit(Opcode::Return,Type::I32,{b.load(found,0,Type::I32)});
    return f;
}
}
void build_dynamic_cast(RuntimeProgram& r, SymbolId symbol)
{
    auto walker = walk(r);
    auto f = r.function(symbol,Type::Ptr); RuntimeBody b(r,f);
    auto source = b.parameter(Type::Ptr), from = b.parameter(Type::Ptr), to = b.parameter(Type::Ptr);
    b.parameter(Type::I64); // ABI hint is an optional optimization, never a proof.
    auto state = Operand::slot(b.builder.add_slot(0,Type::object(80,8)));
    auto entry = b.block(), scan = b.block(), down = b.block(), cross = b.block(), cross_unique = b.block(), good = b.block(), fail = b.block();
    b.start(entry); b.branch(b.compare(source,Operand::null()),fail,scan);
    b.start(scan);
    auto state_address = b.emit(Opcode::Addr,Type(),{state});
    b.store(state_address,0,from); b.store(state_address,8,source); b.store(state_address,16,to);
    for (unsigned i = 24; i < 80; i += 8) b.store(state_address,i,Operand::integer(0),Type::I64);
    auto vptr = b.load(source);
    auto complete = b.emit(Opcode::Index,Type::I8,{source,b.load(vptr,-16,Type::I64)});
    b.call(walker,{b.load(vptr,-8),complete,Operand::integer(1),state_address});
    auto down_result = b.load(state_address,24);
    b.branch(b.compare(down_result,Operand::null()),cross,down);
    b.start(down); auto unique = b.block();
    b.branch(b.compare(b.load(state_address,32,Type::I64),Operand::integer(0),Operation::Eq,Type::I64),unique,cross);
    b.start(unique); auto accessible_down = b.block();
    b.branch(b.compare(b.load(state_address,64,Type::I32),Operand::integer(0),Operation::Ne,Type::I32),accessible_down,cross);
    b.start(accessible_down); b.emit(Opcode::Return,Type::Ptr,{down_result});
    b.start(cross); b.branch(b.compare(b.load(state_address,56,Type::I32),Operand::integer(0),Operation::Ne,Type::I32),cross_unique,fail);
    b.start(cross_unique); b.branch(b.compare(b.load(state_address,48,Type::I64),Operand::integer(0),Operation::Eq,Type::I64),good,fail);
    b.start(good); auto accessible_cross = b.block();
    b.branch(b.compare(b.load(state_address,60,Type::I32),Operand::integer(0),Operation::Ne,Type::I32),accessible_cross,fail);
    b.start(accessible_cross); b.emit(Opcode::Return,Type::Ptr,{b.load(state_address,40)});
    b.start(fail); b.emit(Opcode::Return,Type::Ptr,{Operand::null()});
}
} }
