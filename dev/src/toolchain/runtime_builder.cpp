#include "toolchain/runtime.h"
namespace cppgm { namespace toolchain {
using namespace lowir_model;
SymbolId RuntimeProgram::symbol(const std::string& name, SymbolRole role)
{
    auto id = p.symbol(p.intern("@runtime"+std::to_string(p.symbols.size())));
    auto& m = p.symbols[id.index-1].metadata;
    m.binding = name.empty() ? SBM_INTERNAL : SBM_WEAK;
    if (!name.empty()) m.object = p.intern(name);
    m.role = role; return id;
}
FunctionId RuntimeProgram::function(SymbolId symbol, Type result)
{
    Signature sig; sig.result = result; sig.parameters.begin = p.parameters.size();
    p.signatures.push_back(sig);
    Function f; f.symbol = symbol; f.signature = SignatureId(p.signatures.size()); p.functions.push_back(f);
    auto& s = p.symbols[symbol.index-1]; s.kind = lowir_model::Symbol::FunctionSymbol; s.entity = p.functions.size();
    return FunctionId(p.functions.size());
}
void RuntimeProgram::table(SymbolId symbol)
{
    // Reserved Itanium vtable header and address point. Standalone class RTTI
    // identifies its three runtime classes by these canonical address points.
    Global g; g.symbol = symbol; g.structured = true; g.data.begin = p.data.size(); g.data.count = 1;
    DataItem d; d.kind = DataItem::Zero; d.zero_bytes = 32; p.data.push_back(d); p.globals.push_back(g);
    auto& s = p.symbols[symbol.index-1]; s.kind = lowir_model::Symbol::GlobalSymbol; s.entity = p.globals.size();
}
RuntimeBody::RuntimeBody(RuntimeProgram& program, FunctionId f) : p(program.p), function(f), builder(p,f) {}
RuntimeBody::Operand RuntimeBody::parameter(Type type)
{
    Parameter a; a.type = type; a.value = builder.value(p.intern("%arg"+std::to_string(++ordinal)));
    builder.parameter(a); ++p.signatures[p.functions[function.index-1].signature.index-1].parameters.count;
    return Operand::value(a.value);
}
RuntimeBody::Operand RuntimeBody::emit(Opcode op, Type type, std::initializer_list<Operand> args, Operation operation)
{
    Instruction i(op,type); i.operation = operation;
    Name dest = i.result_type() == Type() ? 0 : p.intern("%v"+std::to_string(++ordinal));
    return Operand::value(builder.append(i,args,dest));
}
RuntimeBody::Operand RuntimeBody::call(FunctionId f, std::initializer_list<Operand> args)
{
    Instruction i(Opcode::Call,p.signatures[p.functions[f.index-1].signature.index-1].result);
    i.signature = p.functions[f.index-1].signature; i.operands.begin = p.operands.size();
    p.operands.push_back(Operand::symbol(p.functions[f.index-1].symbol));
    for (auto a : args) p.operands.push_back(a);
    i.operands.count = args.size()+1;
    if (i.result_type() != Type()) i.destination = builder.value(p.intern("%v"+std::to_string(++ordinal)));
    builder.append(i); return Operand::value(i.destination);
}
RuntimeBody::Operand RuntimeBody::offset(Operand base, std::int64_t bytes)
{
    if (!bytes) return base;
    auto n = Operand::integer(bytes); n.negative_integer = bytes < 0;
    return emit(Opcode::Index,Type::I8,{base,n});
}
RuntimeBody::Operand RuntimeBody::load(Operand base, std::int64_t bytes, Type type) { return emit(Opcode::Load,type,{offset(base,bytes)}); }
void RuntimeBody::store(Operand base, std::int64_t bytes, Operand value, Type type) { emit(Opcode::Store,type,{value,offset(base,bytes)}); }
RuntimeBody::Operand RuntimeBody::compare(Operand a, Operand b, Operation op, Type type) { return emit(Opcode::Compare,type,{a,b},op); }
BlockId RuntimeBody::block() { return builder.block(p.intern("^b"+std::to_string(++ordinal))); }
void RuntimeBody::start(BlockId b) { builder.start_block(b); }
void RuntimeBody::jump(BlockId b) { emit(Opcode::Jump,Type(),{Operand::label(b)}); }
void RuntimeBody::branch(Operand condition, BlockId yes, BlockId no) { emit(Opcode::Branch,Type(),{condition,Operand::label(yes),Operand::label(no)}); }
} }
