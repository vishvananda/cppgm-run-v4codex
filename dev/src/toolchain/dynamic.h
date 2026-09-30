#pragma once
#include "toolchain/object.h"
namespace cppgm { namespace toolchain {
struct DynamicImport {
    unsigned symbol = 0, type = 0;
    std::uint64_t size = 0;
};
std::vector<DynamicImport> host_imports(const std::vector<unsigned>&,
    const std::vector<Symbol>&, const IdentifierTable&, std::vector<std::string>&);
std::size_t write_dynamic_executable(native::Image&, const std::vector<Symbol>&,
    const IdentifierTable&, const std::vector<DynamicImport>&, const std::vector<std::string>&,
    const std::vector<ObjectUnwind>&, const std::vector<ObjectReference>&,
    const std::vector<ObjectReference>&, std::size_t frame_begin, unsigned entry, unsigned startup, const std::string&);
} }
