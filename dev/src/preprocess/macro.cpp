// Adapted from CPPGM PA4 macro replacement rules; see NOTICE.
#include "preprocess/preprocessor.h"
#include <algorithm>
#include <stdexcept>

namespace cppgm {
namespace {
bool hash(const ExpansionToken& t) { return t.is("#") || t.is("%:"); }
bool paste(const ExpansionToken& t) { return t.is("##") || t.is("%:%:"); }
ExpansionToken end_token()
{
    ExpansionToken t;
    t.token.kind = PPTokenKind::eof;
    return t;
}
}

void Preprocessor::define(const std::vector<ExpansionToken>& line)
{
    if (line.size() < 2 || line[1].token.kind != PPTokenKind::identifier || line[1].is("__VA_ARGS__"))
        throw std::runtime_error("define requires a macro name");
    IdentifierId id = line[1].token.identifier;
    MacroDefinition m;
    m.defined = true;
    parameter_index_.resize(identifiers_.size() + 2, -1);
    std::size_t i = 2;
    if (i < line.size() && line[i].is("(") && !line[i].space) {
        m.function = true;
        ++i;
        if (i < line.size() && !line[i].is(")")) {
            for (;;) {
                if (i == line.size()) throw std::runtime_error("unterminated macro parameters");
                if (line[i].is("...")) {
                    m.variadic = true;
                    m.parameters.push_back(name("__VA_ARGS__"));
                    ++i;
                    break;
                }
                if (line[i].token.kind != PPTokenKind::identifier || line[i].is("__VA_ARGS__"))
                    throw std::runtime_error("invalid macro parameter");
                IdentifierId p = line[i++].token.identifier;
                if (parameter_index_[p] >= 0)
                    throw std::runtime_error("duplicate macro parameter");
                parameter_index_[p] = m.parameters.size();
                m.parameters.push_back(p);
                if (i == line.size() || !line[i].is(",")) break;
                ++i;
            }
        }
        if (i == line.size() || !line[i].is(")")) throw std::runtime_error("expected parameter close");
        ++i;
    } else if (i < line.size() && !line[i].space) {
        throw std::runtime_error("object replacement requires whitespace");
    }
    // Reuse a dense scratch index, touching only this definition's parameters.
    // Neither definition nor invocation scans unrelated TU identifiers.
    for (std::size_t p = 0; p < m.parameters.size(); ++p) parameter_index_[m.parameters[p]] = p;
    for (; i < line.size(); ++i) {
        ExpansionToken t = line[i];
        if (t.is("__VA_ARGS__") && !m.variadic) throw std::runtime_error("invalid __VA_ARGS__");
        if (t.token.identifier) t.parameter = parameter_index_[t.token.identifier];
        if (m.replacement.empty()) t.space = false;
        m.replacement.push_back(t);
    }
    for (IdentifierId p : m.parameters) parameter_index_[p] = -1;
    for (std::size_t p = 0; p < m.replacement.size(); ++p) {
        if (paste(m.replacement[p]) && (!p || p + 1 == m.replacement.size()))
            throw std::runtime_error("paste at replacement boundary");
        if (m.function && hash(m.replacement[p]) &&
            (p + 1 == m.replacement.size() || m.replacement[p + 1].parameter < 0))
            throw std::runtime_error("stringize requires a parameter");
    }
    if (id >= macros_.size()) macros_.resize(id + 1);
    const MacroDefinition& old = macros_[id];
    if (old.defined) {
        bool same = !old.builtin && old.function == m.function && old.variadic == m.variadic &&
            old.parameters == m.parameters && old.replacement.size() == m.replacement.size();
        for (std::size_t p = 0; same && p < m.replacement.size(); ++p) {
            const ExpansionToken& a = old.replacement[p];
            const ExpansionToken& b = m.replacement[p];
            same = a.token.kind == b.token.kind && a.space == b.space &&
                a.token.spelling.size == b.token.spelling.size &&
                std::equal(a.token.spelling.data, a.token.spelling.data + a.token.spelling.size,
                           b.token.spelling.data);
        }
        if (!same) throw std::runtime_error("incompatible macro redefinition");
    } else macros_[id] = std::move(m);
}

MacroExpander::MacroExpander(Preprocessor& owner, bool source, bool expression)
    : owner_(owner), source_(source), expression_(expression) {}

void MacroExpander::push(const std::vector<ExpansionToken>& tokens)
{
    pending_.insert(pending_.end(), tokens.rbegin(), tokens.rend());
    if (owner_.telemetry_)
        owner_.stats_.max_pending = std::max(owner_.stats_.max_pending, pending_.size());
}

ExpansionToken MacroExpander::take()
{
    if (pending_.empty()) return source_ ? owner_.raw() : end_token();
    ExpansionToken t = pending_.back();
    pending_.pop_back();
    return t;
}

static std::string stringize(const std::vector<ExpansionToken>& tokens)
{
    std::string out = "\"";
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        const ExpansionToken& t = tokens[i];
        if (i && t.space) out += ' ';
        bool literal = t.token.kind == PPTokenKind::string || t.token.kind == PPTokenKind::user_string ||
            t.token.kind == PPTokenKind::character || t.token.kind == PPTokenKind::user_character;
        for (std::size_t j = 0; j < t.token.spelling.size; ++j) {
            char c = t.token.spelling.data[j];
            if (literal && (c == '\\' || c == '"')) out += '\\';
            out += c;
        }
    }
    out += '"';
    return out;
}

