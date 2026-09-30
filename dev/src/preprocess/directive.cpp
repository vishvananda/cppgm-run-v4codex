// Adapted from CPPGM PA4 directive and file-identity rules; see NOTICE.
#include "preprocess/preprocessor.h"
#include <cstdlib>
#include <algorithm>
#include <tuple>
#include <stdexcept>
#include <sys/stat.h>

namespace cppgm {

void Preprocessor::include_paths(const std::vector<std::string>& paths)
{
    // Duplicate physical directories must not make include_next re-enter the
    // same search location. Sort temporary identities, retaining original order.
    std::vector<std::tuple<std::uint64_t,std::uint64_t,std::size_t>> directories;
    for (std::size_t i = 0; i < paths.size(); ++i) {
        struct stat info;
        if (!stat(paths[i].c_str(),&info)) directories.emplace_back(info.st_dev,info.st_ino,i);
    }
    std::sort(directories.begin(),directories.end());
    std::vector<bool> duplicate(paths.size(),false);
    for (std::size_t i = 1; i < directories.size(); ++i)
        if (std::get<0>(directories[i]) == std::get<0>(directories[i-1]) &&
            std::get<1>(directories[i]) == std::get<1>(directories[i-1])) duplicate[std::get<2>(directories[i])] = true;
    include_paths_.clear();
    for (std::size_t i = 0; i < paths.size(); ++i) if (!duplicate[i]) include_paths_.push_back(paths[i]);
}

bool Preprocessor::once(const std::string& path, bool insert)
{
    struct stat info;
    if (stat(path.c_str(), &info)) return false;
    FileIdentity key;
    key.device = info.st_dev; key.inode = info.st_ino; key.used = true;
    if (once_.empty()) once_.resize(16);
    auto hash = [](const FileIdentity& k) {
        std::uint64_t x = k.inode ^ (k.device * 0x9e3779b97f4a7c15ULL);
        x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27; x *= 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    };
    if (insert && (once_count_ + 1) * 2 >= once_.size()) {
        std::vector<FileIdentity> old;
        old.swap(once_);
        once_.resize(old.size() * 2);
        for (const FileIdentity& k : old) {
            if (!k.used) continue;
            std::size_t slot = hash(k) & (once_.size() - 1);
            while (once_[slot].used) slot = (slot + 1) & (once_.size() - 1);
            once_[slot] = k;
        }
    }
    std::size_t slot = hash(key) & (once_.size() - 1);
    while (once_[slot].used) {
        if (once_[slot].device == key.device && once_[slot].inode == key.inode) return true;
        slot = (slot + 1) & (once_.size() - 1);
    }
    if (insert) { once_[slot] = key; ++once_count_; }
    return false;
}

void Preprocessor::pragma(const std::string& text, IdentifierId filename)
{
    SourceBuffer source(text);
    PPTokenCursor cursor(source, identifiers_, 0, false, true);
    std::vector<ExpansionToken> tokens;
    SpellingArena spellings;
    for (;;) {
        PPToken token = cursor.next();
        if (token.kind == PPTokenKind::eof) break;
        if (token.kind == PPTokenKind::whitespace || token.kind == PPTokenKind::newline) continue;
        ExpansionToken value; value.token = token;
        value.token.spelling = spellings.save(token.spelling);
        tokens.push_back(value);
    }
    pragma(tokens, filename);
}
void Preprocessor::pragma(const std::vector<ExpansionToken>& tokens, IdentifierId filename)
{
    if (tokens.empty()) return;
    if (tokens[0].is("once")) { once(spelling(filename), true); return; }
    if (!tokens[0].is("pack")) return;
    if (tokens.size() < 3 || !tokens[1].is("(") || !tokens.back().is(")")) throw std::runtime_error("invalid pack pragma");
    std::size_t end = tokens.size()-1, i = 2;
    if (i == end) { packing_ = 0; return; }
    bool push = tokens[i].is("push"), pop = tokens[i].is("pop");
    if (push || pop) ++i;
    IdentifierId name = 0;
    if ((push || pop) && i < end) {
        if (!tokens[i++].is(",")) throw std::runtime_error("invalid pack stack operand");
        if (i < end && tokens[i].token.kind == PPTokenKind::identifier) {
            name = tokens[i++].token.identifier;
            if (i < end && !tokens[i++].is(",")) throw std::runtime_error("invalid pack alignment separator");
        }
    }
    bool has_value = i < end;
    unsigned value = 0;
    if (has_value) {
        auto spelling = tokens[i++].token.spelling;
        if (spelling.equals("1")) value = 1;
        else if (spelling.equals("2")) value = 2;
        else if (spelling.equals("4")) value = 4;
        else if (spelling.equals("8")) value = 8;
        else if (spelling.equals("16")) value = 16;
        else if (!spelling.equals("0")) throw std::runtime_error("unsupported pack alignment");
    }
    if (i != end) throw std::runtime_error("excess pack pragma operands");
    if (push) pack_stack_.push_back(PackFrame{name, packing_});
    if (pop) {
        std::size_t selected = pack_stack_.size();
        if (name) while (selected && pack_stack_[selected-1].name != name) --selected;
        if (!selected) throw std::runtime_error("pack stack entry not found");
        packing_ = pack_stack_[selected-1].value; pack_stack_.resize(selected-1);
    }
    if (has_value) packing_ = value;
}

ExpansionToken Preprocessor::builtin(const ExpansionToken& head, unsigned kind, MacroExpander& expansion)
{
    if (telemetry_) ++stats_.invocations;
    if (kind == 1) return generated(quote_pp_string(spelling(head.filename)), head);
    if (kind == 2) return generated(std::to_string(head.token.line), head);
    if (kind == 3) return generated(std::to_string(counter_++), head);
    if (!expansion.take().is("(")) throw std::runtime_error("preprocessor probe requires parentheses");
    if (kind == 6 || kind == 7) {
        std::vector<ExpansionToken> operand;
        unsigned depth = 1;
        while (depth) {
            auto token = expansion.take();
            if (token.token.kind == PPTokenKind::eof) throw std::runtime_error("unterminated header probe");
            if (token.is("(")) ++depth;
            if (token.is(")")) --depth;
            if (depth) operand.push_back(token);
        }
        MacroExpander argument(*this); argument.push(operand);
        auto header = argument.next();
        std::string name; bool quoted = true;
        if (header.token.kind == PPTokenKind::header) {
            auto text = header.token.spelling;
            name.assign(text.data+1,text.size-2); quoted = text.data[0] == '"';
        } else if (header.is("<")) {
            quoted = false;
            for (;;) {
                auto token = argument.next();
                if (token.is(">")) break;
                if (token.token.kind == PPTokenKind::eof) throw std::runtime_error("unterminated header name");
                name.append(token.token.spelling.data,token.token.spelling.size);
            }
        } else name = decode_pp_string(header);
        if (name.empty() || argument.next().token.kind != PPTokenKind::eof)
            throw std::runtime_error("invalid header probe");
        int index = -1;
        return generated(find_header(name,quoted,kind == 7,index).empty() ? "0" : "1",head);
    }
    ExpansionToken attribute = expansion.take();
    bool recognized = attribute.is("no_unique_address") || attribute.is("__no_unique_address__");
    unsigned value = recognized ? 201803 :
        (attribute.is("noreturn") || attribute.is("carries_dependency") ? 200809 : 0);
    if (kind == 5) value = attribute.is("cppgm_stable_prefix") || attribute.is("__cppgm_stable_prefix__") ||
        attribute.is("packed") || attribute.is("__packed__") || attribute.is("noinline") || attribute.is("__noinline__") ||
        attribute.is("always_inline") || attribute.is("__always_inline__");
    ExpansionToken close = expansion.take();
    if (close.is("::")) {
        attribute = expansion.take();
        if (attribute.token.kind != PPTokenKind::identifier) throw std::runtime_error("invalid attribute name");
        value = 0;
        close = expansion.take();
    }
    if (!close.is(")")) throw std::runtime_error("invalid attribute probe");
    return generated(std::to_string(value), head);
}

bool Preprocessor::condition(const std::vector<ExpansionToken>& tokens)
{
    MacroExpander expansion(*this, false, true);
    expansion.push(tokens);
    PPExpressionEvaluator evaluator(identifiers_, query, this);
    for (;;) {
        ExpansionToken token = expansion.next();
        if (token.token.kind == PPTokenKind::eof) break;
        evaluator.push(token.token);
    }
    PPExpressionResult result = evaluator.finish();
    if (result.empty || !result.valid) throw std::runtime_error("invalid controlling expression in " +
        spelling(files_.back()->filename) + ":" + std::to_string(next_line_));
    return result.value.bits != 0;
}

std::string Preprocessor::find_header(const std::string& name, bool quoted, bool resume, int& index) const
{
    const auto& file = *files_.back();
    struct stat info;
    auto exists = [&](const std::string& path) { return !stat(path.c_str(),&info) && S_ISREG(info.st_mode); };
    if (!name.empty() && name[0] == '/' && exists(name)) return name;
    if (quoted && !resume) {
        std::string current = spelling(file.physical_filename);
        auto slash = current.rfind('/');
        auto relative = (slash == std::string::npos ? "" : current.substr(0,slash+1)) + name;
        if (exists(relative)) { index = file.include_index; return relative; }
    }
    for (std::size_t i = resume ? file.include_index+1 : 0; i < include_paths_.size(); ++i) {
        auto path = include_paths_[i] + "/" + name;
        if (exists(path)) { index = i; return path; }
    }
    // Preserve PA4's explicit-tool search convention.
    if (!resume && exists(name)) return name;
    return {};
}

void Preprocessor::directive()
{
    if (telemetry_) ++stats_.directives;
    if (directive_.empty()) return;
    FileFrame& f = *files_.back();
    ExpansionToken command = directive_.front();
    std::vector<ExpansionToken> rest(directive_.begin() + 1, directive_.end());
    bool active = f.active();
    if (command.is("if") || command.is("ifdef") || command.is("ifndef")) {
        bool selected = false;
        if (active) {
            if (command.is("if")) selected = condition(rest);
            else {
                if (rest.size() != 1 || rest[0].token.kind != PPTokenKind::identifier)
                    throw std::runtime_error("conditional requires an identifier");
                selected = defined(rest[0].token.identifier);
                if (command.is("ifndef")) selected = !selected;
            }
        }
        Conditional c = {active, active && selected, selected, false};
        f.conditions.push_back(c);
        return;
    }
    if (command.is("elif") || command.is("else") || command.is("endif")) {
        if (f.conditions.empty()) throw std::runtime_error("conditional without matching if");
        Conditional& c = f.conditions.back();
        if (command.is("endif")) {
            if (c.parent && !rest.empty()) throw std::runtime_error("tokens after endif");
            f.conditions.pop_back();
        } else {
            if (c.saw_else) throw std::runtime_error("conditional after else");
            if (command.is("else")) {
                if (c.parent && !rest.empty()) throw std::runtime_error("tokens after else");
                c.saw_else = true;
                c.active = c.parent && !c.taken;
            } else c.active = c.parent && !c.taken && condition(rest);
            c.taken = c.taken || c.active;
        }
        return;
    }
    if (!active) return;
    if (command.is("define")) { define(directive_); return; }
    if (command.is("undef")) {
        if (rest.size() != 1 || rest[0].token.kind != PPTokenKind::identifier || rest[0].is("__VA_ARGS__"))
            throw std::runtime_error("undef requires one macro name");
        IdentifierId id = rest[0].token.identifier;
        if (id < macros_.size()) macros_[id] = MacroDefinition();
        return;
    }
    if (command.is("error")) throw std::runtime_error("active error directive");
    if (command.is("pragma")) {
        pragma(rest, f.filename);
        return;
    }
    if (!command.is("include") && !command.is("include_next") && !command.is("line")) throw std::runtime_error("unknown directive");
    MacroExpander expansion(*this);
    expansion.push(rest);
    rest.clear();
    for (;;) {
        ExpansionToken token = expansion.next();
        if (token.token.kind == PPTokenKind::eof) break;
        rest.push_back(token);
    }
    if (command.is("include") || command.is("include_next")) {
        if (rest.size() != 1) throw std::runtime_error("expected one header name");
        std::string next;
        if (rest[0].token.kind == PPTokenKind::header) {
            TextView text = rest[0].token.spelling;
            next.assign(text.data + 1, text.size - 2);
        } else next = decode_pp_string(rest[0]);
        const bool quoted = rest[0].token.kind != PPTokenKind::header || rest[0].token.spelling.data[0] == '"';
        bool resume = command.is("include_next"); int include_index = -1;
        auto found = find_header(next,quoted,resume,include_index);
        if (found.empty()) throw std::runtime_error("cannot find header " + next);
        next = found;
        if (!once(next, false)) { include(next); files_.back()->include_index = include_index; }
    } else {
        if (rest.empty() || rest.size() > 2 || rest[0].token.kind != PPTokenKind::number)
            throw std::runtime_error("invalid line directive");
        PPExpressionEvaluator evaluator(identifiers_, query, this);
        evaluator.push(rest[0].token);
        PPExpressionResult line = evaluator.finish();
        if (!line.valid || !line.value.bits || line.value.bits > 2147483647)
            throw std::runtime_error("invalid line number");
        f.line_delta = static_cast<std::int64_t>(line.value.bits) - next_line_;
        if (rest.size() == 2) f.filename = name(decode_pp_string(rest[1]));
    }
}

} // namespace cppgm
