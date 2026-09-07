#include "abi/itanium/fact_reader.h"
#include <cctype>
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
AbiFactFile parse_fact_text(const std::string& text) {
    AbiFactFile file; std::istringstream in(text); std::string line;
    FactReader reader(file.graph); bool started = false;
    while (std::getline(in, line)) {
        Words w = split_words(line);
        if (w.empty()) continue;
        if (w[0] == "case") {
            if (started) file.cases.push_back(reader.finish());
            reader.~FactReader(); new (&reader) FactReader(file.graph);
            started = false;
        } else { reader.record(w); started = true; }
    }
    if (started) file.cases.push_back(reader.finish());
    if (file.cases.empty()) throw std::runtime_error("empty ABI fact file");
    return file;
}
void parse_fact_stream(std::istream& input, std::ostream& output) {
    // Release all facts, binders and substitutions after each case. Neither
    // the whole batch input nor a second record graph is retained.
    Graph graph; FactReader reader(graph); bool started = false;
    std::string line;
    while (std::getline(input, line)) {
        Words w = split_words(line);
        if (w.empty()) continue;
        if (w[0] == "case") {
            if (started) output << mangle(graph, reader.finish()) << '\n';
            reader.~FactReader(); graph = Graph(); new (&reader) FactReader(graph);
            started = false;
        } else { reader.record(w); started = true; }
    }
    if (started) output << mangle(graph, reader.finish()) << '\n';
    else if (!input.eof()) throw std::runtime_error("unable to read ABI facts");
    if (!output) throw std::runtime_error("unable to write ABI names");
}
std::string mangle_fact_file(AbiFactFile& file) {
    std::string output;
    for (const Target& target : file.cases) output += mangle(file.graph, target) + '\n';
    return output;
}
std::string mangle_fact_files(const std::vector<std::string>& paths) {
    std::ostringstream output;
    for (const std::string& path : paths) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("unable to open ABI facts: " + path);
        parse_fact_stream(in, output);
    }
    return output.str();
}
} // namespace abi_mangle
