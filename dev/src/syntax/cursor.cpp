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
    if (post.kind == PostTokenKind::invalid) {
        auto file = ids_.spelling(post.source.presumed_file);
        throw std::runtime_error("invalid phase-7 token '" + std::string(post.source.spelling.data,post.source.spelling.size) +
            "' in " + std::string(file.data,file.size) + ":" + std::to_string(post.source.line));
    }
    if (post.source.end > std::numeric_limits<std::uint32_t>::max())
        throw std::runtime_error("source exceeds compact location capacity");
    Token token;
    token.kind = post.kind;
    token.packing = post.source.packing;
    token.op = post.simple;
    token.text = post.identifier ? post.identifier : ids_.intern(post.source.spelling);
    Location location;
    location.file = post.source.file_id;
    location.begin = post.source.begin;
    location.end = post.source.end;
    location.presumed_file = post.source.presumed_file;
    location.line = post.source.line; location.column = post.source.column;
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

Token Cursor::take()
{
    Token result = peek();
    head_ = (head_ + 1) % pending_.size();
    --count_;
    ++consumed;
    return result;
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
        const auto& loc = ast_.locations[peek().location];
        TextView file = ids_.spelling(loc.presumed_file);
        throw std::runtime_error(std::string("expected '") + spelling + "', found '" +
                                 std::string(found.data, found.size) + "' in " + std::string(file.data,file.size) + ":" + std::to_string(loc.line));
    }
    return take();
}

std::string Cursor::position()
{
    const auto& loc = ast_.locations[peek().location];
    auto file = ids_.spelling(loc.presumed_file), word = ids_.spelling(peek().text);
    return std::string(file.data,file.size) + ":" + std::to_string(loc.line) +
        " near '" + std::string(word.data,word.size) + "'";
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
    if (token.angle_end == std::size_t(-1)) return token.angle_end;
    return token.angle_end ? token.angle_end - consumed : ahead;
}

void Cursor::remember_angle(std::size_t open, std::size_t end, bool split)
{
    auto& token = pending_[(head_ + open) % pending_.size()];
    token.angle_end = end == std::size_t(-1) ? end : consumed + end;
    token.split_angle_end = split;
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
        ++second.column;
        ast_.locations.push_back(second);
        return;
    }
    require(">");
}

} }
