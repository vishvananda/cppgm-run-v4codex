#include "lowering/procedural.h"
#include <cstring>
namespace cppgm { namespace lowering {
namespace {
// Hash only the typed payload, never struct padding, printed IR or source names.
std::uint64_t mix(std::uint64_t h, std::uint64_t v) { return (h ^ v)*1099511628211ULL; }
std::uint64_t item_hash(const lowir_model::DataItem& item)
{
    auto h = mix(1469598103934665603ULL,item.kind);
    if (item.kind == lowir_model::DataItem::Zero) return mix(h,item.zero_bytes);
    h = mix(h,item.type.kind());
    if (item.kind == lowir_model::DataItem::Address) return mix(mix(h,item.symbol.index),item.addend);
    h = mix(h,item.value.kind);
    if (item.value.kind != Operand::Floating) return mix(h,item.value.data.integer);
    // The target and host are x86-64; the x87 payload occupies ten bytes.
    const auto* bytes = reinterpret_cast<const unsigned char*>(&item.value.data.floating);
    for (unsigned i = 0; i < 10; ++i) h = mix(h,bytes[i]);
    return h;
}
bool item_equal(const lowir_model::DataItem& a, const lowir_model::DataItem& b)
{
    if (a.kind != b.kind) return false;
    if (a.kind == lowir_model::DataItem::Zero) return a.zero_bytes == b.zero_bytes;
    if (a.type != b.type) return false;
    if (a.kind == lowir_model::DataItem::Address) return a.symbol == b.symbol && a.addend == b.addend;
    if (a.value.kind != b.value.kind) return false;
    return a.value.kind == Operand::Floating ? !std::memcmp(&a.value.data.floating,&b.value.data.floating,10) : a.value.data.integer == b.value.data.integer;
}
}
void Procedural::initialize_constant_array(EntityId e, Value location)
{
    auto target = sem.entities[e].type;
    SymbolId source(constant_arrays.get(e));
    auto bytes = sem.object_size(target), alignment = sem.object_alignment(target);
    if (!source.index) {
        lowir_model::Range data; data.begin = p.data.size(); global_plan(sem.constant_array_plan(e));
        data.count = p.data.size()-data.begin;
        auto hash = mix(mix(1469598103934665603ULL,bytes),alignment);
        for (unsigned i = 0; i < data.count; ++i) { ++constant_data_work; hash = mix(hash,item_hash(p.data[data.begin+i])); }
        auto head = constant_data_index.get(hash);
        for (auto id = head; id; id = constant_data_records[id].next) {
            auto other = constant_data_records[id];
            if (other.bytes != bytes || other.alignment != alignment || other.data.count != data.count) continue;
            bool equal = true;
            for (unsigned i = 0; equal && i < data.count; ++i) { ++constant_data_work; equal = item_equal(p.data[data.begin+i],p.data[other.data.begin+i]); }
            if (equal) { ++constant_data_hits; source = other.symbol; p.data.resize(data.begin); break; }
        }
        if (!source) {
            lowir_model::Global g; g.structured = true; g.data = data;
            g.symbol = fresh_symbol("@__constant_array_"+std::to_string(e)); p.globals.push_back(g);
            auto& symbol = p.symbols[g.symbol.index-1];
            symbol.kind = lowir_model::Symbol::GlobalSymbol; symbol.entity = p.globals.size();
            symbol.metadata.binding = ir_model::SBM_INTERNAL; symbol.metadata.storage = ir_model::GSM_READONLY;
            source = g.symbol;
            constant_data_index.put(hash,constant_data_records.size());
            constant_data_records.push_back({data,bytes,alignment,source,head});
        }
        constant_arrays.put(e,source.index);
    }
    Instruction copy(Opcode::CopyObject); copy.bytes = bytes; copy.alignment = alignment;
    emit(copy,{Operand::symbol(source),address(location).operand});
}
} }
