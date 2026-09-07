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
    : owner_(owner), source_(source), expression_(expression) { tasks_.emplace_back(); }

bool MacroExpander::empty() const
{
    return tasks_.size() == 1 && tasks_.front().pending.empty() && !tasks_.front().invocation.active;
}

void MacroExpander::push(const std::vector<ExpansionToken>& tokens)
{
    std::vector<ExpansionToken>& pending = tasks_.back().pending;
    if (owner_.telemetry_ && pending.size() + tokens.size() > pending.capacity()) ++owner_.stats_.scratch_growths;
    pending.insert(pending.end(), tokens.rbegin(), tokens.rend());
    if (owner_.telemetry_) owner_.stats_.max_pending = std::max(owner_.stats_.max_pending, pending.size());
}

ExpansionToken MacroExpander::take()
{
    Task& task = tasks_.back();
    task.last_from_slice = false;
    if (!task.pending.empty()) {
        ExpansionToken t = task.pending.back();
        task.pending.pop_back();
        return t;
    }
    if (task.input.storage && task.position < task.input.end) {
        task.last_from_slice = true;
        task.last_index = task.position;
        return task.input.storage->tokens[task.position++];
    }
    return source_ && tasks_.size() == 1 ? owner_.raw() : end_token();
}

void MacroExpander::ArgumentStorage::index()
{
    boundary.resize(tokens.size());
    std::vector<std::size_t> stack;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].is("(")) stack.push_back(i);
        else if (tokens[i].is(",") || tokens[i].is(")")) {
            if (stack.empty()) throw std::runtime_error("unbalanced captured invocation");
            boundary[stack.back()] = i;
            if (tokens[i].is(")")) stack.pop_back();
            else stack.back() = i;
        }
    }
}

void MacroExpander::collect(const ExpansionToken& open, const MacroDefinition& macro)
{
    Task& task = tasks_.back();
    Invocation& invocation = task.invocation;
    const ArgumentStorage* storage;
    std::size_t start;
    if (task.last_from_slice) {
        if (owner_.telemetry_) ++owner_.stats_.borrowed_arguments;
        storage = task.input.storage;
        start = task.last_index;
    } else {
        storage = &invocation.captured;
        start = 0;
        invocation.captured.tokens.clear();
        invocation.captured.tokens.push_back(open);
        unsigned depth = 1;
        while (depth) {
            ExpansionToken t = take();
            if (t.token.kind == PPTokenKind::eof) throw std::runtime_error("unterminated macro invocation");
            if (t.is("(")) ++depth;
            else if (t.is(")")) --depth;
            invocation.captured.tokens.push_back(t);
        }
        invocation.captured.index();
        if (owner_.telemetry_) owner_.stats_.captured_tokens += invocation.captured.tokens.size();
    }
    std::size_t position = start;
    invocation.arguments.clear();
    do {
        std::size_t end = storage->boundary[position];
        Argument argument;
        argument.raw.storage = storage;
        argument.raw.begin = position + 1;
        argument.raw.end = end;
        invocation.arguments.push_back(std::move(argument));
        position = end;
    } while (storage->tokens[position].is(","));
    if (storage != &invocation.captured) task.position = position + 1;
    invocation.replacement_context = owner_.intersect(invocation.head.context, storage->tokens[position].context);
    std::vector<Argument>& args = invocation.arguments;
    if (macro.parameters.empty() && args.size() == 1 && args[0].raw.begin == args[0].raw.end) args.clear();
    if (macro.variadic && args.size() >= macro.parameters.size()) {
        args[macro.parameters.size() - 1].raw.end = position;
        args.resize(macro.parameters.size());
    }
    if (macro.variadic && args.size() + 1 == macro.parameters.size()) {
        Argument empty;
        empty.raw.storage = storage;
        empty.raw.begin = empty.raw.end = position;
        args.push_back(std::move(empty));
    }
    if (args.size() != macro.parameters.size()) throw std::runtime_error("wrong macro argument count");
}

