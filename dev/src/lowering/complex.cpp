#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
Value Procedural::complex_component(Value value, unsigned part)
{
    auto component = sem.types.qualify(sem.complex_component(value.type),sem.types[value.type].cv);
    // A complex SSA value has storage in LowIR. Publish an address only when
    // projecting a component; never reconstruct an entity or source node.
    value.address = true;
    auto base = address(value);
    if (part) base = emit(Opcode::Index,IRType::I8,{base.operand,Operand::integer(sem.object_size(component))});
    base.type = component; base.ir = type(component); base.address = true; return base;
}
Value Procedural::complex_construct(TypeId t, Value real, Value imag)
{
    auto slot = builder->add_slot(0,type(t));
    Value object(Operand::slot(slot),type(t),t,true);
    store(real,complex_component(object,0)); store(imag,complex_component(object,1));
    object.address = false; return object;
}
Value Procedural::complex_projection(NodeId n)
{
    auto node = ast[n]; auto value = expression(node.first,true);
    if (sem.complex_type(value.type)) {
        auto result = complex_component(value,node.op == KW_IMAG);
        return sem.expression_fact(n).category == semantic::ValueCategory::Prvalue ? load(result) : result;
    }
    if (node.op == KW_REAL) return value;
    load(value); auto t = sem.expression_fact(n).type;
    return Value(type(t).floating() ? Operand::floating(0) : Operand::integer(0),type(t),t);
}
Value Procedural::complex_convert(Value value, TypeId target)
{
    bool from = sem.complex_type(value.type), to = sem.complex_type(target);
    if (from && to && sem.types.unqualified(value.type) == sem.types.unqualified(target)) {
        value = load(value); value.type = target; return value;
    }
    auto real = from ? load(complex_component(value,0)) : load(value);
    if (!to) {
        if (sem.types[target].fundamental == FT_BOOL) {
            auto imag = convert(load(complex_component(value,1)),target);
            real = convert(real,target);
            auto result = emit(Opcode::Binary,real.ir,{real.operand,imag.operand},Operation::Or);
            result.type = target; return result;
        }
        return convert(real,target);
    }
    auto component = sem.complex_component(target);
    auto imag = from ? convert(load(complex_component(value,1)),component) : Value(Operand::floating(0),type(component),component);
    return complex_construct(target,convert(real,component),imag);
}
Value Procedural::complex_operation(ETokenType op, Value a, Value b, TypeId result)
{
    auto ar = load(complex_component(a,0)), ai = load(complex_component(a,1));
    if (!b.type) {
        if (op == OP_PLUS) return a;
        if (op == OP_MINUS) { auto type = ar.type; ar = emit(Opcode::Unary,ar.ir,{ar.operand},Operation::Neg); ar.type = type; }
        auto type = ai.type; ai = emit(Opcode::Unary,ai.ir,{ai.operand},Operation::Neg); ai.type = type;
        return complex_construct(result,ar,ai);
    }
    if (!sem.complex_type(b.type)) b = convert(b,a.type);
    auto br = load(complex_component(b,0)), bi = load(complex_component(b,1));
    if (op == OP_EQ || op == OP_NE) {
        auto real = operation(op,ar,br,result), imag = operation(op,ai,bi,result);
        auto v = emit(Opcode::Binary,real.ir,{real.operand,imag.operand},op == OP_EQ ? Operation::And : Operation::Or);
        v.type = result; return v;
    }
    if (op == OP_PLUS || op == OP_MINUS) {
        // IR identity must not depend on the host's argument evaluation order.
        auto imag = operation(op,ai,bi,ai.type);
        auto real = operation(op,ar,br,ar.type);
        return complex_construct(result,real,imag);
    }
    // libgcc's complex multiply/divide implement scaling and exceptional-value
    // recovery. Ordinary scalar formulas lose infinities and overflow division.
    // Runtime calls are explicit typed LowIR, shared by every backend adapter.
    using namespace lowir_model;
    unsigned precision = sem.types[ar.type].fundamental-FT_FLOAT;
    unsigned index = precision*2+(op == OP_DIV);
    if (!linkage.complex_runtime_symbols[index]) {
        static const char* names[] = {"__mulsc3","__divsc3","__muldc3","__divdc3","__mulxc3","__divxc3"};
        Function f; f.symbol = fresh_symbol("@complex_runtime"); f.declaration = true;
        auto sig = sem.types.function(result,{ar.type,ar.type,ar.type,ar.type},false);
        FunctionId owner(p.functions.size()+1); f.signature = signature(sig,owner);
        p.signatures[f.signature.index-1].boundary.unwind = ir_model::CUM_NO;
        p.functions.push_back(f); linkage.complex_runtime_symbols[index] = f.symbol;
        auto& symbol = p.symbols[f.symbol.index-1]; symbol.kind = Symbol::FunctionSymbol; symbol.entity = owner.index;
        symbol.metadata.binding = SBM_STRONG; symbol.metadata.linkage = LLM_C; symbol.metadata.object = p.intern(names[index]);
    }
    auto value = emit(Opcode::Call,type(result),{Operand::symbol(linkage.complex_runtime_symbols[index]),ar.operand,ai.operand,br.operand,bi.operand});
    value.type = result; return value;
}
void Procedural::complex_data(TypeId t, semantic::Constant value)
{
    for (unsigned part = 0; part < 2; ++part) {
        auto c = sem.complex_part(value,part);
        p.data.push_back(floating_data(type(sem.complex_component(t)),sem.floating_value(c),sem.floating_signaling(c)));
    }
}
} }
