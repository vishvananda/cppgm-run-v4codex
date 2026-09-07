#include "abi/itanium/fact_reader.h"
#include <cctype>
#include <chrono>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace abi_mangle {
Words split_words(const std::string& line) {
    Words words; std::istringstream input(line.substr(0, line.find('#')));
    std::string word;
    while (input >> word) words.push_back(word);
    return words;
}
const std::string& take(const Words& words, std::size_t& pos) {
    if (pos == words.size()) throw std::runtime_error("missing ABI fact operand");
    return words[pos++];
}
void exhausted(const Words& words, std::size_t pos) {
    if (pos != words.size()) throw std::runtime_error("extra ABI fact operand");
}
std::uint64_t index_value(const std::string& word) {
    if (word.empty()) throw std::runtime_error("empty ABI index");
    std::uint64_t value = 0;
    for (unsigned char c : word) {
        if (!std::isdigit(c) || value > (std::numeric_limits<std::uint64_t>::max() - (c - '0')) / 10)
            throw std::runtime_error("invalid ABI index: " + word);
        value = value * 10 + c - '0';
    }
    return value;
}
std::uint64_t integral_value(const std::string& word) {
    if (!word.empty() && word[0] == '-') {
        std::uint64_t magnitude = index_value(word.substr(1));
        if (magnitude > (1ull << 63)) throw std::runtime_error("signed ABI value out of range");
        return 0 - magnitude;
    }
    return index_value(word);
}
bool boolean(const std::string& word) {
    if (word == "yes" || word == "true" || word == "1") return true;
    if (word == "no" || word == "false" || word == "0") return false;
    throw std::runtime_error("invalid ABI boolean");
}
unsigned qualifier(const std::string& word) {
    if (word == "const") return 1;
    if (word == "volatile") return 2;
    if (word == "lvalue-ref") return 4;
    if (word == "rvalue-ref") return 8;
    throw std::runtime_error("invalid ABI function qualifier");
}
Binding FactReader::lookup(const std::string& name) {
    Id key = g.string(name);
    if (key >= bindings.size()) return {};
    return bindings[key];
}
Id FactReader::reference(const std::string& name, BindingKind kind) {
    Binding b = lookup(name);
    if (b.kind != kind) throw std::runtime_error("invalid ABI fact reference: " + name);
    return b.id;
}
void FactReader::bind(const std::string& name, BindingKind kind, Id id) {
    Id key = g.string(name);
    if (key >= bindings.size()) bindings.resize(key + 1);
    if (bindings[key].kind != BindingKind::None)
        throw std::runtime_error("duplicate ABI definition: " + name);
    bindings[key].kind = kind; bindings[key].id = id;
}
std::vector<Id> FactReader::refs(const Words& w, std::size_t& p, BindingKind kind) {
    std::vector<Id> ids;
    while (p < w.size()) ids.push_back(reference(take(w, p), kind));
    return ids;
}
void FactReader::record(const Words& w) {
    if (w.empty()) return;
    std::size_t p = 1;
    if (w[0].compare(0, 4, "let-") == 0) {
        const std::string name = take(w, p);
        // Reject before construction, including cross-kind duplicate binders.
        if (lookup(name).kind != BindingKind::None)
            throw std::runtime_error("duplicate ABI definition: " + name);
        Id id; BindingKind kind;
        if (w[0] == "let-type") { kind = BindingKind::Type; id = type(w, p); }
        else if (w[0] == "let-arg") { kind = BindingKind::Argument; id = argument(w, p); }
        else if (w[0] == "let-expr") { kind = BindingKind::Expression; id = expression(w, p); }
        else if (w[0] == "let-context") { kind = BindingKind::Context; id = context(w, p); }
        else if (w[0] == "let-entity") { kind = BindingKind::Entity; id = entity(w, p); }
        else throw std::runtime_error("unknown ABI definition");
        exhausted(w, p); bind(name, kind, id); return;
    }
    if (!target_seen) target_record(w);
    else function_record(w);
}
Target FactReader::finish() {
    if (!target_seen) throw std::runtime_error("ABI case has no target");
    if ((target.function.qualifiers & 12) == 12)
        throw std::runtime_error("conflicting ABI ref qualifiers");
    return std::move(target);
}
void FactReader::reset() {
    bindings.clear(); target = Target(); target_seen = false; depth = 0;
}
AbiFactFile parse_fact_text(const std::string& text) {
    AbiFactFile file; std::istringstream in(text); std::string line;
    FactReader reader(file.graph); bool started = false;
    while (std::getline(in, line)) {
        Words w = split_words(line);
        if (w.empty()) continue;
        if (w[0] == "case") {
            if (started) file.cases.push_back(reader.finish());
            reader.reset();
            started = false;
        } else { reader.record(w); started = true; }
    }
    if (started) file.cases.push_back(reader.finish());
    if (file.cases.empty()) throw std::runtime_error("empty ABI fact file");
    return file;
}
void parse_fact_stream(std::istream& input, std::ostream& output, RunStats* stats) {
    // Release all facts, binders and substitutions after each case. Neither
    // the whole batch input nor a second record graph is retained.
    Graph graph; FactReader reader(graph); bool started = false, emitted = false;
    using Clock = std::chrono::steady_clock;
    auto parse_start = stats ? Clock::now() : Clock::time_point();
    auto flush = [&]() {
        auto encode_start = stats ? Clock::now() : Clock::time_point();
        std::string name = mangle(graph, reader.finish());
        output << name << '\n'; emitted = true;
        if (stats) {
            auto end = Clock::now();
            stats->parse_nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(encode_start - parse_start).count();
            stats->encode_nanoseconds += std::chrono::duration_cast<std::chrono::nanoseconds>(end - encode_start).count();
            ++stats->cases; stats->output_bytes += name.size() + 1;
            stats->graph_nodes += graph.size() - 1;
            stats->peak_graph_bytes = std::max<std::uint64_t>(stats->peak_graph_bytes, graph.storage_bytes());
            stats->intern_requests += graph.stats.requests; stats->intern_hits += graph.stats.hits;
            stats->intern_probes += graph.stats.probes;
            stats->substitution_lookups += graph.stats.substitution_lookups;
            stats->substitution_hits += graph.stats.substitution_hits;
            stats->substitutions += graph.stats.substitutions; stats->emitted_nodes += graph.stats.emitted_nodes;
            parse_start = end;
        }
    };
    std::string line;
    while (std::getline(input, line)) {
        if (stats) stats->source_bytes += line.size() + !input.eof();
        Words w = split_words(line);
        if (w.empty()) continue;
        if (w[0] == "case") {
            if (started) flush();
            reader.reset(); graph = Graph(); started = false;
        } else {
            reader.record(w); started = true; if (stats) ++stats->records;
        }
    }
    if (started) flush();
    if (!input.eof()) throw std::runtime_error("unable to read ABI facts");
    if (!emitted) throw std::runtime_error("empty ABI fact file");
    if (!output) throw std::runtime_error("unable to write ABI names");
}