static void stringize_token(std::string& out, const ExpansionToken& t, bool first)
{
    if (!first && t.space) out += ' ';
    bool literal = t.token.kind == PPTokenKind::string || t.token.kind == PPTokenKind::user_string ||
        t.token.kind == PPTokenKind::character || t.token.kind == PPTokenKind::user_character;
    for (std::size_t j = 0; j < t.token.spelling.size; ++j) {
        char c = t.token.spelling.data[j];
        if (literal && (c == '\\' || c == '"')) out += '\\';
        out += c;
    }
}

void MacroExpander::substitute(Invocation& invocation, const MacroDefinition& m)
{
    const ExpansionToken& head = invocation.head;
    std::vector<ExpansionToken>& result = tasks_.back().replacement;
    result.clear();
    bool pasting = false;
    for (std::size_t i = 0; i < m.replacement.size(); ++i) {
        ExpansionToken t = m.replacement[i], single;
        if (paste(t)) { pasting = true; continue; }
        const ExpansionToken* piece = &single;
        std::size_t count = 1;
        if (m.function && hash(t)) {
            const Slice& raw = invocation.arguments[m.replacement[++i].parameter].raw;
            std::string text = "\"";
            for (std::size_t p = raw.begin; p < raw.end; ++p)
                stringize_token(text, raw.storage->tokens[p], p == raw.begin);
            single = owner_.generated(text + '"', head);
        } else if (t.parameter >= 0) {
            const Argument& arg = invocation.arguments[t.parameter];
            bool raw = pasting || (i + 1 < m.replacement.size() && paste(m.replacement[i + 1]));
            if (raw) {
                count = arg.raw.end - arg.raw.begin;
                piece = arg.raw.storage->tokens.data() + arg.raw.begin;
            } else {
                count = arg.expanded.size();
                piece = arg.expanded.data();
            }
            if (!count && raw) { single.placemarker = true; piece = &single; count = 1; }
        } else single = t;
        for (std::size_t p = 0; p < count; ++p) {
            ExpansionToken e = piece[p];
            e.context = t.parameter < 0 ? invocation.replacement_context : head.context;
            e.substituted = t.parameter >= 0;
            e.parameter = -1;
            if (t.parameter < 0) {
                e.token.file_id = head.token.file_id;
                e.token.presumed_file = head.filename;
                e.token.begin = head.token.begin; e.token.end = head.token.end;
                e.token.line = head.token.line; e.token.column = head.token.column;
                e.filename = head.filename;
            }
            if (!p) e.space = t.space;
            if (!p && pasting) {
                if (result.empty()) throw std::runtime_error("missing paste operand");
                ExpansionToken left = result.back();
                result.pop_back();
                bool comma = left.is(",") && t.parameter >= 0 && m.variadic &&
                    static_cast<std::size_t>(t.parameter) + 1 == m.parameters.size();
                if (comma) {
                    if (!e.placemarker) result.push_back(left);
                } else if (left.placemarker) e.space = left.space;
                else if (e.placemarker) e = left;
                else {
                    std::string joined(left.token.spelling.data, left.token.spelling.size);
                    joined.append(e.token.spelling.data, e.token.spelling.size);
                    if (owner_.telemetry_) owner_.stats_.paste_bytes += joined.size();
                    e = owner_.generated(joined, head);
                    e.context = invocation.replacement_context;
                    e.substituted = false;
                    e.space = left.space;
                }
            }
            if (owner_.telemetry_ && result.size() == result.capacity()) ++owner_.stats_.scratch_growths;
            result.push_back(e);
        }
        pasting = false;
    }
    result.erase(std::remove_if(result.begin(), result.end(),
        [](const ExpansionToken& t) { return t.placemarker; }), result.end());
    if (!result.empty()) result.front().space = head.space;
    if (owner_.telemetry_) owner_.stats_.replacement_tokens += result.size();
    push(result);
}

