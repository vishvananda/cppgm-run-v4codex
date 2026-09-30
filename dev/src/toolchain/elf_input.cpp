#include "toolchain/elf_reader.h"
namespace cppgm { namespace toolchain {
using lowir_model::require;
Object read_elf(const std::vector<unsigned char>& bytes)
{
    ElfReader r(bytes); auto h = r.get<Elf64_Ehdr>(0);
    require(h.e_ident[EI_CLASS] == ELFCLASS64 && h.e_ident[EI_DATA] == ELFDATA2LSB &&
        h.e_type == ET_REL && h.e_machine == EM_X86_64 && h.e_version == EV_CURRENT,"unsupported ELF object target/type");
    require(h.e_shentsize == sizeof(Elf64_Shdr) && h.e_shnum,"invalid ELF section table");
    r.range(h.e_shoff,std::uint64_t(h.e_shnum)*sizeof(Elf64_Shdr));
    ElfModule module(h.e_shnum,0); unsigned symtab = 0;
    for (unsigned i = 0; i < h.e_shnum; ++i) {
        auto& s = module.sections[i].header; s = r.get<Elf64_Shdr>(h.e_shoff+i*sizeof(Elf64_Shdr));
        if (s.sh_type == SHT_SYMTAB) { require(!symtab,"multiple ELF symbol tables"); symtab = i; }
    }
    require(symtab,"ELF object lacks symbol table");
    const auto& table = module.sections[symtab].header;
    require(table.sh_entsize == sizeof(Elf64_Sym) && table.sh_size % sizeof(Elf64_Sym) == 0 && table.sh_link < h.e_shnum,"invalid ELF symbols");
    module.strings = table.sh_link; r.range(table.sh_offset,table.sh_size);
    module.symbols.resize(table.sh_size/sizeof(Elf64_Sym));
    for (unsigned i = 0; i < module.symbols.size(); ++i)
        module.symbols[i] = r.get<Elf64_Sym>(table.sh_offset+i*sizeof(Elf64_Sym));
    require(h.e_shstrndx < h.e_shnum,"invalid ELF section names");
    for (unsigned i = 1; i < h.e_shnum; ++i) {
        auto& section = module.sections[i]; const auto& s = section.header;
        if (h.e_shstrndx) section.name = r.string(module.sections[h.e_shstrndx].header,s.sh_name);
        if (s.sh_flags & SHF_ALLOC || i == module.strings) {
            require(s.sh_size < 0x70000000,"ELF section too large");
            if (s.sh_type == SHT_NOBITS) section.bytes.resize(s.sh_size,0);
            else { r.range(s.sh_offset,s.sh_size); section.bytes.assign(bytes.begin()+s.sh_offset,bytes.begin()+s.sh_offset+s.sh_size); }
        }
        if (section.name == ".eh_frame") {
            module.unwind_section = i;
            for (std::uint64_t offset = 0; offset < s.sh_size;) {
                require(s.sh_size-offset >= 4,"truncated ELF CFI record");
                auto size = r.get<std::uint32_t>(s.sh_offset+offset);
                if (!size) break;
                require(size >= 4 && size <= s.sh_size-offset-4,"invalid ELF CFI record");
                if (r.get<std::uint32_t>(s.sh_offset+offset+4)) module.fdes.push_back(offset);
                offset += 4+size;
            }
        }
        if (s.sh_type != SHT_RELA && s.sh_type != SHT_REL) continue;
        require(s.sh_info < h.e_shnum,"invalid ELF relocation target section");
        if (!(module.sections[s.sh_info].header.sh_flags & SHF_ALLOC)) continue;
        require(s.sh_type == SHT_RELA && s.sh_link == symtab && s.sh_entsize == sizeof(Elf64_Rela) && s.sh_size % sizeof(Elf64_Rela) == 0,"unsupported ELF relocations");
        r.range(s.sh_offset,s.sh_size); section.relocations.reserve(s.sh_size/sizeof(Elf64_Rela));
        for (std::uint64_t at = 0; at < s.sh_size; at += sizeof(Elf64_Rela))
            section.relocations.push_back(r.get<Elf64_Rela>(s.sh_offset+at));
    }
    return link_elf(std::move(module));
}
} }
