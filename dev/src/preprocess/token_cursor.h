#pragma once

#include "preprocess/identifier_table.h"

namespace cppgm {

enum class PPTokenKind : unsigned char {
    whitespace, newline, header, identifier, number, character,
    user_character, string, user_string, punctuation, other, eof
};

struct PPToken {
    PPTokenKind kind;
    unsigned char packing = 0; // Snapshot at preprocessing output, before parser lookahead.
    std::uint32_t file_id;
    std::size_t begin, end, line, column;
    IdentifierId identifier = 0;
    IdentifierId presumed_file = 0; // PA4 logical filename; file_id/offset stay physical
    IdentifierId suffix = 0;
    std::size_t suffix_begin = 0, suffix_line = 0, suffix_column = 0;
    TextView spelling;
};

const char* token_kind_name(PPTokenKind kind);

// Pull one token at a time. PPToken has no owning strings. A spelling borrows
// immutable source bytes or this cursor's one reusable translated-token buffer,
// and is valid until next(). Persistent semantic names use IdentifierId.
class PPTokenSource {
public:
    virtual ~PPTokenSource() {}
    virtual PPToken next() = 0;
};

class PPTokenCursor : public PPTokenSource {
public:
    PPTokenCursor(const SourceBuffer& source, IdentifierTable& identifiers,
                  LexStats* stats = 0, bool recover_empty_character = false,
                  bool translated_input = false);
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
    bool recover_empty_character_;

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
