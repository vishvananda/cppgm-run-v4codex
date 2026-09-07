#pragma once

// Typed ABI naming API. This compact graph replaces the optional owning-record
// scaffold: integrated clients construct Graph/Target directly and call mangle.
// Fact text is an explicit standalone input/output adapter, never phase transport.
#include "abi/itanium/graph.h"
#include <iosfwd>

namespace abi_mangle {
struct RunStats {
    std::uint64_t cases = 0, records = 0, source_bytes = 0, output_bytes = 0;
    std::uint64_t graph_nodes = 0, peak_graph_bytes = 0;
    std::uint64_t intern_requests = 0, intern_hits = 0, intern_probes = 0;
    std::uint64_t substitution_lookups = 0, substitution_hits = 0, substitutions = 0;
    std::uint64_t emitted_nodes = 0, parse_nanoseconds = 0, encode_nanoseconds = 0;
};
struct AbiFactFile {
    Graph graph;
    std::vector<Target> cases;
};
AbiFactFile parse_fact_text(const std::string& text);
void parse_fact_stream(std::istream& input, std::ostream& output, RunStats* stats = nullptr);
void mangle_fact_files_to_stream(const std::vector<std::string>& paths,
                                 std::ostream& output, RunStats* stats = nullptr);
void print_stats(std::ostream& output, const RunStats& stats);
std::string mangle_fact_file(AbiFactFile& file);
std::string serialize_fact_file(const AbiFactFile& file);
std::string mangle_fact_files(const std::vector<std::string>& paths);
} // namespace abi_mangle
