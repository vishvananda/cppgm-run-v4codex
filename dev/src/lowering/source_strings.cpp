#include "lowering/procedural.h"
namespace cppgm { namespace lowering {
SymbolId Procedural::source_string_symbol(IdentifierId text)
{
    if (auto old = source_string_symbols.get(text)) return SymbolId(old);
    auto symbol = fresh_symbol("@__source_string_"+std::to_string(p.symbols.size()+1));
    source_string_symbols.put(text,symbol.index); source_strings.push_back({text,symbol});
    auto& metadata = p.symbols[symbol.index-1].metadata;
    metadata.binding = ir_model::SBM_INTERNAL; metadata.storage = ir_model::GSM_READONLY;
    return symbol;
}
void Procedural::emit_source_strings()
{
    // A use records identity only. Deferred data cannot interrupt a global's
    // contiguous initializer slice, including a constexpr pointer relocation.
    for (auto source : source_strings) {
        auto text = identifiers.spelling(source.text);
        lowir_model::Global global; global.structured = true; global.symbol = source.symbol;
        global.data.begin = p.data.size(); global.data.count = text.size+1;
        for (unsigned j = 0; j <= text.size; ++j) {
            lowir_model::DataItem item; item.kind = lowir_model::DataItem::Scalar; item.type = IRType::I8;
            item.value = Operand::integer(j < text.size ? static_cast<unsigned char>(text.data[j]) : 0);
            p.data.push_back(item);
        }
        p.globals.push_back(global);
        auto& symbol = p.symbols[source.symbol.index-1];
        symbol.kind = lowir_model::Symbol::GlobalSymbol; symbol.entity = p.globals.size();
    }
}
} }
