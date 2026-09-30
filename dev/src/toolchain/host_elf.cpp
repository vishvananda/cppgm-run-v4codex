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
    append(sections[section].bytes,r);
}
HostElf::HostElf(const Object& obj) : mapping(obj.symbols.size()), section_symbols(Count)
{
    const char* names[] = {"",".text",".data",".eh_frame",".gcc_except_table",".data.rel.local",".init_array",".fini_array",
        ".rela.text",".rela.data",".rela.eh_frame",".rela.gcc_except_table",".rela.data.rel.local",".rela.init_array",".rela.fini_array",
        ".symtab",".strtab",".shstrtab",".note.GNU-stack"};
    sections[Strtab].bytes.push_back(0); sections[Shstrtab].bytes.push_back(0);
    for (unsigned n = 1; n < Count; ++n) {
        auto& s = sections[n]; s.name = names[n]; s.header.sh_name = string(sections[Shstrtab].bytes,s.name);
        s.header.sh_type = SHT_PROGBITS; s.header.sh_addralign = 1;
        if (n >= Text && n <= Fini) { s.header.sh_flags = SHF_ALLOC; s.header.sh_addralign = 8; }
        if (n == Text) { s.header.sh_flags |= SHF_EXECINSTR; s.header.sh_addralign = 16; }
        if (n == Data || n == Refs || n == Init || n == Fini) s.header.sh_flags |= SHF_WRITE;
        if (n == Data) s.header.sh_addralign = obj.alignment;
        if (n == Init || n == Fini) { s.header.sh_type = n == Init ? SHT_INIT_ARRAY : SHT_FINI_ARRAY; s.header.sh_entsize = 8; }
        if (n >= RelaText && n <= RelaFini) {
            s.header.sh_type = SHT_RELA; s.header.sh_link = Symtab; s.header.sh_info = n-RelaText+Text;
            s.header.sh_addralign = 8; s.header.sh_entsize = sizeof(Elf64_Rela);
        }
        if (n == Symtab) { s.header.sh_type = SHT_SYMTAB; s.header.sh_link = Strtab; s.header.sh_addralign = 8; s.header.sh_entsize = sizeof(Elf64_Sym); }
        if (n == Strtab || n == Shstrtab) s.header.sh_type = SHT_STRTAB;
        if (n <= Fini) section_symbols[n] = symbol("",STB_LOCAL,STT_SECTION,n,0);
    }
    sections[Text].bytes = obj.image.code; sections[Data].bytes = obj.image.data; sections[Lsda].bytes = obj.image.lsda;
    std::vector<bool> used(obj.symbols.size()); std::vector<std::uint64_t> sizes(obj.symbols.size());
    for (const auto* fixes : {&obj.image.code_fixups,&obj.image.data_fixups,&obj.image.lsda_fixups})
        for (const auto& fix : *fixes) used[fix.symbol] = true;
    for (const auto& u : obj.image.unwind) sizes[u.symbol] = u.end-u.begin;
    IdIndex exports;
    for (unsigned i = 1; i < obj.symbols.size(); ++i) {
        if (!used[i] && !obj.image.defined[i]) continue;
        const auto& s = obj.symbols[i];
        auto binding = s.binding == ir_model::SBM_INTERNAL ? STB_LOCAL : s.binding == ir_model::SBM_WEAK ? STB_WEAK : STB_GLOBAL;
        bool defined = obj.image.defined[i];
        if (binding != STB_LOCAL && s.name) if (auto old = exports.get(s.name)) {
            mapping[i] = old;
            lowir_model::require(!defined || !symbols[old].st_shndx,"duplicate host object definition");
            if (defined) { symbols[old].st_shndx = obj.image.data_symbols[i] ? Data : Text; symbols[old].st_value = obj.image.symbols[i]; symbols[old].st_size = sizes[i]; }
            continue;
        }
        lowir_model::require(s.name || !used[i],"host relocation has no symbol identity");
        mapping[i] = symbol(s.name ? obj.name(i) : "",binding,defined ? (obj.image.data_symbols[i] ? STT_OBJECT : STT_FUNC) : STT_NOTYPE,
            defined ? (obj.image.data_symbols[i] ? Data : Text) : 0,obj.image.symbols[i],sizes[s.definition ? s.definition : i]);
        if (binding != STB_LOCAL && s.name) exports.put(s.name,mapping[i]);
        if (defined && (s.role == ir_model::SR_INIT || s.role == ir_model::SR_FINI)) {
            unsigned section = s.role == ir_model::SR_INIT ? Init : Fini;
            relocate(section == Init ? RelaInit : RelaFini,sections[section].bytes.size(),mapping[i],R_X86_64_64);
            host_number(sections[section].bytes,0,8);
        }
    }
    for (unsigned lane = 0; lane < 2; ++lane) {
        const auto& fixes = lane ? obj.image.data_fixups : obj.image.code_fixups;
        for (const auto& f : fixes) {
            unsigned type = f.kind == native::Fixup::AbsoluteSymbol ? R_X86_64_64 :
                f.kind == native::Fixup::Absolute32 ? R_X86_64_32 : f.kind == native::Fixup::Absolute32Signed ? R_X86_64_32S : R_X86_64_PC32;
            auto addend = f.addend;
            if ((f.kind == native::Fixup::RelativeSymbol || f.kind == native::Fixup::CallSymbol)) {
                addend += std::int64_t(f.offset)-std::int64_t(f.end);
                if (f.kind == native::Fixup::CallSymbol) type = R_X86_64_PLT32;
            }
            lowir_model::require(f.kind != native::Fixup::ThreadOffset,"host TLS model is not implemented");
            relocate(lane ? RelaData : RelaText,f.offset,mapping[f.symbol],type,addend);
        }
    }
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
    for (unsigned n = RelaText; n <= RelaFini; ++n) for (std::size_t at = 0; at < sections[n].bytes.size(); at += sizeof(Elf64_Rela)) {
        Elf64_Rela r; std::memcpy(&r,sections[n].bytes.data()+at,sizeof(r));
        r.r_info = ELF64_R_INFO(renumber[ELF64_R_SYM(r.r_info)],ELF64_R_TYPE(r.r_info)); std::memcpy(sections[n].bytes.data()+at,&r,sizeof(r));
    }
    Elf64_Ehdr h = {}; std::memcpy(h.e_ident,ELFMAG,SELFMAG); h.e_ident[EI_CLASS] = ELFCLASS64; h.e_ident[EI_DATA] = ELFDATA2LSB; h.e_ident[EI_VERSION] = EV_CURRENT;
    h.e_type = ET_REL; h.e_machine = EM_X86_64; h.e_version = EV_CURRENT; h.e_ehsize = sizeof(h); h.e_shentsize = sizeof(Elf64_Shdr); h.e_shnum = Count; h.e_shstrndx = Shstrtab;
    std::vector<unsigned char> bytes(sizeof(h));
    for (unsigned n = 1; n < Count; ++n) {
        auto& s = sections[n]; auto a = s.header.sh_addralign;
        bytes.resize((bytes.size()+a-1)&~(a-1),0); s.header.sh_offset = bytes.size(); s.header.sh_size = s.bytes.size();
        bytes.insert(bytes.end(),s.bytes.begin(),s.bytes.end());
    }
    bytes.resize((bytes.size()+7)&~std::size_t(7),0); h.e_shoff = bytes.size();
    for (const auto& s : sections) append(bytes,s.header);
    std::memcpy(bytes.data(),&h,sizeof(h));
    std::ofstream out(path,std::ios::binary); out.write(reinterpret_cast<const char*>(bytes.data()),bytes.size()); out.close(); lowir_model::require(bool(out),"cannot write host ELF object");
}
void write_host_object(const Object& obj, const std::string& path) { HostElf(obj).write(path); }
} }
