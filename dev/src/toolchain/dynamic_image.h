#pragma once
#include "toolchain/dynamic.h"
#include <elf.h>
namespace cppgm { namespace toolchain {
struct DynamicImage {
    enum Section { Null, Text, Data, Tdata, Hash, Symbols, Strings, Relocations, Dynamic, EhHeader, Shstrings, Count };
    native::Image& image;
    const std::vector<Symbol>& symbols;
    const IdentifierTable& names;
    std::vector<Elf64_Sym> dynsym = std::vector<Elf64_Sym>(1);
    std::vector<Elf64_Rela> relocations;
    std::vector<unsigned> dynamic_ids, import_types;
    std::vector<bool> copies;
    std::vector<unsigned char> strings = std::vector<unsigned char>(1);
    std::vector<Elf64_Dyn> dynamic;
    std::vector<Elf64_Shdr> sections = std::vector<Elf64_Shdr>(Count);
    std::vector<unsigned char> section_names = std::vector<unsigned char>(1);
    struct Stub { std::size_t displacement, slot; };
    std::vector<Stub> stubs;
    static constexpr std::uint64_t base = 0x400000, code_offset = 4096;
    std::uint64_t data_offset = 0, entry_offset = 0, hash_offset = 0;
    std::uint64_t init_offset = 0, fini_offset = 0;
    std::size_t tls_offset = 0, tls_size = 0;
    unsigned init_count = 0, fini_count = 0;
    DynamicImage(native::Image&, const std::vector<Symbol>&, const IdentifierTable&);
    std::uint64_t address(unsigned) const;
    unsigned string(const std::string&);
    void imports(const std::vector<DynamicImport>&);
    void startup(unsigned main, unsigned runtime);
    void arrays(const std::vector<ObjectReference>&, bool fini);
    void patch();
    void metadata(const std::vector<std::string>&, const std::vector<ObjectUnwind>&, std::size_t frame_begin);
    void write(const std::string&);
};
void dynamic_number(std::vector<unsigned char>&, std::uint64_t, unsigned);
void dynamic_patch(std::vector<unsigned char>&, std::size_t, std::uint64_t, unsigned);
} }
