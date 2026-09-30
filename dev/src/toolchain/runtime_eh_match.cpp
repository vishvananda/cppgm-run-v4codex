#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
namespace {
FunctionId hierarchy(RuntimeProgram& r)
{
    auto f=r.function(r.symbol("",SR_NONE),Type()); RuntimeBody b(r,f);
    auto type=b.parameter(Type::Ptr), object=b.parameter(Type::Ptr), anchor=b.parameter(Type::Ptr);
    auto pub=b.parameter(Type::I32), state=b.parameter(Type::Ptr), null=b.parameter(Type::I32);
    auto index=Operand::slot(b.builder.add_slot(0,Type::I32));
    auto offset=Operand::slot(b.builder.add_slot(0,Type::I64));
    auto child_anchor=Operand::slot(b.builder.add_slot(0,Type::Ptr));
    auto entry=b.block(), candidate=b.block(), first=b.block(), repeat=b.block(), distinct=b.block(), access=b.block(), kind=b.block();
    auto single=b.block(), multiple=b.block(), init=b.block(), header=b.block(), edge=b.block(), virt=b.block(), real=b.block(), symbolic=b.block(), child=b.block(), done=b.block();
    b.start(entry); b.branch(b.compare(type,b.load(state)),candidate,kind);
    b.start(candidate); b.branch(b.compare(b.load(state,8,Type::I32),Operand::integer(0),Operation::Eq,Type::I32),first,repeat);
    b.start(first); b.store(state,8,Operand::integer(1),Type::I32); b.store(state,24,object); b.store(state,32,anchor); b.jump(access);
    b.start(repeat);
    auto same=b.emit(Opcode::Binary,Type::I32,{b.compare(object,b.load(state,24)),b.compare(anchor,b.load(state,32))},Operation::And);
    b.branch(same,access,distinct);
    b.start(distinct); b.store(state,12,Operand::integer(1),Type::I32); b.jump(access);
    b.start(access); b.store(state,16,b.emit(Opcode::Binary,Type::I32,{b.load(state,16,Type::I32),pub},Operation::Or),Type::I32); b.jump(kind);
    b.start(kind); auto vptr=b.load(type);
    auto si=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[1])}),16);
    b.branch(b.compare(vptr,si),single,multiple);
    b.start(single); b.call(f,{b.load(type,16),object,anchor,pub,state,null}); b.jump(done);
    b.start(multiple); auto vmi=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[2])}),16);
    b.branch(b.compare(vptr,vmi),init,done);
    b.start(init); b.store(index,0,Operand::integer(0),Type::I32); b.jump(header);
    b.start(header); auto n=b.load(index,0,Type::I32);
    b.branch(b.compare(n,b.load(type,20,Type::I32),Operation::Ult,Type::I32),edge,done);
    b.start(edge); auto bytes=b.emit(Opcode::Binary,Type::I64,{n,Operand::integer(16)},Operation::Mul);
    auto record=b.emit(Opcode::Index,Type::I8,{b.offset(type,24),bytes});
    auto base=b.load(record), flags=b.load(record,8,Type::I64);
    auto off=b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(8)},Operation::Shr);
    b.store(offset,0,off,Type::I64); b.store(child_anchor,0,anchor);
    auto isvirtual=b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(1)},Operation::And);
    b.branch(b.compare(isvirtual,Operand::integer(0),Operation::Ne,Type::I64),virt,child);
    b.start(virt); b.branch(null,symbolic,real);
    b.start(real); auto row=b.emit(Opcode::Index,Type::I8,{b.load(object),off});
    b.store(offset,0,b.load(row,0,Type::I64),Type::I64); b.jump(child);
    b.start(symbolic); b.store(child_anchor,0,base);
    // A virtual subobject has a canonical (type, offset-within-virtual-base)
    // identity even when a null pointer prevents reading an object vtable.
    auto integer=b.emit(Opcode::Copy,Type::I64,{object});
    auto neg=b.emit(Opcode::Unary,Type::I64,{integer},Operation::Neg); b.store(offset,0,neg,Type::I64); b.jump(child);
    b.start(child); auto at=b.emit(Opcode::Index,Type::I8,{object,b.load(offset,0,Type::I64)});
    auto public_edge=b.compare(b.emit(Opcode::Binary,Type::I64,{flags,Operand::integer(2)},Operation::And),Operand::integer(0),Operation::Ne,Type::I64);
    auto path=b.emit(Opcode::Binary,Type::I32,{pub,public_edge},Operation::And);
    b.call(f,{base,at,b.load(child_anchor),path,state,null});
    b.store(index,0,b.emit(Opcode::Binary,Type::I32,{b.load(index,0,Type::I32),Operand::integer(1)},Operation::Add),Type::I32); b.jump(header);
    b.start(done); b.emit(Opcode::Return,Type(),{}); return f;
}
FunctionId conversion(RuntimeProgram& r, SymbolId void_type)
{
    auto walk=hierarchy(r);
    auto f=r.function(r.symbol("",SR_NONE),Type::I32); RuntimeBody b(r,f);
    auto from=b.parameter(Type::Ptr), to=b.parameter(Type::Ptr), object=b.parameter(Type::Ptr), out=b.parameter(Type::Ptr);
    auto depth=b.parameter(Type::I32), allowed=b.parameter(Type::I32);
    auto state=Operand::slot(b.builder.add_slot(0,Type::object(48,8)));
    auto entry=b.block(), equal=b.block(), kind=b.block(), pointer=b.block(), qualifiers=b.block(), recurse=b.block(), scalar=b.block(), classes=b.block(), success=b.block(), fail=b.block();
    b.start(entry); b.branch(b.compare(from,to),equal,kind);
    b.start(equal); b.store(out,0,object); b.emit(Opcode::Return,Type::I32,{Operand::integer(1)});
    b.start(kind); auto ptrtable=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[3])}),16);
    auto both=b.emit(Opcode::Binary,Type::I32,{b.compare(b.load(from),ptrtable),b.compare(b.load(to),ptrtable)},Operation::And);
    b.branch(both,pointer,scalar);
    b.start(pointer); auto ff=b.load(from,16,Type::I32), tf=b.load(to,16,Type::I32);
    auto removed=b.emit(Opcode::Binary,Type::I32,{ff,b.emit(Opcode::Unary,Type::I32,{tf},Operation::Bitnot)},Operation::And);
    b.branch(b.compare(removed,Operand::integer(0),Operation::Eq,Type::I32),qualifiers,fail);
    b.start(qualifiers); auto permitted=b.emit(Opcode::Binary,Type::I32,{allowed,b.compare(ff,tf,Operation::Eq,Type::I32)},Operation::Or);
    b.branch(permitted,recurse,fail);
    b.start(recurse); auto can_add=b.emit(Opcode::Binary,Type::I32,{allowed,b.emit(Opcode::Binary,Type::I32,{tf,Operand::integer(1)},Operation::And)},Operation::And);
    auto next=b.emit(Opcode::Binary,Type::I32,{depth,Operand::integer(1)},Operation::Add);
    auto result=b.call(f,{b.load(from,24),b.load(to,24),object,out,next,can_add}); b.emit(Opcode::Return,Type::I32,{result});
    b.start(scalar); auto shallow=b.compare(depth,Operand::integer(1),Operation::Ule,Type::I32);
    auto first=b.block(); b.branch(shallow,first,fail);
    b.start(first); auto isvoid=b.compare(to,b.emit(Opcode::Addr,Type(),{Operand::symbol(void_type)}));
    auto pointer_depth=b.compare(depth,Operand::integer(1),Operation::Eq,Type::I32);
    auto function_table=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[4])}),16);
    auto object_type=b.compare(b.load(from),function_table,Operation::Ne);
    auto void_conversion=b.emit(Opcode::Binary,Type::I32,{isvoid,pointer_depth},Operation::And);
    void_conversion=b.emit(Opcode::Binary,Type::I32,{void_conversion,object_type},Operation::And);
    b.branch(void_conversion,equal,classes);
    b.start(classes); auto address=b.emit(Opcode::Addr,Type(),{state});
    for(unsigned i=0;i<48;i+=8)b.store(address,i,Operand::integer(0),Type::I64);
    b.store(address,0,to); auto null=b.compare(object,Operand::null());
    b.call(walk,{from,object,from,Operand::integer(1),address,null});
    auto seen=b.load(address,8,Type::I32), ambiguous=b.load(address,12,Type::I32), pub=b.load(address,16,Type::I32);
    auto valid=b.emit(Opcode::Binary,Type::I32,{seen,pub},Operation::And);
    valid=b.emit(Opcode::Binary,Type::I32,{valid,b.compare(ambiguous,Operand::integer(0),Operation::Eq,Type::I32)},Operation::And);
    b.branch(valid,success,fail);
    b.start(success); auto normal=b.block(); b.branch(null,equal,normal);
    b.start(normal); b.store(out,0,b.load(address,24)); b.emit(Opcode::Return,Type::I32,{Operand::integer(1)});
    b.start(fail); b.emit(Opcode::Return,Type::I32,{Operand::integer(0)}); return f;
}
}
FunctionId build_exception_match(RuntimeProgram& r, SymbolId void_type)
{
    auto convert=conversion(r,void_type);
    auto nullptr_type=runtime_typeinfo(r,"Dn");
    auto f=r.function(r.symbol("",SR_NONE),Type::I32); RuntimeBody b(r,f);
    auto target=b.parameter(Type::Ptr);
    auto entry=b.block(), pointer=b.block(), ordinary=b.block();
    b.start(entry); auto object=b.load(Operand::symbol(r.state(native::RuntimeEntity::ExceptionValue)));
    auto header=b.offset(object,-ExceptionHeaderBytes), type=b.load(header);
    auto table=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[3])}),16);
    auto null_pointer=b.block(), typed=b.block(), member=b.block(), null_value=b.block(), null_member=b.block();
    b.branch(b.compare(type,b.emit(Opcode::Addr,Type(),{Operand::symbol(nullptr_type)})),null_pointer,typed);
    b.start(null_pointer); b.branch(b.compare(b.load(target),table),null_value,member);
    b.start(null_value); b.store(header,16,Operand::null());b.emit(Opcode::Return,Type::I32,{Operand::integer(1)});
    b.start(member);auto member_table=b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(r.tables[5])}),16);
    b.branch(b.compare(b.load(target),member_table),null_member,ordinary);
    b.start(null_member);
    // PA25's source member-pointer representation uses zero for null in both
    // lanes (data-member offsets are biased by one). Host ABI conversion is
    // a later object-boundary responsibility.
    b.store(header,64,Operand::integer(0),Type::I64);b.store(header,72,Operand::integer(0),Type::I64);
    b.store(header,16,b.offset(header,64));b.emit(Opcode::Return,Type::I32,{Operand::integer(1)});
    b.start(typed);b.branch(b.compare(b.load(type),table),pointer,ordinary);
    b.start(pointer); auto result=b.call(convert,{type,target,b.load(object),b.offset(header,16),Operand::integer(0),Operand::integer(1)});
    b.emit(Opcode::Return,Type::I32,{result});
    b.start(ordinary); result=b.call(convert,{type,target,object,b.offset(header,16),Operand::integer(0),Operand::integer(1)});
    b.emit(Opcode::Return,Type::I32,{result}); return f;
}
} }