std::vector<ExpansionToken> MacroExpander::substitute(const ExpansionToken& head,
    const MacroDefinition& m, std::vector<std::vector<ExpansionToken> >& args,
    std::uint32_t replacement_context)
{
    std::vector<std::vector<ExpansionToken> > expanded(args.size());
    std::vector<bool> ready(args.size(), false);
    std::vector<ExpansionToken> result;
    bool pasting = false;
    for (std::size_t i = 0; i < m.replacement.size(); ++i) {
        ExpansionToken t = m.replacement[i];
        if (paste(t)) { pasting = true; continue; }
        std::vector<ExpansionToken> piece;
        if (m.function && hash(t)) {
            t = owner_.generated(stringize(args[m.replacement[++i].parameter]), head);
            piece.push_back(t);
        } else if (t.parameter >= 0) {
            std::size_t p = t.parameter;
            bool raw = pasting || (i + 1 < m.replacement.size() && paste(m.replacement[i + 1]));
            if (!raw && !ready[p]) {
                if (owner_.telemetry_) ++owner_.stats_.argument_prescans;
                MacroExpander prescan(owner_, false, expression_);
                prescan.push(args[p]);
                for (;;) {
                    ExpansionToken e = prescan.next();
                    if (e.token.kind == PPTokenKind::eof) break;
                    expanded[p].push_back(e);
                }
                ready[p] = true;
            }
            piece = raw ? args[p] : expanded[p];
            if (piece.empty() && raw) {
                ExpansionToken marker;
                marker.placemarker = true;
                piece.push_back(marker);
            }
        } else piece.push_back(t);
        for (ExpansionToken& e : piece) {
            // Argument prescan ancestry ends at substitution; permanent paint
            // survives. The new nesting relationship belongs to this head.
            e.context = t.parameter < 0 ? replacement_context : head.context;
            e.substituted = t.parameter >= 0;
            e.parameter = -1;
            if (t.parameter < 0) {
                e.token.file_id = head.token.file_id;
                e.token.begin = head.token.begin; e.token.end = head.token.end;
                e.token.line = head.token.line; e.token.column = head.token.column;
                e.filename = head.filename;
            }
        }
        if (!piece.empty()) piece.front().space = t.space;
        if (pasting) {
            if (result.empty() || piece.empty()) throw std::runtime_error("missing paste operand");
            ExpansionToken left = result.back();
            result.pop_back();
            ExpansionToken right = piece.front();
            // Supported GNU comma-elision form is specific to a variadic
            // parameter. Other token pastes must form one preprocessing token.
            if (left.is(",") && t.parameter >= 0 && m.variadic &&
                static_cast<std::size_t>(t.parameter) + 1 == m.parameters.size()) {
                if (!right.placemarker) result.push_back(left);
            } else if (left.placemarker) {
                piece.front().space = left.space;
            } else if (right.placemarker) piece.front() = left;
            else {
                std::string joined(left.token.spelling.data, left.token.spelling.size);
                joined.append(right.token.spelling.data, right.token.spelling.size);
                if (owner_.telemetry_) owner_.stats_.paste_bytes += joined.size();
                piece.front() = owner_.generated(joined, head);
                piece.front().context = replacement_context;
                piece.front().space = left.space;
            }
            pasting = false;
        }
        result.insert(result.end(), piece.begin(), piece.end());
    }
    result.erase(std::remove_if(result.begin(), result.end(),
        [](const ExpansionToken& t) { return t.placemarker; }), result.end());
    if (!result.empty()) result.front().space = head.space;
    if (owner_.telemetry_) owner_.stats_.replacement_tokens += result.size();
    return result;
}

