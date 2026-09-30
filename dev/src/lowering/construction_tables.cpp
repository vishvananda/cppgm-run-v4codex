#include "lowering/procedural.h"
#include <functional>
namespace cppgm { namespace lowering {
using namespace lowir_model;
void Procedural::emit_construction_tables(EntityId complete, SymbolId table)
{
    auto vtt = abi_global(complete,abi_mangle::TargetKind::Vtt);
    if (p.symbols[vtt.index-1].kind == Symbol::GlobalSymbol) return;
    std::vector<DataItem> entries(sem.vtt_size(complete));
    auto scalar = [](IRType type, std::int64_t value) {
        DataItem item; item.kind = DataItem::Scalar; item.type = type;
        item.value = Operand::integer(value); item.value.negative_integer = value < 0; return item;
    };
    auto address = [](SymbolId symbol, std::uint64_t offset) {
        DataItem item; item.kind = DataItem::Address; item.type = IRType::Ptr;
        item.symbol = symbol; item.addend = offset; return item;
    };
    auto publish = [&](SymbolId symbol, const std::vector<DataItem>& data) {
        Global global; global.symbol = symbol; global.structured = true;
        global.data.begin = p.data.size(); global.data.count = data.size();
        for (const auto& item : data) p.data.push_back(item);
        p.globals.push_back(global);
        auto& s = p.symbols[symbol.index-1]; s.kind = Symbol::GlobalSymbol; s.entity = p.globals.size();
    };
    std::function<void(EntityId,std::uint64_t,unsigned,bool)> node;
    node = [&](EntityId cls, std::uint64_t offset, unsigned begin, bool root) {
        const auto& model = sem.virtual_class(cls);
        auto location = [&](unsigned receiver) -> std::uint64_t {
            if (!receiver) return offset;
            const auto& view = model.views[receiver-1];
            return view.virtual_anchor ? sem.virtual_base_offset(complete,view.virtual_anchor)+view.virtual_tail : offset+view.offset;
        };
        semantic::Index points;
        auto segment = [&](unsigned view, unsigned index) {
            auto point = view ? model.views[view-1].address_point : model.address_point;
            auto at = location(view);
            if (linkage.host) {
                if (auto prior = points.get(at)) { entries[index] = entries[prior-1]; return; }
                points.put(at,index+1);
            }
            if (root) {
                entries[index] = linkage.host ? address(table,view ? model.views[view-1].group_address_point : point) :
                    address(view ? view_symbol(cls,view) : table,point);
                return;
            }
            auto key = (std::uint64_t(table.index)<<32)|index;
            auto old = linkage.construction_symbols.get(key);
            auto symbol = old ? SymbolId(old) : fresh_symbol("@construction_vtable");
            entries[index] = address(symbol,point);
            if (old) return;
            linkage.construction_symbols.put(key,symbol.index);
            auto& metadata = p.symbols[symbol.index-1].metadata;
            metadata.binding = ir_model::SBM_INTERNAL;
            metadata.object = p.intern("__cppgm_construction_vtable_"+std::to_string(symbol.index));
            auto owner = view ? model.views[view-1].type : cls;
            std::vector<DataItem> data;
            if (linkage.host) {
                const auto& rows = view ? model.view_prefix : model.prefix;
                auto first = view ? model.views[view-1].prefix_begin : 0;
                auto n = view ? model.views[view-1].prefix_count : rows.size();
                for (unsigned j = n; j; --j) {
                    const auto& row = rows[first+j-1];
                    auto target = row.base ? sem.virtual_base_offset(complete,row.base) : location(row.receiver);
                    data.push_back(scalar(IRType::I64,std::int64_t(target)-at));
                }
            } else {
                if (view) for (unsigned j = 0; j < model.views[view-1].vcall_rows; ++j)
                    data.push_back(scalar(IRType::I64,0));
                for (unsigned j = sem.virtual_base_count(owner); j; --j)
                    data.push_back(scalar(IRType::I64,std::int64_t(sem.virtual_base_offset(complete,sem.virtual_base_type(owner,j-1)))-at));
            }
            data.push_back(scalar(IRType::I64,std::int64_t(offset)-at));
            data.push_back(sem.polymorphic(cls) ? address(typeinfo(cls),0) : scalar(IRType::Ptr,0));
            auto first = view ? model.views[view-1].begin : 0;
            auto count = view ? model.views[view-1].count : model.primary_count;
            for (unsigned j = 0; j < count; ++j) {
                auto slot = model.slots[first+j];
                slot.this_adjustment = std::int64_t(location(slot.receiver))-at;
                bool deleting = sem.destructor_member(slot.function) && j && model.slots[first+j-1].function == slot.function;
                data.push_back(address(virtual_target(slot,deleting),0));
            }
            publish(symbol,data);
        };
        segment(0,begin);
        for (unsigned j = 0; j < sem.lifecycle_count(cls); ++j) {
            const auto& base = sem.lifecycle_bases[sem.lifecycle_begin(cls)+j];
            if (!base.virtual_base && base.vtt) node(base.type,offset+base.offset,begin+base.vtt,false);
        }
        unsigned index = begin+sem.vtt_secondary(cls);
        for (unsigned j : linkage.host ? model.vtt_order : model.store_order) segment(j+1,index++);
        if (root) for (unsigned j = 0; j < sem.virtual_base_count(cls); ++j) {
            const auto& base = sem.lifecycle_bases[sem.lifecycle_begin(cls)+j];
            if (base.vtt) node(base.type,base.offset,begin+base.vtt,false);
        }
    };
    node(complete,0,0,true);
    p.symbols[vtt.index-1].metadata.object_root = true;
    publish(vtt,entries);
}
} }
