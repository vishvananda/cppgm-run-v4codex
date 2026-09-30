#include "toolchain/object.h"
#include <algorithm>
namespace cppgm { namespace toolchain {
using lowir_model::require;
Linker::Linker() : image_(0), symbols_(image_.symbols.size()) {}
unsigned Linker::new_symbol()
{
    unsigned id = symbols_.size(); symbols_.push_back(Symbol());
    image_.symbols.push_back(0); image_.defined.push_back(false);
    image_.data_symbols.push_back(false); image_.tls_targets.push_back(0); return id;
}
void Linker::add(Object&& obj)
{
    auto& source = obj.image;
    alignment_ = std::max(alignment_,obj.alignment);
    image_.code.resize((image_.code.size()+obj.alignment-1)&~std::size_t(obj.alignment-1),0x90);
    image_.data.resize((image_.data.size()+obj.alignment-1)&~std::size_t(obj.alignment-1),0);
    auto code_offset = image_.code.size(), data_offset = image_.data.size();
    std::vector<unsigned> map(obj.symbols.size());
    for (unsigned i = 1; i < map.size(); ++i) {
        auto record = obj.symbols[i];
        if (record.name) { auto text = obj.name(i); record.name = names_.intern({text.data(),text.size()}); }
        unsigned id = 0;
        bool runtime = i >= source.runtime_begin && i < source.runtime_begin+unsigned(native::RuntimeEntity::Count);
        if (runtime) id = image_.runtime_begin + i-source.runtime_begin;
        else if (record.binding == ir_model::SBM_INTERNAL) id = new_symbol();
        else {
            auto text = obj.name(i); require(!text.empty(),"external object symbol has no name");
            auto name = names_.intern({text.data(),text.size()});
            id = externals_.find(name);
            if (!id) { id = new_symbol(); externals_.insert(name,id); }
            record.name = name;
        }
        map[i] = id;
        if (!image_.defined[id]) symbols_[id] = record;
        if (!source.defined[i]) continue;
        if (image_.defined[id]) {
            if (runtime) continue;
            if (record.binding == ir_model::SBM_WEAK) continue;
            if (symbols_[id].binding != ir_model::SBM_WEAK)
                throw std::runtime_error("duplicate native definition: " + obj.name(i));
        }
        symbols_[id] = record; image_.defined[id] = true; image_.data_symbols[id] = source.data_symbols[i];
        image_.symbols[id] = source.symbols[i] + (source.data_symbols[i] ? data_offset : code_offset);
        if (record.role == ir_model::SR_ENTRY) {
            require(!entry_ || entry_ == id,"multiple entry functions"); entry_ = id; parameters_ = record.parameters;
        } else if (record.role == ir_model::SR_INIT) initializers_.push_back(id);
        else if (record.role == ir_model::SR_FINI) finalizers_.push_back(id);
    }
    struct Definition { std::uint64_t offset; unsigned symbol; bool fragment; };
    std::vector<Definition> code_owners, data_owners;
    for (unsigned i = 1; i < map.size(); ++i) if (source.defined[i]) {
        auto& owners = source.data_symbols[i] ? data_owners : code_owners;
        owners.push_back({source.symbols[i],i,obj.symbols[i].fragment});
    }
    auto order = [](const Definition& a, const Definition& b) {
        if (a.offset != b.offset) return a.offset < b.offset;
        if (a.fragment != b.fragment) return !a.fragment;
        return a.symbol > b.symbol;
    };
    std::sort(code_owners.begin(),code_owners.end(),order);
    std::sort(data_owners.begin(),data_owners.end(),order);
    auto fixes = [&](const std::vector<native::Fixup>& input, std::vector<native::Fixup>& output, std::size_t offset, const std::vector<Definition>& owners) {
        for (auto f : input) {
            auto owner = std::upper_bound(owners.begin(),owners.end(),f.offset,
                [](std::uint64_t address, const Definition& d) { return address < d.offset; });
            if (owner != owners.begin()) {
                --owner; f.owner = map[owner->symbol]; f.definition = owner->offset+offset;
            }
            f.symbol = map.at(f.symbol); f.offset += offset;
            if (f.kind == native::Fixup::RelativeSymbol) f.end += offset;
            output.push_back(f);
        }
    };
    fixes(source.code_fixups,image_.code_fixups,code_offset,code_owners);
    fixes(source.data_fixups,image_.data_fixups,data_offset,data_owners);
    image_.code.insert(image_.code.end(),source.code.begin(),source.code.end());
    image_.data.insert(image_.data.end(),source.data.begin(),source.data.end());
    image_.has_tls |= source.has_tls;
}
std::size_t Linker::finish(const std::string& path)
{
    require(entry_ && image_.defined[entry_],"missing main");
    for (auto* fixes : {&image_.code_fixups,&image_.data_fixups})
        fixes->erase(std::remove_if(fixes->begin(),fixes->end(),[&](const native::Fixup& f) {
            return f.owner && image_.symbols[f.owner] != f.definition;
        }),fixes->end());
    // Relocations use the one resolved identity even from discarded weak bodies.
    for (const auto* fixes : {&image_.code_fixups,&image_.data_fixups}) for (const auto& f : *fixes)
        if (!image_.defined[f.symbol]) {
            auto name = symbols_[f.symbol].name ? names_.spelling(symbols_[f.symbol].name) : cppgm::TextView{"<runtime>",9};
            throw std::runtime_error("unresolved native symbol: " + std::string(name.data,name.size));
        }
    std::vector<lowir_model::SymbolId> init, fini;
    for (auto id : initializers_) init.push_back(lowir_model::SymbolId(id));
    for (auto id : finalizers_) fini.push_back(lowir_model::SymbolId(id));
    native::Image header(symbols_.size()-1-unsigned(native::RuntimeEntity::Count));
    header.runtime_begin = image_.runtime_begin; header.has_tls = image_.has_tls;
    native::Encoder encoder(header);
    encoder.startup(native::startup(lowir_model::SymbolId(entry_),parameters_,init,fini));
    // Preserve object section alignments after prefixing process startup.
    header.code.resize((header.code.size()+alignment_-1)&~std::size_t(alignment_-1),0x90);
    auto prefix = header.code.size();
    for (unsigned i = 1; i < image_.symbols.size(); ++i)
        if (image_.defined[i] && !image_.data_symbols[i]) image_.symbols[i] += prefix;
    for (auto& f : image_.code_fixups) {
        f.offset += prefix; if (f.kind == native::Fixup::RelativeSymbol) f.end += prefix;
    }
    image_.code_fixups.insert(image_.code_fixups.end(),header.code_fixups.begin(),header.code_fixups.end());
    header.code.insert(header.code.end(),image_.code.begin(),image_.code.end()); image_.code.swap(header.code);
    native::write_executable(image_,path); return image_.code.size();
}
} }
