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
    Location location;
    location.file = post.source.file_id;
    location.begin = post.source.begin;
    location.end = post.source.end;
    location.presumed_file = post.source.presumed_file;
    location.line = post.source.line;
    token.location = ast_.locations.size();
    if (ast_.telemetry && ast_.locations.size() == ast_.locations.capacity()) ++ast_.location_growths;
    ast_.locations.push_back(location);
    if (post.kind == PostTokenKind::literal || post.kind == PostTokenKind::user_literal)
        token.literal = ast_.save_literal(post, post.prefix.size ? ids_.intern(post.prefix) : 0);
    index_delimiter(token);
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
                                 std::string(found.data, found.size) + "' at byte " + std::to_string(ast_.locations[peek().location].begin));
    }
    return take();
}

void Cursor::index_delimiter(Token& token)
{
    if (ast_.telemetry) ++delimiter_work;
    ETokenType close = TOK_INVALID;
    if (token.op == OP_LPAREN) close = OP_RPAREN;
    else if (token.op == OP_LSQUARE) close = OP_RSQUARE;
    else if (token.op == OP_LBRACE) close = OP_RBRACE;
    if (close != TOK_INVALID) {
        delimiters_.push_back(Delimiter{produced, close});
    } else if (!delimiters_.empty() && token.op == delimiters_.back().close) {
        std::size_t open = delimiters_.back().ordinal;
        delimiters_.pop_back();
        if (open >= consumed)
            pending_[(head_ + open - consumed) % pending_.size()].delimiter_end = produced + 1;
    }
}

std::size_t Cursor::matching(std::size_t ahead)
{
    for (;;) {
        Token token = peek(ahead);
        if (token.delimiter_end) return token.delimiter_end - consumed - 1;
        if (count_ && pending_[(head_ + count_ - 1) % pending_.size()].kind == PostTokenKind::eof)
            throw std::runtime_error("unclosed delimiter");
        fill();
    }
}

std::size_t Cursor::angle_end(std::size_t ahead)
{
    Token token = peek(ahead);
    return token.angle_end ? token.angle_end - consumed : ahead;
}

void Cursor::remember_angle(std::size_t open, std::size_t end)
{
    pending_[(head_ + open) % pending_.size()].angle_end = consumed + end;
}

void Cursor::close_angle()
{
    if (is(">>")) {
        Token& token = pending_[head_];
        token.op = OP_GT;
        token.text = ids_.intern(TextView(">", 1));
        Location second = ast_.locations[token.location];
        ++second.begin;
        token.location = ast_.locations.size();
        if (ast_.telemetry && ast_.locations.size() == ast_.locations.capacity()) ++ast_.location_growths;
        ast_.locations.push_back(second);
        return;
    }
    require(">");
}

} }
