// Adapted from CPPGM PA4 directive and file-identity rules; see NOTICE.
#include "preprocess/preprocessor.h"
#include <cstdlib>
#include <stdexcept>
#include <sys/stat.h>

namespace cppgm {

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
    PPToken token = cursor.next();
    while (token.kind == PPTokenKind::whitespace) token = cursor.next();
    if (token.spelling.equals("once")) once(spelling(filename), true);
}

ExpansionToken Preprocessor::builtin(const ExpansionToken& head, unsigned kind, MacroExpander& expansion)
{
    if (telemetry_) ++stats_.invocations;
    if (kind == 1) return generated(quote_pp_string(spelling(head.filename)), head);
    if (kind == 2) return generated(std::to_string(head.token.line), head);
    if (kind == 3) return generated(std::to_string(counter_++), head);
    if (!expansion.take().is("(")) throw std::runtime_error("attribute probe requires parentheses");
    ExpansionToken attribute = expansion.take();
    bool recognized = attribute.is("no_unique_address") || attribute.is("__no_unique_address__");
    unsigned value = recognized ? 201803 :
        (attribute.is("noreturn") || attribute.is("carries_dependency") ? 200809 : 0);
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
    if (result.empty || !result.valid) throw std::runtime_error("invalid controlling expression");
    return result.value.bits != 0;
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
        if (!rest.empty() && rest[0].is("once")) once(spelling(f.filename), true);
        return;
    }
    if (!command.is("include") && !command.is("line")) throw std::runtime_error("unknown directive");
    MacroExpander expansion(*this);
    expansion.push(rest);
    rest.clear();
    for (;;) {
        ExpansionToken token = expansion.next();
        if (token.token.kind == PPTokenKind::eof) break;
        rest.push_back(token);
    }
    if (command.is("include")) {
        if (rest.size() != 1) throw std::runtime_error("expected one header name");
        std::string next;
        if (rest[0].token.kind == PPTokenKind::header) {
            TextView text = rest[0].token.spelling;
            next.assign(text.data + 1, text.size - 2);
        } else next = decode_pp_string(rest[0]);
        std::string current = spelling(f.filename);
        std::size_t slash = current.rfind('/');
        if (slash != std::string::npos) {
            std::string relative = current.substr(0, slash + 1) + next;
            struct stat info;
            if (!stat(relative.c_str(), &info)) next = relative;
        }
        if (!once(next, false)) include(next);
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
