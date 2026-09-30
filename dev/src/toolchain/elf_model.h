#pragma once
#include "toolchain/object.h"
#include <elf.h>
namespace cppgm { namespace toolchain {
// Typed section/symbol/relocation records are shared by native object emission
// and linking. Only the explicit file adapter reads serialized ELF records.
struct HostSection {
    std::string name;
    Elf64_Shdr header = {};
    std::vector<unsigned char> bytes;
    std::vector<Elf64_Rela> relocations;
};
struct ElfModule {
    std::vector<HostSection> sections;
    std::vector<Elf64_Sym> symbols = std::vector<Elf64_Sym>(1);
    unsigned strings;
    unsigned unwind_section = 0;
    std::vector<std::size_t> fdes;
    ElfModule(unsigned count, unsigned strtab) : sections(count), strings(strtab) {}
};
Object link_elf(ElfModule&&);
Object host_link_object(Object&&);
} }
