#pragma once

// Typed ABI naming API. This compact graph replaces the optional owning-record
// scaffold: integrated clients construct Graph/Target directly and call mangle.
// Fact text is an explicit standalone input/output adapter, never phase transport.
#include "abi/itanium/graph.h"
#include <iosfwd>

namespace abi_mangle {
struct AbiFactFile {
    Graph graph;
    std::vector<Target> cases;
};
AbiFactFile parse_fact_text(const std::string& text);
void parse_fact_stream(std::istream& input, std::ostream& output);
std::string mangle_fact_file(AbiFactFile& file);
std::string serialize_fact_file(const AbiFactFile& file);
std::string mangle_fact_files(const std::vector<std::string>& paths);
} // namespace abi_mangle