ExpansionToken MacroExpander::next()
{
    for (;;) {
        ExpansionToken head = take();
        if (head.token.kind != PPTokenKind::identifier) return head;
        if (expression_ && head.is("defined")) {
            ExpansionToken operand = take();
            bool parens = operand.is("(");
            if (parens) operand = take();
            bool word = operand.token.kind == PPTokenKind::identifier ||
                (operand.token.spelling.size && identifier_start(static_cast<unsigned char>(operand.token.spelling.data[0])));
            if (!word || (parens && !take().is(")"))) throw std::runtime_error("invalid defined operand");
            IdentifierId id = owner_.identifiers_.intern(operand.token.spelling);
            return owner_.generated(owner_.defined(id) ? "1" : "0", head);
        }
        if (head.is("__VA_ARGS__")) throw std::runtime_error("__VA_ARGS__ outside replacement");
        IdentifierId id = head.token.identifier;
        if (!owner_.defined(id) || head.unavailable) return head;
        const MacroDefinition& m = owner_.macros_[id];
        if (!m.function && owner_.painted(head.context, id)) { head.unavailable = true; return head; }
        if (m.builtin) return owner_.builtin(head, m.builtin, *this);
        std::vector<std::vector<ExpansionToken> > args;
        std::uint32_t replacement_context = head.context;
        if (m.function) {
            ExpansionToken open = take();
            if (!open.is("(")) { pending_.push_back(open); return head; }
            if (owner_.painted(head.context, id)) {
                head.unavailable = true;
                pending_.push_back(open);
                return head;
            }
            args.resize(1);
            unsigned depth = 0;
            for (;;) {
                ExpansionToken t = take();
                if (t.token.kind == PPTokenKind::eof) throw std::runtime_error("unterminated macro invocation");
                if (t.is(")") && !depth) {
                    replacement_context = owner_.intersect(head.context, t.context);
                    break;
                }
                if (t.is("(")) ++depth;
                else if (t.is(")")) --depth;
                if (t.is(",") && !depth && (!m.variadic || args.size() < m.parameters.size()))
                    args.resize(args.size() + 1);
                else args.back().push_back(t);
            }
            if (m.parameters.empty() && args.size() == 1 && args[0].empty()) args.clear();
            if (m.variadic && args.size() + 1 == m.parameters.size()) args.resize(args.size() + 1);
            if (args.size() != m.parameters.size()) throw std::runtime_error("wrong macro argument count");
        }
        if (owner_.telemetry_) ++owner_.stats_.invocations;
        std::uint32_t inherited = head.context;
        replacement_context = owner_.paint(replacement_context, id);
        head.context = head.substituted ? owner_.paint(inherited, id) : replacement_context;
        push(substitute(head, m, args, replacement_context));
    }
}

} // namespace cppgm
