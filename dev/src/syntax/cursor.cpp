#include "syntax/cursor.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace cppgm { namespace syntax {

Cursor::Cursor(PostTokenCursor& input, IdentifierTable& ids, Ast& ast)
    : input_(input), ids_(ids), ast_(ast), pending_(16) {}

void Cursor::fill()
{
    if (count_ == pending_.size()) {
        std::vector<Token> grown(pending_.size() * 2);
        for (std::size_t i = 0; i < count_; ++i) grown[i] = pending_[(head_ + i) % pending_.size()];
        pending_.swap(grown);
        head_ = 0;
    }
    PostToken post = input_.next();
    if (post.kind == PostTokenKind::invalid) throw std::runtime_error("invalid phase-7 token");
    if (post.source.end > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error("source exceeds compact location capacity");
    Token token;
    token.kind = post.kind;
    token.op = post.simple;
    token.text = post.identifier ? post.identifier : ids_.intern(post.source.spelling);
    token.location.file = post.source.file_id;
    token.location.begin = post.source.begin;
    token.location.end = post.source.end;
    if (post.kind == PostTokenKind::literal || post.kind == PostTokenKind::user_literal)
        token.literal = ast_.save_literal(post);
    pending_[(head_ + count_) % pending_.size()] = token;
    ++count_;
    ++produced;
    max_pending = std::max(max_pending, count_);
}

Token Cursor::peek(std::size_t ahead)
{
    while (count_ <= ahead) fill();
    return pending_[(head_ + ahead) % pending_.size()];
}

Token Cursor::take()
{
    Token result = peek();
    head_ = (head_ + 1) % pending_.size();
    --count_;
    ++consumed;
    return result;
}

bool Cursor::is(const char* spelling, std::size_t ahead)
{
    return ids_.spelling(peek(ahead).text).equals(spelling);
}

bool Cursor::eat(const char* spelling)
{
    if (!is(spelling)) return false;
    take();
    return true;
}

Token Cursor::require(const char* spelling)
{
    if (!is(spelling)) {
        TextView found = ids_.spelling(peek().text);
        throw std::runtime_error(std::string("expected '") + spelling + "', found '" +
                                 std::string(found.data, found.size) + "'");
    }
    return take();
}

void Cursor::close_angle()
{
    if (is(">>")) {
        Token& token = pending_[head_];
        token.op = OP_GT;
        token.text = ids_.intern(TextView(">", 1));
        ++token.location.begin;
        return;
    }
    require(">");
}

} }
