#pragma once
#include "toolchain/object.h"
#include <elf.h>
namespace cppgm { namespace toolchain {
struct HostSection {
    std::string name;
    Elf64_Shdr header = {};
    std::vector<unsigned char> bytes;
};
struct HostElf {
    enum Section { Null, Text, Data, EhFrame, Lsda, Refs, Init, Fini,
        RelaText, RelaData, RelaEh, RelaLsda, RelaRefs, RelaInit, RelaFini, Symtab, Strtab, Shstrtab, Stack, Count };
    std::vector<HostSection> sections = std::vector<HostSection>(Count);
    std::vector<Elf64_Sym> symbols = std::vector<Elf64_Sym>(1);
    std::vector<unsigned> mapping, section_symbols;
    explicit HostElf(Object&&);
    unsigned symbol(const std::string&, unsigned binding, unsigned type, unsigned section, std::uint64_t value, std::uint64_t size = 0);
    void relocate(unsigned section, std::size_t offset, unsigned symbol, unsigned type, std::int64_t addend = 0);
    void unwind(const Object&);
    void write(const std::string&);
};
void host_number(std::vector<unsigned char>&, std::uint64_t, unsigned);
void host_uleb(std::vector<unsigned char>&, std::uint64_t);
} }
