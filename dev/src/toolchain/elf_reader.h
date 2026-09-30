#pragma once
#include "toolchain/elf_model.h"
#include <cstring>
namespace cppgm { namespace toolchain {
struct ElfReader {
    const std::vector<unsigned char>& bytes;
    explicit ElfReader(const std::vector<unsigned char>& b) : bytes(b) {}
    void range(std::uint64_t offset, std::uint64_t size) const {
        lowir_model::require(offset <= bytes.size() && size <= bytes.size()-offset,"invalid ELF range");
    }
    template<class T> T get(std::uint64_t offset) const {
        range(offset,sizeof(T)); T t; std::memcpy(&t,bytes.data()+offset,sizeof(T)); return t;
    }
    TextView text(const Elf64_Shdr& table, unsigned offset) const {
        lowir_model::require(offset < table.sh_size,"invalid ELF string offset"); range(table.sh_offset,table.sh_size);
        const char* begin = reinterpret_cast<const char*>(bytes.data()+table.sh_offset+offset);
        auto end = static_cast<const char*>(std::memchr(begin,0,table.sh_size-offset));
        lowir_model::require(end,"unterminated ELF string"); return {begin,std::size_t(end-begin)};
    }
    std::string string(const Elf64_Shdr& table, unsigned offset) const {
        auto view = text(table,offset); return std::string(view.data,view.size);
    }
};
} }
