#pragma once
#include "syntax/ast.h"
#include "posttoken/cursor.h"

namespace cppgm { namespace syntax {

// Ring lookahead grows only for unresolved syntactic prefixes. Consumed tokens
// are discarded immediately. No owning post-token or recognition-token stream.
class Cursor {
public:
    Cursor(PostTokenCursor& input, IdentifierTable& ids, Ast& ast);
    Token peek(std::size_t ahead = 0);
    Token take();
    bool is(const char* spelling, std::size_t ahead = 0);
    bool eat(const char* spelling);
    Token require(const char* spelling);
    void close_angle();
    std::size_t matching(std::size_t ahead);
    std::size_t angle_end(std::size_t ahead);
    void remember_angle(std::size_t open, std::size_t end);
    std::size_t delimiter_work = 0;
    std::size_t consumed = 0, produced = 0, max_pending = 0;
private:
    PostTokenCursor& input_;
    IdentifierTable& ids_;
    Ast& ast_;
    std::vector<Token> pending_;
    std::size_t head_ = 0, count_ = 0;
    struct Delimiter { std::size_t ordinal; ETokenType close; };
    std::vector<Delimiter> delimiters_;
    void index_delimiter(Token& token);
    void fill();
};

} }
