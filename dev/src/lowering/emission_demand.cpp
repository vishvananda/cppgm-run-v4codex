#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
void Procedural::emit_demanded_functions(const std::vector<EntityId>& deferred)
{
    std::vector<bool> declared(deferred.size());
    auto declare_pending = [&]() {
        for (unsigned n = 0; n < deferred.size(); ++n) {
            auto e = deferred[n];
            if (!declared[n] && symbols[e]) { declare_function(e); declared[n] = true; }
        }
    };
    for (;;) {
        declare_pending();
        // Bound rescanning on arbitrarily deep call chains. The fallback keeps
        // ordinary eager lowering's semantics without unbounded graph work.
        bool eager = emission_rounds >= 32;
        auto live = eager ? std::vector<bool>() :
            lowir_model::emission_demand(p,linkage.conditional_abi_roots);
        auto before = p.instructions.size();
        for (auto e : definitions) {
            auto emit_entry = [&](SymbolId symbol, bool base) {
                if (!symbol) return;
                const auto& f = p.functions[p.symbols[symbol.index-1].entity-1];
                if (f.declaration || f.blocks.count || (!eager && !live[symbol.index])) return;
                function_body(e,base);
            };
            emit_entry(symbols[e],false);
            emit_entry(base_symbols[e],true);
        }
        if (!emission_rounds) {
            if (!global_initializers.empty()) global_initialization();
            global_finalization();
        }
        // Generated bodies can introduce edges to ordinary source functions.
        // Each helper is materialized once, then participates in the next walk.
        emit_deleting_entries();
        emit_adjustor_thunks();
        emit_allocation_adapters();
        emit_tls_initializers();
        emit_aggregate_helpers();
        emit_local_static_destructors();
        emit_member_thunks();
        emit_terminate_adapter();
        ++emission_rounds;
        if (p.instructions.size() == before) break;
    }
    for (auto e : definitions) {
        for (auto symbol : {symbols[e],base_symbols[e]}) {
            if (!symbol) continue;
            auto& f = p.functions[p.symbols[symbol.index-1].entity-1];
            if (!f.blocks.count) f.declaration = true;
        }
    }
    unsigned kept = 0;
    for (auto alias : p.aliases) {
        const auto& s = p.symbols[alias.target.index-1];
        if (s.kind == lowir_model::Symbol::FunctionSymbol && p.functions[s.entity-1].declaration) continue;
        p.aliases[kept++] = alias;
    }
    p.aliases.resize(kept);
    emit_string_literals();
}
} }
