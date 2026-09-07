#pragma once

#include "preprocess/identifier_table.h"

namespace cppgm {

enum class PPTokenKind : unsigned char {
    whitespace, newline, header, identifier, number, character,
    user_character, string, user_string, punctuation, other, eof
};

struct PPToken {
    PPTokenKind kind;
    std::uint32_t file_id;
    std::size_t begin, end, line, column;
    IdentifierId identifier = 0;
    IdentifierId suffix = 0;
    TextView spelling;
};

const char* token_kind_name(PPTokenKind kind);

// Pull one token at a time. PPToken has no owning strings. A spelling borrows
// immutable source bytes or this cursor's one reusable translated-token buffer,
// and is valid until next(). Persistent semantic names use IdentifierId.
class PPTokenCursor {
public:
    PPTokenCursor(const SourceBuffer& source, IdentifierTable& identifiers,
                  LexStats* stats = 0);
    PPToken next();
    std::size_t spelling_storage_bytes() const { return translated_.capacity(); }

private:
    enum class Directive { line_start, after_hash, after_include, other };
    const SourceBuffer& source_;
    IdentifierTable& identifiers_;
    LexStats* stats_;
    CharacterCursor characters_;
    Directive directive_ = Directive::line_start;
    PPToken token_;
    std::string translated_;
    bool copied_ = false;

    int peek(std::size_t ahead = 0) { return characters_.peek(ahead).value; }
    int take(bool spelling = true);
    TextView spelling() const;
    void whitespace();
    void identifier();
    void number();
    void punctuation();
    void literal(bool raw, int quote);
    void raw_literal();
    void escape();
    void header();
    void update_directive();
};

} // namespace cppgm
