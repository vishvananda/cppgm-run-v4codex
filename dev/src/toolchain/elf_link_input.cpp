#include "toolchain/elf_model.h"
#include "support/id_index.h"
#include <algorithm>
#include <cstring>
namespace cppgm { namespace toolchain {
using lowir_model::require;
namespace {
// Explicit ELF input establishes producer extents once. Later resolution uses
// definition identity, including aliases and discarded weak-body relocations.
struct Definitions {
    struct Range { std::uint64_t begin, end; unsigned owner; };
    std::vector<Range> lanes[3];
    void add(unsigned lane, std::uint64_t begin, std::uint64_t size, unsigned owner) {
        if (size) lanes[lane].push_back({begin,begin+size,owner});
    }
    void finish(Object& obj) {
        for (auto* ranges : {&lanes[0],&lanes[1],&lanes[2]}) {
            std::sort(ranges->begin(),ranges->end(),[](const Range& a,const Range& b) { return a.begin < b.begin; });
            unsigned count = 0;
            for (auto r : *ranges) {
                if (count && (*ranges)[count-1].begin == r.begin) {
                    auto& prior = (*ranges)[count-1]; prior.end = std::max(prior.end,r.end);
                    obj.symbols[r.owner].definition = prior.owner;
                } else {
                    require(!count || (*ranges)[count-1].end <= r.begin,"overlapping ELF definitions");
                    obj.symbols[r.owner].definition = r.owner; (*ranges)[count++] = r;
                }
            }
            ranges->resize(count);
        }
    }
    unsigned owner(unsigned lane, std::uint64_t offset) const {
        const auto& ranges = lanes[lane];
        auto at = std::upper_bound(ranges.begin(),ranges.end(),offset,
            [](std::uint64_t n,const Range& r) { return n < r.begin; });
        if (at == ranges.begin()) return 0;
        --at; return offset < at->end ? at->owner : 0;
    }
};
}
Object link_elf(ElfModule&& module)
{
    auto& sections = module.sections; auto count = module.symbols.size();
    Object obj(count+sections.size()); obj.image.host = true;
    std::vector<std::uint64_t> offsets(sections.size());
    std::vector<bool> loaded(sections.size());
    for (unsigned i = 1; i < sections.size(); ++i) {
        auto& section = sections[i]; const auto& s = section.header;
        if (!(s.sh_flags & SHF_ALLOC)) continue;
        section.header.sh_size = section.bytes.size();
        auto alignment = std::max(std::uint64_t(1),s.sh_addralign);
        require(alignment <= 4096 && !(alignment&(alignment-1)),"unsupported ELF section alignment");
        obj.alignment = std::max(obj.alignment,unsigned(alignment));
        bool data = !(s.sh_flags & SHF_EXECINSTR);
        bool tls = s.sh_flags & SHF_TLS;
        if (tls) { obj.image.has_tls |= s.sh_size != 0; obj.image.tls_alignment = std::max(obj.image.tls_alignment,unsigned(alignment)); }
        auto& out = tls ? obj.image.tls : data ? obj.image.data : obj.image.code;
        out.resize((out.size()+alignment-1)&~std::size_t(alignment-1),0); offsets[i] = out.size(); loaded[i] = true;
        if (out.empty()) out = std::move(section.bytes);
        else out.insert(out.end(),section.bytes.begin(),section.bytes.end());
        if (i == module.unwind_section && s.sh_size) obj.frame_begin = offsets[i];
        auto id = count+i+1;
        obj.image.defined[id] = true; obj.image.data_symbols[id] = data; obj.image.symbols[id] = offsets[i];
        if (tls) obj.image.tls_targets[id] = id;
    }
    Definitions definitions;
    const auto& strings = sections[module.strings].bytes;
    for (unsigned i = 1; i < count; ++i) {
        const auto& s = module.symbols[i]; auto id = i+1; auto& record = obj.symbols[id];
        require(s.st_name < strings.size(),"invalid ELF symbol name");
        const char* begin = reinterpret_cast<const char*>(strings.data()+s.st_name);
        auto end = static_cast<const char*>(std::memchr(begin,0,strings.size()-s.st_name));
        require(end,"unterminated ELF symbol name");
        if (begin != end) record.name = obj.intern(std::string(begin,end));
        auto binding = ELF64_ST_BIND(s.st_info); require(binding <= STB_WEAK,"unsupported ELF symbol binding");
        record.binding = binding == STB_LOCAL ? ir_model::SBM_INTERNAL : binding == STB_WEAK ? ir_model::SBM_WEAK : ir_model::SBM_STRONG;
        record.object_type = ELF64_ST_TYPE(s.st_info); record.size = s.st_size;
        if (record.object_type == STT_TLS) obj.image.tls_targets[id] = id;
        if (s.st_shndx == SHN_UNDEF || record.object_type == STT_FILE) continue;
        require(s.st_shndx < sections.size(),"unsupported ELF special symbol section");
        if (!loaded[s.st_shndx]) continue;
        const auto& sec = sections[s.st_shndx].header;
        require(s.st_value <= sec.sh_size && s.st_size <= sec.sh_size-s.st_value,"invalid ELF symbol extent");
        obj.image.defined[id] = true; obj.image.symbols[id] = offsets[s.st_shndx]+s.st_value;
        obj.image.data_symbols[id] = !(sec.sh_flags & SHF_EXECINSTR);
        if (record.object_type == STT_FUNC || record.object_type == STT_OBJECT || record.object_type == STT_TLS)
            definitions.add(sec.sh_flags & SHF_TLS ? 2 : obj.image.data_symbols[id],obj.image.symbols[id],s.st_size,id);
        if (record.object_type == STT_FUNC && binding != STB_LOCAL && std::string(begin,end) == "main") {
            record.role = ir_model::SR_ENTRY; record.parameters = 2;
        }
    }
    definitions.finish(obj);
    IdIndex fdes;
    for (auto offset : module.fdes) fdes.put(offset+8,offset+1);
    std::vector<unsigned> got(count);
    for (const auto& section : sections) {
        if (section.header.sh_type != SHT_RELA) continue;
        auto target_id = section.header.sh_info;
        require(target_id < sections.size(),"invalid ELF relocation target section");
        if (!loaded[target_id]) continue;
        const auto& target = sections[target_id].header;
        bool data = !(target.sh_flags & SHF_EXECINSTR);
        bool tls = target.sh_flags & SHF_TLS;
        auto& fixes = tls ? obj.image.tls_fixups : data ? obj.image.data_fixups : obj.image.code_fixups;
        for (const auto& rel : section.relocations) {
            auto kind = ELF64_R_TYPE(rel.r_info); if (kind == R_X86_64_NONE) continue;
            auto symbol = ELF64_R_SYM(rel.r_info); require(symbol && symbol < count,"invalid ELF relocation symbol");
            native::Fixup f; f.symbol = symbol+1; f.addend = rel.r_addend;
            unsigned width = kind == R_X86_64_64 ? 8 : 4;
            require(rel.r_offset <= target.sh_size && width <= target.sh_size-rel.r_offset,"invalid ELF relocation offset");
            f.offset = offsets[target_id]+rel.r_offset; f.owner = definitions.owner(tls ? 2 : data,f.offset);
            if (kind == R_X86_64_64) f.kind = native::Fixup::AbsoluteSymbol;
            else if (kind == R_X86_64_PC32 || kind == R_X86_64_PLT32) {
                f.kind = native::Fixup::RelativeSymbol; f.end = f.offset;
                if (kind == R_X86_64_PLT32) obj.symbols[f.symbol].object_type = STT_FUNC;
            } else if (kind == R_X86_64_32) f.kind = native::Fixup::Absolute32;
            else if (kind == R_X86_64_32S) f.kind = native::Fixup::Absolute32Signed;
            else if (kind == R_X86_64_TPOFF32) { f.kind = native::Fixup::ThreadOffset; obj.image.has_tls = true; }
            else if (kind == R_X86_64_GOTPCREL || kind == 41 || kind == 42) {
                if (!got[symbol]) {
                    auto id = obj.symbols.size(); got[symbol] = id; obj.symbols.push_back(Symbol());
                    obj.symbols.back().definition = id; obj.symbols.back().lazy = true;
                    obj.image.data.resize((obj.image.data.size()+7)&~std::size_t(7),0);
                    auto offset = obj.image.data.size(); obj.image.symbols.push_back(offset); obj.image.defined.push_back(true);
                    obj.image.data_symbols.push_back(true); obj.image.tls_targets.push_back(0);
                    native::Fixup slot; slot.kind = native::Fixup::AbsoluteSymbol;
                    slot.symbol = symbol+1; slot.offset = offset; slot.owner = id;
                    obj.image.data_fixups.push_back(slot); obj.image.data.resize(offset+8,0);
                }
                f.kind = native::Fixup::RelativeSymbol; f.end = f.offset; f.symbol = got[symbol];
            } else throw std::runtime_error("unsupported ELF relocation " + std::to_string(kind));
            fixes.push_back(f);
            if (target_id == module.unwind_section) if (auto offset = fdes.get(rel.r_offset)) {
                require(kind == R_X86_64_PC32,"unsupported FDE address encoding");
                obj.unwind.push_back({unsigned(symbol+1),rel.r_addend,offsets[target_id]+offset-1});
            }
            if (target.sh_type == SHT_INIT_ARRAY || target.sh_type == SHT_FINI_ARRAY) {
                require(kind == R_X86_64_64,"unsupported lifecycle array relocation");
                (target.sh_type == SHT_INIT_ARRAY ? obj.initializers : obj.finalizers).push_back({unsigned(symbol+1),rel.r_addend});
            }
        }
    }
    return obj;
}
} }
