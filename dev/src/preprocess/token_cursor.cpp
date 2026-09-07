// Adapted from the CPPGM PA1 grammar/starter; see NOTICE for attribution.
#include "preprocess/token_cursor.h"

#include <cstring>
#include <stdexcept>

namespace cppgm {

const char* token_kind_name(PPTokenKind kind)
{
    static const char* const names[] = {
        "whitespace-sequence", "new-line", "header-name", "identifier",
        "pp-number", "character-literal", "user-defined-character-literal",
        "string-literal", "user-defined-string-literal",
        "preprocessing-op-or-punc", "non-whitespace-character", "eof"
    };
    return names[static_cast<unsigned>(kind)];
}

static bool horizontal_space(int c)
{
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

static bool word_operator(TextView text)
{
    // Bounded language metadata, not an allocating hash container.
    static const char* const words[] = {
        "new", "delete", "and", "and_eq", "bitand", "bitor", "compl",
        "not", "not_eq", "or", "or_eq", "xor", "xor_eq"
    };
    if (text.size < 2 || text.size > 6) return false;
    for (const char* word : words)
        if (text.equals(word)) return true;
    return false;
}

PPTokenCursor::PPTokenCursor(const SourceBuffer& source, IdentifierTable& identifiers,
                             LexStats* stats, bool recover_empty_character)
    : source_(source), identifiers_(identifiers), stats_(stats), characters_(source, stats),
      recover_empty_character_(recover_empty_character) {}

int PPTokenCursor::take(bool capture)
{
    SourceCharacter c = characters_.take();
    if (capture) {
        if (!copied_ && (!c.unchanged || c.begin != token_.end)) {
            translated_.assign(source_.bytes.data() + token_.begin, token_.end - token_.begin);
            copied_ = true;
        }
        if (copied_) append_utf8(translated_, c.value);
    }
    token_.end = c.end;
    return c.value;
}

TextView PPTokenCursor::spelling() const
{
    if (copied_) return TextView(translated_.data(), translated_.size());
    return TextView(source_.bytes.data() + token_.begin, token_.end - token_.begin);
}

void PPTokenCursor::whitespace()
{
    token_.kind = PPTokenKind::whitespace;
    for (;;) {
        if (horizontal_space(peek())) take(false);
        else if (peek() == '/' && peek(1) == '/') {
            take(false); take(false);
            while (peek() != '\n' && peek() != -1) take(false);
        } else if (peek() == '/' && peek(1) == '*') {
            take(false); take(false);
            while (!(peek() == '*' && peek(1) == '/')) {
                if (peek() == -1) throw std::runtime_error("unterminated block comment");
                take(false);
            }
            take(false); take(false);
        } else break;
    }
}

void PPTokenCursor::identifier()
{
    take();
    while (identifier_continue(peek())) take();
    TextView prefix = spelling();
    if (peek() == '"') {
        if (prefix.equals("R") || prefix.equals("u8R") || prefix.equals("uR") ||
            prefix.equals("UR") || prefix.equals("LR")) {
            literal(true, '"');
            return;
        }
        if (prefix.equals("u8") || prefix.equals("u") || prefix.equals("U") || prefix.equals("L")) {
            literal(false, '"');
            return;
        }
    } else if (peek() == '\'' && (prefix.equals("u") || prefix.equals("U") || prefix.equals("L"))) {
        literal(false, '\'');
        return;
    }
    token_.kind = word_operator(prefix) ? PPTokenKind::punctuation : PPTokenKind::identifier;
    if (token_.kind == PPTokenKind::identifier)
        token_.identifier = identifiers_.intern(prefix);
}

void PPTokenCursor::number()
{
    token_.kind = PPTokenKind::number;
    int previous = take();
    for (;;) {
        int c = peek();
        if (identifier_continue(c) || c == '.' ||
            ((previous == 'e' || previous == 'E') && (c == '+' || c == '-')))
            previous = take();
        else break;
    }
}

void PPTokenCursor::escape()
{
    take(); // backslash; spelling remains encoded for PA2's literal conversion
    characters_.ucn_mode(false);
    int c = peek();
    if (c == -1 || c == '\n') throw std::runtime_error("unterminated literal escape");
    take();
    characters_.ucn_mode(true);
    if (c > 0 && c < 128 && std::strchr("'\"?\\abfnrtv", c)) return;
    if (c >= '0' && c <= '7') {
        for (int i = 1; i < 3 && peek() >= '0' && peek() <= '7'; ++i) take();
        return;
    }
    if (c == 'x') {
        if (hex_value(peek()) < 0) throw std::runtime_error("hex escape needs a digit");
        do { take(); } while (hex_value(peek()) >= 0);
        return;
    }
    throw std::runtime_error("invalid literal escape");
}

void PPTokenCursor::raw_literal()
{
    characters_.raw_mode(true);
    int delimiter[16];
    std::size_t length = 0;
    while (peek() != '(') {
        int c = peek();
        if (length == 16 || c < 0x21 || c > 0x7e || c == ')' || c == '\\')
            throw std::runtime_error("invalid raw string delimiter");
        delimiter[length++] = take();
    }
    take();
    for (;;) {
        if (peek() == -1) throw std::runtime_error("unterminated raw string literal");
        if (peek() == ')') {
            std::size_t matched = 0;
            while (matched < length && peek(matched + 1) == delimiter[matched]) ++matched;
            if (matched == length && peek(length + 1) == '"') {
                for (std::size_t i = 0; i < length + 2; ++i) take();
                break;
            }
        }
        take();
    }
    characters_.raw_mode(false);
}

void PPTokenCursor::literal(bool raw, int quote)
{
    take(); // opening quote (encoding prefix, if any, was already consumed)
    if (raw) raw_literal();
    else {
        bool nonempty = false;
        while (peek() != quote) {
            if (peek() == -1 || peek() == '\n')
                throw std::runtime_error("unterminated literal");
            if (peek() == '\\') escape();
            else take();
            nonempty = true;
        }
        if (quote == '\'' && !nonempty && !recover_empty_character_)
            throw std::runtime_error("empty character literal");
        take();
    }
    bool suffix = identifier_start(peek());
    if (suffix) {
        const SourceCharacter& start = characters_.peek();
        token_.suffix_begin = start.begin;
        token_.suffix_line = start.line; token_.suffix_column = start.column;
        std::size_t suffix_offset = spelling().size;
        do { take(); } while (identifier_continue(peek()));
        TextView full = spelling();
        token_.suffix = identifiers_.intern(TextView(full.data + suffix_offset, full.size - suffix_offset));
    }
    if (quote == '\'') token_.kind = suffix ? PPTokenKind::user_character : PPTokenKind::character;
    else token_.kind = suffix ? PPTokenKind::user_string : PPTokenKind::string;
}

void PPTokenCursor::header()
{
    int close = take() == '<' ? '>' : '"';
    bool nonempty = false;
    while (peek() != close) {
        if (peek() == -1 || peek() == '\n') throw std::runtime_error("unterminated header name");
        nonempty = true;
        take();
    }
    if (!nonempty) throw std::runtime_error("empty header name");
    take();
    token_.kind = PPTokenKind::header;
}

void PPTokenCursor::punctuation()
{
    token_.kind = PPTokenKind::punctuation;
    int first = peek();
    // [lex.pptoken] exception: <:: followed by neither ':' nor '>'.
    if (first == '<' && peek(1) == ':' && peek(2) == ':' && peek(3) != ':' && peek(3) != '>') {
        take();
        return;
    }
    // Longest first, grouped by first character to bound irrelevant lookahead.
    static const char* const operators[] = {
        "%:%:", ">>=", "<<=", "->*", "...", "##", "<:", ":>", "<%", "%>", "%:",
        "::", ".*", "+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>",
        "==", "!=", "<=", ">=", "&&", "||", "++", "--", "->"
    };
    for (const char* op : operators) {
        if (first != op[0]) continue;
        std::size_t i = 1;
        while (op[i] && peek(i) == op[i]) ++i;
        if (!op[i]) {
            for (std::size_t j = 0; j < i; ++j) take();
            return;
        }
    }
    if (first <= 0 || first >= 128 || !std::strchr("{}[]#();:?.+-*/%^&|~!=<>,", first))
        token_.kind = PPTokenKind::other;
    take();
}

void PPTokenCursor::update_directive()
{
    if (token_.kind == PPTokenKind::newline) directive_ = Directive::line_start;
    else if (token_.kind != PPTokenKind::whitespace) {
        TextView text = spelling();
        if (directive_ == Directive::line_start && token_.kind == PPTokenKind::punctuation &&
            (text.equals("#") || text.equals("%:"))) directive_ = Directive::after_hash;
        else if (directive_ == Directive::after_hash && token_.kind == PPTokenKind::identifier &&
                 text.equals("include")) directive_ = Directive::after_include;
        else directive_ = Directive::other;
    }
}

PPToken PPTokenCursor::next()
{
    SourceCharacter first = characters_.peek();
    token_ = PPToken();
    token_.file_id = source_.file_id;
    token_.begin = token_.end = first.begin;
    token_.line = first.line; token_.column = first.column;
    copied_ = false;
    translated_.clear();
    int c = first.value;
    if (c == -1) token_.kind = PPTokenKind::eof;
    else if (c == '\n') { token_.kind = PPTokenKind::newline; take(false); }
    else if (horizontal_space(c) || (c == '/' && (peek(1) == '/' || peek(1) == '*'))) whitespace();
    else if (directive_ == Directive::after_include && (c == '<' || c == '"')) header();
    else if (identifier_start(c)) identifier();
    else if (decimal_digit(c) || (c == '.' && decimal_digit(peek(1)))) number();
    else if (c == '\'' || c == '"') literal(false, c);
    else punctuation();
    update_directive();
    if (token_.kind != PPTokenKind::whitespace && token_.kind != PPTokenKind::newline &&
        token_.kind != PPTokenKind::eof) token_.spelling = spelling();
    if (stats_) {
        ++stats_->tokens;
        stats_->spelling_bytes += translated_.size();
    }
    return token_;
}

} // namespace cppgm