std::string mangle_fact_file(AbiFactFile& file) {
    std::string output;
    for (const Target& target : file.cases) output += mangle(file.graph, target) + '\n';
    return output;
}
void mangle_fact_files_to_stream(const std::vector<std::string>& paths, std::ostream& output, RunStats* stats) {
    for (const std::string& path : paths) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("unable to open ABI facts: " + path);
        parse_fact_stream(in, output, stats);
    }
}
std::string mangle_fact_files(const std::vector<std::string>& paths) {
    std::ostringstream output;
    mangle_fact_files_to_stream(paths, output);
    return output.str();
}
void print_stats(std::ostream& out, const RunStats& s) {
    out << "abi cases=" << s.cases << " records=" << s.records
        << " source_bytes=" << s.source_bytes << " output_bytes=" << s.output_bytes
        << " graph_nodes=" << s.graph_nodes << " peak_graph_bytes=" << s.peak_graph_bytes
        << " intern_requests=" << s.intern_requests << " intern_hits=" << s.intern_hits
        << " intern_probes=" << s.intern_probes << " substitutions=" << s.substitutions
        << " substitution_lookups=" << s.substitution_lookups << " substitution_hits=" << s.substitution_hits
        << " emitted_nodes=" << s.emitted_nodes << " parse_ns=" << s.parse_nanoseconds
        << " encode_ns=" << s.encode_nanoseconds << '\n';
}
} // namespace abi_mangle
