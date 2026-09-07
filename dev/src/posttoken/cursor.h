#pragma once
#include "posttoken/literal.h"
#include <vector>

namespace cppgm {

// Borrows the TU's source and identifier table. A single PP lookahead and one
// maximal string's decoded elements suffice. No owning vectors of tokens.
// retain_source is for the explicit PA2/debug view; production need not build
// the joined spellings. All views expire on next(), identities remain TU-stable.
class PostTokenCursor {
public:
    PostTokenCursor(PPTokenSource& input, IdentifierTable& identifiers,
                    bool retain_source = false, PostStats* stats = 0);
    PostToken next();
    std::size_t storage_bytes() const;
private:
    PPTokenSource& input_;
    IdentifierTable& identifiers_;
    bool retain_source_;
    PostStats* stats_;
    PPToken pending_;
    bool has_pending_ = false, after_operator_ = false;
    std::vector<LiteralElement> elements_;
    std::string joined_source_, bytes_;

    PPToken take();
    PostToken string_sequence(PPToken first);
};

}
