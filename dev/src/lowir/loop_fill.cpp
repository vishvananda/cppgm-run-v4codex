#include "lowir/loop_simplify.h"
namespace lowir_model {
SymbolId fill_runtime(Program& p, SymbolId& cached, std::uint64_t& work)
{
    if (cached) return cached;
    using Builtin = SymbolMetadata::Builtin;
    // Once per invocation, recover a compatible already-published runtime
    // entity (for example after a text roundtrip). Extra parameter promises
    // are part of the semantic key: do not attach them to new calls.
    for (const auto& f : p.functions) {
        ++work;
        const auto& symbol = p.symbols[f.symbol.index-1];
        if (!f.declaration || symbol.metadata.builtin != Builtin::FillBytes ||
            symbol.metadata.binding != SBM_INTERNAL || symbol.metadata.role != SR_NONE || symbol.metadata.force_inline) continue;
        const auto& sig = p.signatures[f.signature.index-1];
        if (sig.result != Type::Void || sig.parameters.count != 3 ||
            sig.boundary.arity != CAM_FIXED || sig.boundary.effects != CFXM_DEFAULT ||
            sig.boundary.returns != CRM_DEFAULT || sig.boundary.query != CQM_DEFAULT) continue;
        bool compatible = true;
        for (unsigned n = 0; n < 3; ++n) {
            const auto& a = p.parameters[sig.parameters.begin+n];
            compatible &= a.type == (n == 0 ? Type::Ptr : n == 1 ? Type::I32 : Type::I64) &&
                a.passing == PPM_DIRECT && a.alias == PALM_DEFAULT && !a.object_bytes;
        }
        if (compatible) { cached = f.symbol; return cached; }
    }
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
