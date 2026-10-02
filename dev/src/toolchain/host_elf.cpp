#include "toolchain/host_elf.h"
#include "support/id_index.h"
#include <algorithm>
#include <cstring>
#include <fstream>
namespace cppgm { namespace toolchain {
void host_number(std::vector<unsigned char>& v, std::uint64_t n, unsigned width) {
    for (unsigned k = 0; k < width; ++k) { v.push_back(n); n >>= 8; }
}
void host_uleb(std::vector<unsigned char>& v, std::uint64_t n) {
    do { auto b = n&127; n >>= 7; v.push_back(b|(n ? 128 : 0)); } while (n);
}
namespace {
unsigned string(std::vector<unsigned char>& bytes, const std::string& s) {
    unsigned at = bytes.size(); bytes.insert(bytes.end(),s.begin(),s.end()); bytes.push_back(0); return at;
}
template<class T> void append(std::vector<unsigned char>& bytes, const T& v) {
    const auto* data = reinterpret_cast<const unsigned char*>(&v); bytes.insert(bytes.end(),data,data+sizeof(v));
}
}
unsigned HostElf::symbol(const std::string& name, unsigned binding, unsigned type, unsigned section, std::uint64_t value, std::uint64_t size)
{
    Elf64_Sym s = {}; s.st_name = name.empty() ? 0 : string(sections[Strtab].bytes,name);
    s.st_info = ELF64_ST_INFO(binding,type); s.st_shndx = section; s.st_value = value; s.st_size = size;
    symbols.push_back(s); return symbols.size()-1;
}
void HostElf::relocate(unsigned section, std::size_t offset, unsigned symbol, unsigned type, std::int64_t addend)
{
    Elf64_Rela r = {}; r.r_offset = offset; r.r_info = ELF64_R_INFO(symbol,type); r.r_addend = addend;
    sections[section].relocations.push_back(r);
}
HostElf::HostElf(Object&& obj) : ElfModule(Count,Strtab), mapping(obj.symbols.size()), section_symbols(Count)
{
    unwind_section = EhFrame;
    const char* names[] = {"",".text",".data",".eh_frame",".gcc_except_table",".data.rel.local",".init_array",".fini_array",".tdata",
        ".rela.text",".rela.data",".rela.eh_frame",".rela.gcc_except_table",".rela.data.rel.local",".rela.init_array",".rela.fini_array",".rela.tdata",
        ".symtab",".strtab",".shstrtab",".note.GNU-stack"};
    sections[Strtab].bytes.push_back(0); sections[Shstrtab].bytes.push_back(0);
    for (unsigned n = 1; n < Count; ++n) {
        auto& s = sections[n]; s.name = names[n]; s.header.sh_name = string(sections[Shstrtab].bytes,s.name);
        s.header.sh_type = SHT_PROGBITS; s.header.sh_addralign = 1;
        if (n >= Text && n <= Tdata) { s.header.sh_flags = SHF_ALLOC; s.header.sh_addralign = 8; }
        if (n == Text) { s.header.sh_flags |= SHF_EXECINSTR; s.header.sh_addralign = 16; }
        if (n == Data || n == Refs || n == Init || n == Fini) s.header.sh_flags |= SHF_WRITE;
        if (n == Data) s.header.sh_addralign = obj.alignment;
        if (n == Tdata) { s.header.sh_flags |= SHF_WRITE|SHF_TLS; s.header.sh_addralign = obj.image.tls_alignment; }
        if (n == Init || n == Fini) { s.header.sh_type = n == Init ? SHT_INIT_ARRAY : SHT_FINI_ARRAY; s.header.sh_entsize = 8; }
        if (n >= RelaText && n <= RelaTdata) {
            s.header.sh_type = SHT_RELA; s.header.sh_link = Symtab; s.header.sh_info = n-RelaText+Text;
            s.header.sh_addralign = 8; s.header.sh_entsize = sizeof(Elf64_Rela);
        }
        if (n == Symtab) { s.header.sh_type = SHT_SYMTAB; s.header.sh_link = Strtab; s.header.sh_addralign = 8; s.header.sh_entsize = sizeof(Elf64_Sym); }
        if (n == Strtab || n == Shstrtab) s.header.sh_type = SHT_STRTAB;
        if (n <= Tdata) section_symbols[n] = symbol("",STB_LOCAL,STT_SECTION,n,0);
    }
    // Native bytes have one owner across this phase boundary. Relocation and
    // unwind records below consume offsets, so they do not need old buffers.
    sections[Text].bytes = std::move(obj.image.code);
    sections[Data].bytes = std::move(obj.image.data);
    sections[Tdata].bytes = std::move(obj.image.tls);
    sections[Lsda].bytes = std::move(obj.image.lsda);
    std::vector<bool> used(obj.symbols.size()); std::vector<std::uint64_t> sizes(obj.symbols.size());
    for (unsigned i = 1; i < sizes.size(); ++i) sizes[i] = obj.symbols[i].size;
    for (const auto* fixes : {&obj.image.code_fixups,&obj.image.data_fixups,&obj.image.tls_fixups,&obj.image.lsda_fixups})
        for (const auto& fix : *fixes) used[fix.symbol] = true;
    for (const auto& u : obj.image.unwind) sizes[u.symbol] = obj.symbols[u.symbol].size = u.end-u.begin;
    place(obj);
    IdIndex exports;
    // Output order cannot depend on the source/reader's symbol insertion order.
    // Compare interned spelling views only at this final emission boundary.
    std::vector<unsigned> symbol_order;
    for (unsigned i = 1; i < obj.symbols.size(); ++i) if (used[i] || obj.image.defined[i]) symbol_order.push_back(i);
    std::stable_sort(symbol_order.begin(),symbol_order.end(),[&](unsigned a, unsigned b) {
        auto x = obj.symbols[a].name ? obj.names.spelling(obj.symbols[a].name) : TextView("",0);
        auto y = obj.symbols[b].name ? obj.names.spelling(obj.symbols[b].name) : TextView("",0);
        return std::lexicographical_compare(x.data,x.data+x.size,y.data,y.data+y.size);
    });
    for (unsigned i : symbol_order) {
        if (!used[i] && !obj.image.defined[i]) continue;
        const auto& s = obj.symbols[i];
        auto binding = s.binding == ir_model::SBM_INTERNAL ? STB_LOCAL : s.binding == ir_model::SBM_WEAK ? STB_WEAK : STB_GLOBAL;
        bool defined = obj.image.defined[i];
        bool tls = obj.image.tls_targets[i] == i;
        unsigned section = placement[i].section;
        unsigned type = tls ? STT_TLS : defined ? (obj.image.data_symbols[i] ? STT_OBJECT : STT_FUNC) : STT_NOTYPE;
        if (binding != STB_LOCAL && s.name) if (auto old = exports.get(s.name)) {
            mapping[i] = old;
            lowir_model::require(!defined || !symbols[old].st_shndx,"duplicate host object definition");
            if (defined) { symbols[old].st_shndx = section; symbols[old].st_value = placement[i].offset; symbols[old].st_size = sizes[i]; symbols[old].st_info = ELF64_ST_INFO(binding,type); }
            continue;
        }
        lowir_model::require(s.name || !used[i],"host relocation has no symbol identity");
        mapping[i] = symbol(s.name ? obj.name(i) : "",binding,type,
            defined ? section : 0,placement[i].offset,sizes[s.definition ? s.definition : i]);
        if (binding != STB_LOCAL && s.name) exports.put(s.name,mapping[i]);
        if (defined && (s.role == ir_model::SR_INIT || s.role == ir_model::SR_FINI)) {
            unsigned section = s.role == ir_model::SR_INIT ? Init : Fini;
            relocate(section == Init ? RelaInit : RelaFini,sections[section].bytes.size(),mapping[i],R_X86_64_64);
            host_number(sections[section].bytes,0,8);
        }
    }
    for (unsigned lane = 0; lane < 3; ++lane) {
        const auto& fixes = lane == 2 ? obj.image.tls_fixups : lane ? obj.image.data_fixups : obj.image.code_fixups;
        for (const auto& f : fixes) {
            unsigned type = f.kind == native::Fixup::AbsoluteSymbol ? R_X86_64_64 :
                f.kind == native::Fixup::Absolute32 ? R_X86_64_32 : f.kind == native::Fixup::Absolute32Signed ? R_X86_64_32S : R_X86_64_PC32;
            auto addend = f.addend;
            if (f.kind == native::Fixup::RelativeSymbol || f.kind == native::Fixup::CallSymbol || f.kind == native::Fixup::GotSymbol) {
                addend += std::int64_t(f.offset)-std::int64_t(f.end);
                if (f.kind == native::Fixup::CallSymbol) type = R_X86_64_PLT32;
                if (f.kind == native::Fixup::GotSymbol) type = R_X86_64_GOTPCREL;
            }
            if (f.kind == native::Fixup::ThreadOffset) type = R_X86_64_TPOFF32;
            auto owner = f.owner;
            unsigned target = owner ? placement[owner].section : lane == 2 ? Tdata : lane ? Data : Text;
            auto offset = owner ? f.offset-obj.image.symbols[owner]+placement[owner].offset : f.offset;
            relocate(relocation_section(target),offset,mapping[f.symbol],type,addend);
        }
    }
    for (const auto& g : groups) sections[g.section].header.sh_info = mapping[g.owner];
    unwind(obj);
    if (sections[Lsda].bytes.empty()) sections[Lsda].header.sh_name = 0;
    if (sections[EhFrame].bytes.empty()) sections[EhFrame].header.sh_name = 0;
}
void HostElf::write(const std::string& path)
{
    // ELF requires local symbols first. Reindex relocations once after all
    // indirect references and unwind records have published their symbols.
    std::vector<unsigned> order(symbols.size()), renumber(symbols.size());
    for (unsigned n = 0; n < order.size(); ++n) order[n] = n;
    auto first = std::stable_partition(order.begin(),order.end(),[&](unsigned n) { return ELF64_ST_BIND(symbols[n].st_info) == STB_LOCAL; });
    sections[Symtab].header.sh_info = first-order.begin();
    for (unsigned n = 0; n < order.size(); ++n) { renumber[order[n]] = n; append(sections[Symtab].bytes,symbols[order[n]]); }
    for (unsigned n = 1; n < sections.size(); ++n) {
        if (sections[n].header.sh_type == SHT_GROUP) sections[n].header.sh_info = renumber[sections[n].header.sh_info];
        for (auto r : sections[n].relocations) {
            r.r_info = ELF64_R_INFO(renumber[ELF64_R_SYM(r.r_info)],ELF64_R_TYPE(r.r_info)); append(sections[n].bytes,r);
        }
    }
    Elf64_Ehdr h = {}; std::memcpy(h.e_ident,ELFMAG,SELFMAG); h.e_ident[EI_CLASS] = ELFCLASS64; h.e_ident[EI_DATA] = ELFDATA2LSB; h.e_ident[EI_VERSION] = EV_CURRENT;
    lowir_model::require(sections.size() < SHN_LORESERVE,"too many ELF sections");
    h.e_type = ET_REL; h.e_machine = EM_X86_64; h.e_version = EV_CURRENT; h.e_ehsize = sizeof(h); h.e_shentsize = sizeof(Elf64_Shdr); h.e_shnum = sections.size(); h.e_shstrndx = Shstrtab;
    std::uint64_t offset = sizeof(h);
    for (unsigned n = 1; n < sections.size(); ++n) {
        auto& s = sections[n]; auto a = s.header.sh_addralign;
        offset = (offset+a-1)&~(a-1); s.header.sh_offset = offset; s.header.sh_size = s.bytes.size();
        offset += s.bytes.size();
    }
    h.e_shoff = (offset+7)&~std::uint64_t(7);
    // Layout is complete before writing. Stream each section exactly once;
    // no second whole-object byte vector is needed to patch the ELF header.
    std::ofstream out(path,std::ios::binary);
    lowir_model::require(bool(out),"cannot create host ELF object");
    out.write(reinterpret_cast<const char*>(&h),sizeof(h)); offset = sizeof(h);
    const char zeros[4096] = {};
    auto pad = [&](std::uint64_t end) {
        while (offset < end) {
            auto count = std::min<std::uint64_t>(end-offset,sizeof(zeros));
            out.write(zeros,count); offset += count;
        }
    };
    for (unsigned n = 1; n < sections.size(); ++n) {
        const auto& s = sections[n]; pad(s.header.sh_offset);
        out.write(reinterpret_cast<const char*>(s.bytes.data()),s.bytes.size()); offset += s.bytes.size();
    }
    pad(h.e_shoff);
    for (const auto& s : sections) out.write(reinterpret_cast<const char*>(&s.header),sizeof(s.header));
    out.close(); lowir_model::require(bool(out),"cannot write host ELF object");
}
void write_host_object(Object&& obj, const std::string& path) { HostElf(std::move(obj)).write(path); }
Object host_link_object(Object&& obj) { return link_elf(HostElf(std::move(obj))); }
} }
