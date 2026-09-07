// Adapted from CPPGM PA4 preprocessing and host file-identity scaffold; see NOTICE.
#include "preprocess/preprocessor.h"
#include "posttoken/literal.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <sys/stat.h>

namespace cppgm {

IdentifierId Preprocessor::name(const std::string& text)
{
    return identifiers_.intern(TextView(text.data(), text.size()));
}

std::string Preprocessor::spelling(IdentifierId id) const
{
    TextView text = identifiers_.spelling(id);
    return std::string(text.data, text.size);
}

TextView Preprocessor::SpellingArena::save(TextView text)
{
    if (text.size == 0) return TextView();
    if (current < slabs.size() && slabs[current].capacity() - slabs[current].size() < text.size)
        ++current;
    if (current == slabs.size()) {
        slabs.emplace_back();
        slabs.back().reserve(std::max<std::size_t>(65536, text.size));
        bytes += slabs.back().capacity();
    }
    std::vector<char>& slab = slabs[current];
    if (slab.capacity() - slab.size() < text.size) {
        // A rewound empty slab can grow; no live spelling points into it.
        bytes -= slab.capacity();
        slab.reserve(text.size);
        bytes += slab.capacity();
    }
    std::size_t start = slab.size();
    slab.insert(slab.end(), text.data, text.data + text.size);
    return TextView(slab.data() + start, text.size);
}

void Preprocessor::SpellingArena::rewind()
{
    for (std::size_t i = 0; i < slabs.size() && i <= current; ++i) slabs[i].clear();
    current = 0;
}

std::string quote_pp_string(const std::string& text)
{
    std::string out = "\"";
    for (char c : text) {
        if (c == '\\' || c == '"') out += '\\';
        out += c;
    }
    return out + '"';
}

std::string decode_pp_string(const ExpansionToken& token)
{
    if (token.token.kind != PPTokenKind::string) throw std::runtime_error("expected string literal");
    LiteralReader reader(token.token.spelling, 0);
    std::string out;
    LiteralElement e;
    while (reader.next(e)) {
        if (!e.valid || !append_string_element(out, e, Encoding::ordinary))
            throw std::runtime_error("invalid directive string");
    }
    return out;
}

Preprocessor::Preprocessor(const std::string& path, const std::string& date,
                           const std::string& time, bool telemetry)
    : telemetry_(telemetry), identifiers_(telemetry ? &lex_stats_ : 0), expander_(*this, true)
{
    reset_contexts();
    const char* dynamic[] = {"__FILE__", "__LINE__", "__COUNTER__", "__has_cpp_attribute"};
    for (unsigned i = 0; i < 4; ++i) {
        IdentifierId id = name(dynamic[i]);
        macros_.resize(id + 1);
        macros_[id].defined = true;
        macros_[id].builtin = i + 1;
    }
    const std::string fixed[][2] = {
        {"__CPPGM__", "201303L"}, {"__cplusplus", "201103L"}, {"__STDC_HOSTED__", "1"},
        {"__CPPGM_AUTHOR__", "\"Codex\""}, {"__DATE__", quote_pp_string(date)},
        {"__TIME__", quote_pp_string(time)}
    };
    for (const auto& entry : fixed) {
        IdentifierId id = name(entry[0]);
        ExpansionToken value = generated(entry[1], ExpansionToken());
        value.token.spelling = persistent_.save(value.token.spelling);
        if (id >= macros_.size()) macros_.resize(id + 1);
        macros_[id].defined = true;
        macros_[id].replacement.push_back(value);
    }
    include(path);
}

bool Preprocessor::defined(IdentifierId id) const
{
    return id < macros_.size() && macros_[id].defined;
}

bool Preprocessor::query(IdentifierId id, void* context)
{
    return static_cast<Preprocessor*>(context)->defined(id);
}

void Preprocessor::reset_contexts()
{
    contexts_.clear();
    ContextNode zero = {{0, 0}};
    contexts_.push_back(zero); // absent set
    contexts_.push_back(zero); // present leaf at depth 32
}

bool Preprocessor::painted(std::uint32_t root, IdentifierId id) const
{
    for (unsigned bit = 0; root && bit < 32; ++bit) root = contexts_[root].child[(id >> bit) & 1];
    return root != 0;
}

std::uint32_t Preprocessor::paint(std::uint32_t root, IdentifierId id)
{
    // Fixed-depth persistent radix set: 32 probes/copies irrespective of macro
    // nesting depth. No copied blacklists or quadratic ancestry-chain walks.
    std::uint32_t path[32];
    for (unsigned bit = 0; bit < 32; ++bit) {
        path[bit] = root;
        root = contexts_[root].child[(id >> bit) & 1];
    }
    std::uint32_t child = 1;
    for (int bit = 31; bit >= 0; --bit) {
        ContextNode node = contexts_[path[bit]];
        node.child[(id >> bit) & 1] = child;
        if (contexts_.size() == std::numeric_limits<std::uint32_t>::max())
            throw std::runtime_error("macro context capacity exceeded");
        child = contexts_.size();
        contexts_.push_back(node);
    }
    if (telemetry_) stats_.context_nodes += 32;
    if (telemetry_) stats_.max_context_nodes = std::max(stats_.max_context_nodes, contexts_.size());
    return child;
}

std::uint32_t Preprocessor::intersect(std::uint32_t a, std::uint32_t b)
{
    if (a == b) return a;
    if (!a || !b) return 0;
    ContextNode x = contexts_[a], y = contexts_[b];
    ContextNode node = {{intersect(x.child[0], y.child[0]), intersect(x.child[1], y.child[1])}};
    if (node.child[0] == x.child[0] && node.child[1] == x.child[1]) return a;
    if (node.child[0] == y.child[0] && node.child[1] == y.child[1]) return b;
    if (!node.child[0] && !node.child[1]) return 0;
    contexts_.push_back(node);
    return contexts_.size() - 1;
}

ExpansionToken Preprocessor::generated(const std::string& text, const ExpansionToken& origin)
{
    SourceBuffer source(text);
    PPTokenCursor cursor(source, identifiers_, 0, false, true);
    ExpansionToken result = origin;
    PPToken t = cursor.next();
    if (t.kind == PPTokenKind::whitespace || t.kind == PPTokenKind::newline || t.kind == PPTokenKind::eof)
        throw std::runtime_error("paste does not form a token");
    t.spelling = transient_.save(t.spelling);
    PPToken tail = cursor.next();
    if (tail.kind == PPTokenKind::newline) tail = cursor.next();
    if (tail.kind != PPTokenKind::eof) throw std::runtime_error("paste forms multiple tokens");
    t.file_id = origin.token.file_id;
    t.presumed_file = origin.filename;
    t.begin = origin.token.begin; t.end = origin.token.end;
    t.line = origin.token.line; t.column = origin.token.column;
    result.token = t;
    result.unavailable = false;
    result.parameter = -1;
    return result;
}

ExpansionToken Preprocessor::stabilize(PPToken token, FileFrame& file, bool persistent)
{
    if (token.spelling.size && token.spelling.data != file.source.bytes.data() + token.begin)
        token.spelling = (persistent ? persistent_ : transient_).save(token.spelling);
    ExpansionToken result;
    result.token = token;
    result.token.line += file.line_delta;
    result.token.presumed_file = file.filename;
    result.filename = file.filename;
    result.space = file.space;
    return result;
}

void Preprocessor::include(const std::string& path)
{
    std::ifstream input(path.c_str(), std::ios::binary);
    if (!input) throw std::runtime_error("cannot include " + path);
    std::string bytes((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (input.bad()) throw std::runtime_error("cannot read " + path);
    if (telemetry_) { stats_.source_bytes += bytes.size(); ++stats_.files; }
    sources_.emplace_back(std::move(bytes), sources_.size() + 1);
    IdentifierId filename = name(path);
    files_.emplace_back(new FileFrame(sources_.back(), identifiers_, telemetry_ ? &lex_stats_ : 0, filename));
}

ExpansionToken Preprocessor::raw()
{
    ExpansionToken end;
    end.token.kind = PPTokenKind::eof;
    if (boundary_ || files_.empty()) return end;
    FileFrame& f = *files_.back();
    for (;;) {
        PPToken t = f.cursor.next();
        if (t.kind == PPTokenKind::eof) {
            if (!f.conditions.empty()) throw std::runtime_error("unterminated conditional group");
            boundary_ = end_file_ = true;
            return end;
        }
        if (t.kind == PPTokenKind::newline) { f.line_start = f.space = true; continue; }
        if (t.kind == PPTokenKind::whitespace) { f.space = true; continue; }
        if (f.line_start && (t.spelling.equals("#") || t.spelling.equals("%:"))) {
            directive_.clear();
            f.space = false;
            for (;;) {
                t = f.cursor.next();
                if (t.kind == PPTokenKind::newline || t.kind == PPTokenKind::eof) break;
                if (t.kind == PPTokenKind::whitespace) f.space = true;
                else { directive_.push_back(stabilize(t, f, true)); f.space = false; }
            }
            next_line_ = t.line + 1;
            f.line_start = f.space = true;
            boundary_ = true;
            return end;
        }
        f.line_start = false;
        ExpansionToken result = stabilize(t, f);
        f.space = false;
        if (f.active()) return result;
    }
}

bool Preprocessor::advance()
{
    if (files_.empty()) return false;
    if (end_file_) { files_.pop_back(); end_file_ = false; }
    else directive();
    boundary_ = false;
    return !files_.empty();
}

PPToken Preprocessor::next()
{
    for (;;) {
        if (expander_.empty()) { reset_contexts(); transient_.rewind(); }
        ExpansionToken token = expander_.next();
        if (token.token.kind == PPTokenKind::eof) {
            if (advance()) continue;
            return token.token;
        }
        if (token.is("_Pragma")) {
            ExpansionToken open = expander_.next();
            ExpansionToken string = expander_.next();
            ExpansionToken close = expander_.next();
            if (!open.is("(") || string.token.kind != PPTokenKind::string || !close.is(")"))
                throw std::runtime_error("invalid _Pragma invocation");
            pragma(decode_pp_string(string), token.filename);
            continue;
        }
        if (telemetry_) {
            ++stats_.output_tokens;
            stats_.arena_bytes = persistent_.bytes + transient_.bytes;
        }
        return token.token;
    }
}

} // namespace cppgm
