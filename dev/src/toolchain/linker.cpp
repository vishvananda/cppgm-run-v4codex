#include "toolchain/runtime.h"
#include <algorithm>
namespace cppgm { namespace toolchain {
using lowir_model::require;
Linker::Linker() : image_(0), symbols_(image_.symbols.size()), symbol_definitions_(symbols_.size()) {}
unsigned Linker::new_symbol()
{
    unsigned id = symbols_.size(); symbols_.push_back(Symbol());
    symbol_definitions_.push_back(0);
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
    std::vector<unsigned> definitions(obj.symbols.size());
    for (unsigned i = 1; i < definitions.size(); ++i) if (obj.symbols[i].definition == i) {
        definitions[i] = lazy_definitions_.size(); lazy_definitions_.push_back(obj.symbols[i].lazy);
    }
    for (unsigned i = 1; i < map.size(); ++i) {
        auto record = obj.symbols[i];
        if (record.name) { auto text = obj.name(i); record.name = names_.intern({text.data(),text.size()}); }
        unsigned id = 0;
        bool runtime = i >= source.runtime_begin && i < source.runtime_begin+unsigned(native::RuntimeEntity::Count);
        if (runtime) id = image_.runtime_begin + i-source.runtime_begin;
        else if (record.binding == ir_model::SBM_INTERNAL) id = new_symbol();
        else {
            require(record.name,"external object symbol has no name");
            auto name = record.name;
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
        symbol_definitions_[id] = definitions[record.definition];
        image_.symbols[id] = source.symbols[i] + (source.data_symbols[i] ? data_offset : code_offset);
        if (record.role == ir_model::SR_ENTRY) {
            require(!entry_ || entry_ == id,"multiple entry functions"); entry_ = id; parameters_ = record.parameters;
        } else if (record.role == ir_model::SR_INIT) initializers_.push_back(id);
        else if (record.role == ir_model::SR_FINI) finalizers_.push_back(id);
    }
    auto fixes = [&](const std::vector<native::Fixup>& input, std::vector<native::Fixup>& output, std::size_t offset) {
        for (auto f : input) {
            f.owner = definitions.at(f.owner);
            f.symbol = map.at(f.symbol); f.offset += offset;
            if (f.kind == native::Fixup::RelativeSymbol) f.end += offset;
            output.push_back(f);
        }
    };
    fixes(source.code_fixups,image_.code_fixups,code_offset);
    fixes(source.data_fixups,image_.data_fixups,data_offset);
    image_.code.insert(image_.code.end(),source.code.begin(),source.code.end());
    image_.data.insert(image_.data.end(),source.data.begin(),source.data.end());
    image_.has_tls |= source.has_tls;
}
void Linker::retain_relocations()
{
    struct Edge { unsigned symbol, next; };
    std::vector<Edge> edges(1);
    std::vector<unsigned> heads(lazy_definitions_.size()), work;
    std::vector<bool> live(heads.size());
    std::vector<bool> requested(symbols_.size());
    auto demand = [&](unsigned definition) {
        if (!live[definition]) { live[definition] = true; work.push_back(definition); ++definition_work; }
    };
    demand(0); // Unowned section bytes, including foreign unwind records.
    for (unsigned id = 1; id < symbols_.size(); ++id)
        if (image_.defined[id] && !lazy_definitions_[symbol_definitions_[id]]) demand(symbol_definitions_[id]);
    for (const auto* fixes : {&image_.code_fixups,&image_.data_fixups}) for (const auto& f : *fixes) {
        edges.push_back({f.symbol,heads[f.owner]}); heads[f.owner] = edges.size()-1;
    }
    // Each definition and dependency edge is visited at most once. A retained
    // alias roots the same definition; a discarded weak body's GOT stays cold.
    for (unsigned at = 0; at < work.size(); ++at) for (auto i = heads[work[at]]; i; i = edges[i].next) {
        ++relocation_work;
        auto symbol = edges[i].symbol;
        if (!image_.defined[symbol]) {
            if (runtime_role(symbols_[symbol].role)) {
                if (!requested[symbol]) { requested[symbol] = true; runtime_demands_.push_back(symbol); }
                continue;
            }
            auto name = symbols_[symbol].name ? names_.spelling(symbols_[symbol].name) : cppgm::TextView{"<runtime>",9};
            throw std::runtime_error("unresolved native symbol: " + std::string(name.data,name.size));
        }
        demand(symbol_definitions_[symbol]);
    }
    for (auto* fixes : {&image_.code_fixups,&image_.data_fixups})
        fixes->erase(std::remove_if(fixes->begin(),fixes->end(),[&](const native::Fixup& f) {
            return !live[f.owner];
        }),fixes->end());
}
std::size_t Linker::finish(const std::string& path)
{
    require(entry_ && image_.defined[entry_],"missing main");
    retain_relocations();
    supply_runtime();
    std::vector<lowir_model::SymbolId> init, fini;
    for (auto id : initializers_) init.push_back(lowir_model::SymbolId(id));
    for (auto id : finalizers_) fini.push_back(lowir_model::SymbolId(id));
    native::Image header(symbols_.size()-1-unsigned(native::RuntimeEntity::Count));
    header.runtime_begin = image_.runtime_begin; header.has_tls = image_.has_tls;
    native::Encoder encoder(header);
    encoder.startup(native::startup(lowir_model::SymbolId(entry_),parameters_,init,fini));
    // Preserve object section alignments after prefixing process startup.
    header.code.resize(((native::executable_code_offset+header.code.size()+alignment_-1)&~std::size_t(alignment_-1))-native::executable_code_offset,0x90);
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
