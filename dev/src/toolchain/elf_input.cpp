#include "toolchain/object.h"
#include <elf.h>
#include <cstring>
#include <algorithm>
namespace cppgm { namespace toolchain {
using lowir_model::require;
namespace {
struct ElfReader {
    const std::vector<unsigned char>& bytes;
    explicit ElfReader(const std::vector<unsigned char>& b) : bytes(b) {}
    void range(std::uint64_t offset, std::uint64_t size) const {
        require(offset <= bytes.size() && size <= bytes.size()-offset,"invalid ELF range");
    }
    template<class T> T get(std::uint64_t offset) const {
        range(offset,sizeof(T)); T t; std::memcpy(&t,bytes.data()+offset,sizeof(T)); return t;
    }
    std::string string(const Elf64_Shdr& table, unsigned offset) const {
        require(offset < table.sh_size,"invalid ELF string offset"); range(table.sh_offset,table.sh_size);
        const char* begin = reinterpret_cast<const char*>(bytes.data()+table.sh_offset+offset);
        auto end = static_cast<const char*>(std::memchr(begin,0,table.sh_size-offset));
        require(end,"unterminated ELF string"); return std::string(begin,end);
    }
};
// ELF is an explicit input adapter. Resolve bounded symbol extents here once;
// the linker consumes identities, never guesses ownership from byte addresses.
struct ElfDefinitions {
    struct Range { std::uint64_t begin, end; unsigned owner; };
    std::vector<Range> code, data;
    void add(bool is_data, std::uint64_t begin, std::uint64_t size, unsigned owner) {
        if (size) (is_data ? data : code).push_back({begin,begin+size,owner});
    }
    void finish(Object& obj) {
        for (auto* ranges : {&code,&data}) {
            std::sort(ranges->begin(),ranges->end(),[](const Range& a,const Range& b) { return a.begin < b.begin; });
            unsigned count = 0;
            for (auto r : *ranges) {
                if (count && (*ranges)[count-1].begin == r.begin) {
                    auto& prior = (*ranges)[count-1]; prior.end = std::max(prior.end,r.end);
                    obj.symbols[r.owner].definition = prior.owner;
                } else {
                    require(!count || (*ranges)[count-1].end <= r.begin,"overlapping ELF definitions");
                    obj.symbols[r.owner].definition = r.owner;
                    (*ranges)[count++] = r;
                }
            }
            ranges->resize(count);
        }
    }
    unsigned owner(bool is_data, std::uint64_t offset) const {
        const auto& ranges = is_data ? data : code;
        auto at = std::upper_bound(ranges.begin(),ranges.end(),offset,
            [](std::uint64_t n,const Range& r) { return n < r.begin; });
        if (at == ranges.begin()) return 0;
        --at; return offset < at->end ? at->owner : 0;
    }
};
}
Object read_elf(const std::vector<unsigned char>& bytes)
{
    ElfReader r(bytes); auto h = r.get<Elf64_Ehdr>(0);
    require(h.e_ident[EI_CLASS] == ELFCLASS64 && h.e_ident[EI_DATA] == ELFDATA2LSB &&
        h.e_type == ET_REL && h.e_machine == EM_X86_64 && h.e_version == EV_CURRENT,"unsupported ELF object target/type");
    require(h.e_shentsize == sizeof(Elf64_Shdr) && h.e_shnum,"invalid ELF section table");
    r.range(h.e_shoff,std::uint64_t(h.e_shnum)*sizeof(Elf64_Shdr));
    std::vector<Elf64_Shdr> sections;
    unsigned symtab = 0;
    for (unsigned i = 0; i < h.e_shnum; ++i) {
        auto s = r.get<Elf64_Shdr>(h.e_shoff+i*sizeof(Elf64_Shdr)); sections.push_back(s);
        if (s.sh_type == SHT_SYMTAB) { require(!symtab,"multiple ELF symbol tables"); symtab = i; }
    }
    require(symtab,"ELF object lacks symbol table");
    const auto& table = sections[symtab];
    require(table.sh_entsize == sizeof(Elf64_Sym) && table.sh_size % sizeof(Elf64_Sym) == 0 && table.sh_link < sections.size(),"invalid ELF symbols");
    r.range(table.sh_offset,table.sh_size); unsigned count = table.sh_size/sizeof(Elf64_Sym);
    Object obj(count+sections.size());
    std::vector<std::uint64_t> offsets(sections.size());
    std::vector<bool> loaded(sections.size());
    for (unsigned i = 1; i < sections.size(); ++i) {
        const auto& s = sections[i]; if (!(s.sh_flags & SHF_ALLOC)) continue;
        require(!(s.sh_flags & SHF_TLS),"foreign TLS requires the hosted object ABI");
        auto alignment = std::max(std::uint64_t(1),s.sh_addralign);
        require(alignment <= 4096 && !(alignment&(alignment-1)),"unsupported ELF section alignment");
        obj.alignment = std::max(obj.alignment,unsigned(alignment));
        bool data = !(s.sh_flags & SHF_EXECINSTR);
        auto& out = data ? obj.image.data : obj.image.code;
        out.resize((out.size()+alignment-1)&~std::size_t(alignment-1),0); offsets[i] = out.size(); loaded[i] = true;
        require(s.sh_size < 0x70000000,"ELF section too large");
        if (s.sh_type == SHT_NOBITS) out.resize(out.size()+s.sh_size,0);
        else { r.range(s.sh_offset,s.sh_size); out.insert(out.end(),bytes.begin()+s.sh_offset,bytes.begin()+s.sh_offset+s.sh_size); }
        auto id = count+i+1;
        obj.image.defined[id] = true; obj.image.data_symbols[id] = data; obj.image.symbols[id] = offsets[i];
    }
    ElfDefinitions definitions;
    for (unsigned i = 1; i < count; ++i) {
        auto s = r.get<Elf64_Sym>(table.sh_offset+i*sizeof(Elf64_Sym)); auto id = i+1;
        auto& record = obj.symbols[id]; auto text = r.string(sections[table.sh_link],s.st_name);
        if (!text.empty()) record.name = obj.intern(text);
        auto binding = ELF64_ST_BIND(s.st_info);
        require(binding <= STB_WEAK,"unsupported ELF symbol binding");
        record.binding = binding == STB_LOCAL ? ir_model::SBM_INTERNAL : binding == STB_WEAK ? ir_model::SBM_WEAK : ir_model::SBM_STRONG;
        if (s.st_shndx == SHN_UNDEF) continue;
        if (ELF64_ST_TYPE(s.st_info) == STT_FILE) continue;
        require(s.st_shndx < sections.size(),"unsupported ELF special symbol section");
        if (!loaded[s.st_shndx]) continue;
        require(s.st_value <= sections[s.st_shndx].sh_size && s.st_size <= sections[s.st_shndx].sh_size-s.st_value,"invalid ELF symbol extent");
        obj.image.defined[id] = true; obj.image.symbols[id] = offsets[s.st_shndx]+s.st_value;
        obj.image.data_symbols[id] = !(sections[s.st_shndx].sh_flags & SHF_EXECINSTR);
        if (ELF64_ST_TYPE(s.st_info) == STT_FUNC || ELF64_ST_TYPE(s.st_info) == STT_OBJECT)
            definitions.add(obj.image.data_symbols[id],obj.image.symbols[id],s.st_size,id);
    }
    definitions.finish(obj);
    std::vector<unsigned> got(count);
    for (const auto& section : sections) {
        if (section.sh_type != SHT_RELA && section.sh_type != SHT_REL) continue;
        require(section.sh_info < sections.size(),"invalid ELF relocation target section");
        if (!loaded[section.sh_info]) continue;
        require(section.sh_type == SHT_RELA && section.sh_link == symtab && section.sh_entsize == sizeof(Elf64_Rela) && section.sh_size % sizeof(Elf64_Rela) == 0,"unsupported ELF relocations");
        r.range(section.sh_offset,section.sh_size);
        const auto& target = sections[section.sh_info];
        bool data = !(target.sh_flags & SHF_EXECINSTR);
        auto& fixes = data ? obj.image.data_fixups : obj.image.code_fixups;
        for (std::uint64_t pos = 0; pos < section.sh_size; pos += sizeof(Elf64_Rela)) {
            auto rel = r.get<Elf64_Rela>(section.sh_offset+pos);
            auto kind = ELF64_R_TYPE(rel.r_info); if (kind == R_X86_64_NONE) continue;
            auto symbol = ELF64_R_SYM(rel.r_info); require(symbol && symbol < count,"invalid ELF relocation symbol");
            native::Fixup f; f.symbol = symbol+1; f.addend = rel.r_addend;
            unsigned width = kind == R_X86_64_64 ? 8 : 4;
            require(rel.r_offset <= target.sh_size && width <= target.sh_size-rel.r_offset,"invalid ELF relocation offset");
            f.offset = offsets[section.sh_info]+rel.r_offset;
            f.owner = definitions.owner(data,f.offset);
            if (kind == R_X86_64_64) f.kind = native::Fixup::AbsoluteSymbol;
            else if (kind == R_X86_64_PC32 || kind == R_X86_64_PLT32) { f.kind = native::Fixup::RelativeSymbol; f.end = f.offset; }
            else if (kind == R_X86_64_32) f.kind = native::Fixup::Absolute32;
            else if (kind == R_X86_64_32S) f.kind = native::Fixup::Absolute32Signed;
            else if (kind == R_X86_64_GOTPCREL || kind == 41 || kind == 42) {
                // Keep the indirect load; no linker relaxation or instruction
                // reconstruction. One local GOT slot per imported symbol.
                if (!got[symbol]) {
                    auto id = obj.symbols.size(); got[symbol] = id;
                    obj.symbols.push_back(Symbol());
                    obj.symbols.back().definition = id; obj.symbols.back().lazy = true;
                    obj.image.data.resize((obj.image.data.size()+7)&~std::size_t(7),0);
                    auto offset = obj.image.data.size();
                    obj.image.symbols.push_back(offset); obj.image.defined.push_back(true);
                    obj.image.data_symbols.push_back(true); obj.image.tls_targets.push_back(0);
                    native::Fixup slot; slot.kind = native::Fixup::AbsoluteSymbol;
                    slot.symbol = symbol+1; slot.offset = offset; slot.owner = id;
                    obj.image.data_fixups.push_back(slot); obj.image.data.resize(offset+8,0);
                }
                f.kind = native::Fixup::RelativeSymbol; f.end = f.offset; f.symbol = got[symbol];
            } else throw std::runtime_error("unsupported ELF relocation " + std::to_string(kind));
            fixes.push_back(f);
        }
    }
    return obj;
}
} }
