#include "toolchain/dynamic.h"
#include "toolchain/elf_reader.h"
#include "toolchain/host_config.h"
#include "support/id_index.h"
#include <fstream>
namespace cppgm { namespace toolchain {
std::vector<DynamicImport> host_imports(const std::vector<unsigned>& requested,
    const std::vector<Symbol>& symbols, const IdentifierTable& names, std::vector<std::string>& libraries)
{
    // Index only the live unresolved names. Read each bounded host ABI symbol
    // table once; never invoke or load a compiler, linker, or DSO as a plugin.
    IdentifierTable needed; IdIndex requests;
    std::vector<DynamicImport> result(requested.size());
    for (unsigned i = 0; i < requested.size(); ++i) {
        result[i].symbol = requested[i];
        lowir_model::require(symbols[requested[i]].name,"host import lacks an ABI name");
        auto text = names.spelling(symbols[requested[i]].name);
        requests.put(needed.intern(text),i+1);
    }
    auto paths = host_library_paths();
    unsigned remaining = result.size();
    for (const char* library : {"libstdc++.so.6","libgcc_s.so.1","libc.so.6","libm.so.6"}) {
        if (!remaining) break;
        std::ifstream input;
        for (const auto& directory : paths) {
            input.open(directory+"/"+library,std::ios::binary|std::ios::ate);
            if (input) break;
            input.clear();
        }
        lowir_model::require(bool(input),"cannot locate host runtime library");
        auto size = input.tellg(); lowir_model::require(size >= 0 && size < 0x70000000,"invalid host runtime size");
        std::vector<unsigned char> bytes(static_cast<std::size_t>(size)); input.seekg(0);
        input.read(reinterpret_cast<char*>(bytes.data()),bytes.size());
        lowir_model::require(bool(input),"cannot read host runtime library");
        ElfReader reader(bytes); auto h = reader.get<Elf64_Ehdr>(0);
        lowir_model::require(!std::memcmp(h.e_ident,ELFMAG,SELFMAG) && h.e_ident[EI_CLASS] == ELFCLASS64 && h.e_ident[EI_DATA] == ELFDATA2LSB &&
            h.e_type == ET_DYN && h.e_machine == EM_X86_64 && h.e_shentsize == sizeof(Elf64_Shdr),"invalid host runtime ELF");
        reader.range(h.e_shoff,std::uint64_t(h.e_shnum)*sizeof(Elf64_Shdr));
        std::vector<Elf64_Shdr> sections;
        unsigned table = 0, versions = 0;
        for (unsigned i = 0; i < h.e_shnum; ++i) {
            auto s = reader.get<Elf64_Shdr>(h.e_shoff+i*sizeof(Elf64_Shdr)); sections.push_back(s);
            if (s.sh_type == SHT_DYNSYM) table = i;
            if (s.sh_type == SHT_GNU_versym) versions = i;
        }
        lowir_model::require(table && sections[table].sh_link < sections.size(),"host runtime lacks dynamic symbols");
        auto t = sections[table];
        lowir_model::require(t.sh_entsize == sizeof(Elf64_Sym) && t.sh_size%sizeof(Elf64_Sym) == 0,"invalid dynamic symbols");
        reader.range(t.sh_offset,t.sh_size);
        if (versions) {
            const auto& v = sections[versions];
            lowir_model::require(v.sh_link == table && v.sh_size/2 >= t.sh_size/sizeof(Elf64_Sym),"invalid dynamic symbol versions");
            reader.range(v.sh_offset,v.sh_size);
        }
        bool used = false;
        for (std::size_t i = 1; i < t.sh_size/sizeof(Elf64_Sym); ++i) {
            auto s = reader.get<Elf64_Sym>(t.sh_offset+i*sizeof(Elf64_Sym));
            if (!s.st_shndx || ELF64_ST_BIND(s.st_info) == STB_LOCAL || ELF64_ST_VISIBILITY(s.st_other) == STV_HIDDEN) continue;
            if (versions && (reader.get<std::uint16_t>(sections[versions].sh_offset+2*i)&0x8000)) continue;
            auto id = requests.get(needed.find(reader.text(sections[t.sh_link],s.st_name)));
            if (!id || result[id-1].type) continue;
            auto type = ELF64_ST_TYPE(s.st_info);
            if (type != STT_FUNC && type != STT_GNU_IFUNC && type != STT_OBJECT) continue;
            result[id-1].type = type; result[id-1].size = s.st_size; used = true; --remaining;
        }
        if (used) libraries.push_back(library);
    }
    for (const auto& symbol : result) if (!symbol.type) {
        auto name = names.spelling(symbols[symbol.symbol].name);
        throw std::runtime_error("unresolved native symbol: " + std::string(name.data,name.size));
    }
    return result;
}
} }
