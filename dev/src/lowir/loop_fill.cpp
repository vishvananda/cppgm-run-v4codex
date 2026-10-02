#include "lowir/loop_simplify.h"
namespace lowir_model {
SymbolId fill_runtime(Program& p, SymbolId& cached)
{
    if (cached) return cached;
    using Builtin = SymbolMetadata::Builtin;
    // The invocation owns this single-entry cache. Runtime identity is typed;
    // names are chosen only at the explicit symbol/ABI boundary.
    unsigned serial = p.symbols.size(); Name name;
    do { name = p.intern("@opt_fill_"+std::to_string(serial++)); } while (p.symbol_names.find(name));
    cached = p.symbol(name);
    auto& symbol = p.symbols[cached.index-1];
    symbol.kind = Symbol::FunctionSymbol; symbol.entity = p.functions.size()+1;
    symbol.metadata.builtin = Builtin::FillBytes; symbol.metadata.binding = SBM_INTERNAL;
    symbol.metadata.object = p.intern("cppgm_opt_fill_bytes");
    Function f; f.symbol = cached; f.declaration = true;
    Signature sig; sig.result = Type::Void; sig.boundary.unwind = CUM_NO;
    sig.parameters.begin = p.parameters.size(); sig.parameters.count = 3;
    for (unsigned n = 0; n < 3; ++n) {
        Value v; v.name = p.intern("%arg"+std::to_string(n)); v.defined = true;
        v.owner = FunctionId(symbol.entity); v.type = n == 2 ? Type::I64 : n == 1 ? Type::I32 : Type::Ptr;
        p.values.push_back(v);
        Parameter a; a.value = ValueId(p.values.size()); a.type = v.type; p.parameters.push_back(a);
    }
    p.signatures.push_back(sig); f.signature = SignatureId(p.signatures.size());
    p.functions.push_back(f);
    if (!p.function_order.empty()) p.function_order.push_back(FunctionId(p.functions.size()));
    return cached;
}
}
