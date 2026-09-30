#pragma once
#include "toolchain/elf_model.h"
namespace cppgm { namespace toolchain {
struct HostElf : ElfModule {
    enum Section { Null, Text, Data, EhFrame, Lsda, Refs, Init, Fini, Tdata,
        RelaText, RelaData, RelaEh, RelaLsda, RelaRefs, RelaInit, RelaFini, RelaTdata, Symtab, Strtab, Shstrtab, Stack, Count };
    std::vector<unsigned> mapping, section_symbols;
    struct Placement { unsigned section = 0; std::uint64_t offset = 0; };
    std::vector<Placement> placement;
    std::vector<unsigned> relocation_sections, section_groups;
    struct Group { unsigned section, owner; };
    std::vector<Group> groups;
    explicit HostElf(Object&&);
    unsigned symbol(const std::string&, unsigned binding, unsigned type, unsigned section, std::uint64_t value, std::uint64_t size = 0);
    void relocate(unsigned section, std::size_t offset, unsigned symbol, unsigned type, std::int64_t addend = 0);
    void unwind(const Object&);
    unsigned section(const std::string&, unsigned type, std::uint64_t flags, unsigned alignment);
    unsigned relocation_section(unsigned);
    void place(const Object&);
    void write(const std::string&);
};
void host_number(std::vector<unsigned char>&, std::uint64_t, unsigned);
void host_uleb(std::vector<unsigned char>&, std::uint64_t);
} }