bool MacroExpander::resume()
{
    Invocation& invocation = tasks_.back().invocation;
    if (!invocation.active) return false;
    const MacroDefinition& m = owner_.macros_[invocation.macro];
    while (invocation.prescan < m.replacement.size()) {
        std::size_t i = invocation.prescan++;
        int parameter = m.replacement[i].parameter;
        bool raw = (i && (hash(m.replacement[i - 1]) || paste(m.replacement[i - 1]))) ||
            (i + 1 < m.replacement.size() && paste(m.replacement[i + 1]));
        if (parameter < 0 || raw || invocation.arguments[parameter].ready) continue;
        invocation.waiting = parameter;
        Slice input = invocation.arguments[parameter].raw;
        if (owner_.telemetry_) ++owner_.stats_.argument_prescans;
        tasks_.emplace_back();
        if (owner_.telemetry_)
            owner_.stats_.max_prescan_depth = std::max(owner_.stats_.max_prescan_depth, tasks_.size() - 1);
        tasks_.back().input = input;
        tasks_.back().position = input.begin;
        return true;
    }
    substitute(invocation, m);
    invocation.active = false;
    invocation.arguments.clear();
    invocation.captured.tokens.clear();
    invocation.captured.boundary.clear();
    return true;
}

ExpansionToken MacroExpander::next()
{
    for (;;) {
        if (resume()) continue;
        ExpansionToken head = take();
        if (head.token.kind == PPTokenKind::eof && tasks_.size() > 1) {
            std::vector<ExpansionToken> result = std::move(tasks_.back().output);
            tasks_.pop_back();
            Invocation& parent = tasks_.back().invocation;
            parent.arguments[parent.waiting].expanded = std::move(result);
            parent.arguments[parent.waiting].ready = true;
            continue;
        }
        if (head.token.kind == PPTokenKind::identifier) {
            if (expression_ && head.is("defined")) {
                ExpansionToken operand = take();
                bool parens = operand.is("(");
                if (parens) operand = take();
                bool word = operand.token.kind == PPTokenKind::identifier ||
                    (operand.token.spelling.size && identifier_start(static_cast<unsigned char>(operand.token.spelling.data[0])));
                if (!word || (parens && !take().is(")"))) throw std::runtime_error("invalid defined operand");
                IdentifierId id = owner_.identifiers_.intern(operand.token.spelling);
                head = owner_.generated(owner_.defined(id) ? "1" : "0", head);
            } else if (head.is("__VA_ARGS__")) throw std::runtime_error("__VA_ARGS__ outside replacement");
        }
        IdentifierId id = head.token.identifier;
        if (head.token.kind == PPTokenKind::identifier && owner_.defined(id) && !head.unavailable) {
            const MacroDefinition& m = owner_.macros_[id];
            bool call = true;
            ExpansionToken open;
            if (m.function) {
                open = take();
                call = open.is("(");
            }
            if (call && owner_.painted(head.context, id)) { head.unavailable = true; call = false; }
            if (!call) {
                if (m.function) tasks_.back().pending.push_back(open);
            } else if (m.builtin) head = owner_.builtin(head, m.builtin, *this);
            else {
                Invocation& invocation = tasks_.back().invocation;
                invocation.head = head;
                invocation.replacement_context = head.context;
                if (m.function) collect(open, m);
                if (owner_.telemetry_) ++owner_.stats_.invocations;
                invocation.replacement_context = owner_.paint(invocation.replacement_context, id);
                invocation.head.context = head.substituted ? owner_.paint(head.context, id) : invocation.replacement_context;
                invocation.macro = id;
                invocation.active = true;
                invocation.prescan = 0;
                continue;
            }
        }
        if (tasks_.size() == 1) return head;
        tasks_.back().output.push_back(head);
    }
}

} // namespace cppgm
