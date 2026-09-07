#pragma once

#include "posttoken/token_types.h"
#include "preprocess/token_cursor.h"
#include <array>

namespace cppgm {

enum class PostTokenKind : unsigned char { invalid, simple, identifier, literal, user_literal, eof };
enum class LiteralKind : unsigned char { integer, floating, character, string };

// No owning strings or per-token allocation. Views are borrowed until next().
// A later persistent consumer copies only demanded literal values into its arena.
struct PostToken {
    PostTokenKind kind = PostTokenKind::invalid;
    PPToken source;
    ETokenType simple = TOK_INVALID;
    IdentifierId identifier = 0, suffix = 0;
    LiteralKind literal = LiteralKind::integer;
    EFundamentalType type = FT_INT;
    std::array<char, 16> scalar = {{0}};
    TextView data; // array code units; scalar bytes are inline above
    TextView prefix; // UD numeric spelling, never a semantic equality key
    std::size_t elements = 0;
};

struct PostStats {
    std::size_t tokens = 0, invalid = 0, number_bytes = 0, literal_bytes = 0;
    std::size_t decoded_elements = 0, encoded_bytes = 0, string_parts = 0;
    std::size_t storage_growths = 0;
};

}
