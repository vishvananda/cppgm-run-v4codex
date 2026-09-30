#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
namespace {
SymbolId data(RuntimeProgram& r, const std::string& name, const std::vector<DataItem>& items)
{
    auto id=r.symbol(name,SR_NONE); Global g; g.symbol=id; g.structured=true;
    g.data.begin=r.p.data.size();g.data.count=items.size();
    for (const auto& item:items) r.p.data.push_back(item);r.p.globals.push_back(g);
    auto& s=r.p.symbols[id.index-1];s.kind=lowir_model::Symbol::GlobalSymbol;s.entity=r.p.globals.size();return id;
}
DataItem address(SymbolId symbol, int addend=0)
{
    DataItem d;d.kind=DataItem::Address;d.type=Type::Ptr;d.symbol=symbol;d.addend=addend;return d;
}
DataItem number(unsigned value,Type type=Type::I64)
{
    DataItem d;d.kind=DataItem::Scalar;d.type=type;d.value=Operand::integer(value);return d;
}
SymbolId string(RuntimeProgram& r, const std::string& name, const std::string& value)
{
    std::vector<DataItem> bytes;for(auto c:value)bytes.push_back(number(c,Type::I8));bytes.push_back(number(0,Type::I8));return data(r,name,bytes);
}
}
SymbolId runtime_typeinfo(RuntimeProgram& r, const std::string& encoding, SymbolId base)
{
    auto name=string(r,"_ZTS"+encoding,encoding);
    std::vector<DataItem> items={address(r.tables[base ? 1 : 0],16),address(name)};
    if(base)items.push_back(address(base));return data(r,"_ZTI"+encoding,items);
}
void build_failures(RuntimeProgram& r, FunctionId allocate, FunctionId raise, const std::vector<RuntimeRequest>& requests)
{
    auto exception=runtime_typeinfo(r,"St9exception");
    auto dtor=r.function(r.symbol("",SR_NONE),Type());
    {RuntimeBody b(r,dtor);b.parameter(Type::Ptr);b.start(b.block());b.emit(Opcode::Return,Type(),{});}
    const char* types[]={"St8bad_cast","St10bad_typeid","St9bad_alloc"};
    const char* messages[]={"std::bad_cast","std::bad_typeid","std::bad_alloc"};
    const char* names[]={"__cxa_bad_cast","__cxa_bad_typeid",""};
    for(unsigned k=0;k<3;++k){
        bool needed=false;std::string name=names[k];
        auto role=k==0?SR_BAD_CAST:k==1?SR_BAD_TYPEID:SR_ALLOCATE_MEMORY;
        for(const auto& request:requests)if(request.role==role){needed=true;if(k!=2)name=request.name;}
        if(!needed)continue;
        auto type=runtime_typeinfo(r,types[k],exception), message=string(r,"",messages[k]);
        auto what=r.function(r.symbol("",SR_NONE),Type::Ptr);
        {RuntimeBody b(r,what);b.parameter(Type::Ptr);b.start(b.block());b.emit(Opcode::Return,Type::Ptr,{b.emit(Opcode::Addr,Type(),{Operand::symbol(message)})});}
        auto table=data(r,"_ZTV"+std::string(types[k]),{number(0),address(type),address(r.p.functions[dtor.index-1].symbol),address(r.p.functions[dtor.index-1].symbol),address(r.p.functions[what.index-1].symbol)});
        auto symbol=r.symbol(name,k==0?SR_BAD_CAST:k==1?SR_BAD_TYPEID:SR_NONE);
        if(k==2)r.allocation_failure=symbol;
        auto f=r.function(symbol,Type());RuntimeBody b(r,f);b.start(b.block());
        auto object=b.call(allocate,{Operand::integer(8)});
        b.store(object,0,b.offset(b.emit(Opcode::Addr,Type(),{Operand::symbol(table)}),16));
        b.call(raise,{object,b.emit(Opcode::Addr,Type(),{Operand::symbol(type)}),Operand::null()});
        b.emit(Opcode::Unreachable,Type(),{});
    }
}
} }
