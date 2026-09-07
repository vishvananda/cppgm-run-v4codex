#include "posttoken/cursor.h"
#include "posttoken/number.h"

namespace cppgm {

PostTokenCursor::PostTokenCursor(PPTokenSource& input, IdentifierTable& identifiers,
                               bool retain_source, PostStats* stats)
    : input_(input), identifiers_(identifiers), retain_source_(retain_source), stats_(stats) {}

std::size_t PostTokenCursor::storage_bytes() const
{
    return elements_.capacity() * sizeof(LiteralElement) + joined_source_.capacity() + bytes_.capacity();
}

PPToken PostTokenCursor::take()
{
    PPToken result;
    if (has_pending_) { result = pending_; has_pending_ = false; }
    else result = input_.next();
    while (result.kind == PPTokenKind::whitespace || result.kind == PPTokenKind::newline)
        result = input_.next();
    return result;
}

PostToken PostTokenCursor::string_sequence(PPToken part)
{
    PostToken token;
    token.source = part;
    token.literal = LiteralKind::string;
    Encoding encoding = Encoding::ordinary;
    bool valid = true;
    bool first_part = true;
    elements_.clear(); joined_source_.clear(); bytes_.clear();
    for (;;) {
        TextView suffix = part.suffix ? identifiers_.spelling(part.suffix) : TextView();
        // A literal-operator-id consumes an empty ordinary string and a name.
        // Split by grammar context, including reserved names, not by spelling
        // of any particular library suffix. The borrowed PP buffer stays live.
        bool split = after_operator_ && part.spelling.size == suffix.size + 2 &&
                     part.spelling.data[0] == '"' && suffix.size;
        after_operator_ = false;
        if (split) {
            pending_ = part;
            pending_.kind = PPTokenKind::identifier;
            pending_.identifier = part.suffix;
            pending_.suffix = 0;
            pending_.spelling = TextView(part.spelling.data + 2, suffix.size);
            pending_.begin = part.suffix_begin;
            pending_.line = part.suffix_line; pending_.column = part.suffix_column;
            has_pending_ = true;
            part.spelling.size = 2;
            part.end = part.suffix_begin;
            part.suffix = 0; suffix = TextView();
        }
        // The source anchor belongs to the first fragment's physical file.
        // Concatenation may cross an include; offsets from another file cannot
        // form its end. Keep split literal-operator spans exact on the first part.
        if (first_part || (part.file_id == token.source.file_id && part.end > token.source.end))
            token.source.end = part.end;
        first_part = false;
        if (retain_source_) {
            std::size_t size = joined_source_.size() + part.spelling.size + !joined_source_.empty();
            if (stats_ && size > joined_source_.capacity()) ++stats_->storage_growths;
            if (!joined_source_.empty()) joined_source_.push_back(' ');
            joined_source_.append(part.spelling.data, part.spelling.size);
        }
        LiteralReader reader(part.spelling, suffix.size);
        Encoding prefix = reader.encoding();
        if (prefix != Encoding::ordinary) {
            if (encoding != Encoding::ordinary && encoding != prefix) valid = false;
            encoding = prefix;
        }
        if (part.suffix) {
            if (!valid_ud_suffix(suffix) || (token.suffix && token.suffix != part.suffix)) valid = false;
            token.suffix = part.suffix;
        }
        if (stats_) { ++stats_->string_parts; stats_->literal_bytes += part.spelling.size; }
        LiteralElement element;
        // Once a sequence is invalid, consume its remaining tokens for recovery
        // but do not allocate unusable values. The full source view is optional.
        while (valid && reader.next(element)) {
            if (stats_) {
                ++stats_->decoded_elements;
                if (elements_.size() == elements_.capacity()) ++stats_->storage_growths;
            }
            valid = element.valid;
            elements_.push_back(element);
        }
        part = take();
        if (part.kind != PPTokenKind::string && part.kind != PPTokenKind::user_string) break;
    }
    pending_ = part; has_pending_ = true;
    token.source.spelling = TextView(joined_source_.data(), joined_source_.size());
    token.type = encoding_type(encoding);
    if (valid) {
        std::size_t capacity = bytes_.capacity();
        for (const LiteralElement& element : elements_) {
            if (!append_string_element(bytes_, element, encoding)) { valid = false; break; }
            if (stats_ && capacity != bytes_.capacity()) { ++stats_->storage_growths; capacity = bytes_.capacity(); }
        }
        unsigned width = fundamental_width(token.type);
        if (stats_ && bytes_.size() + width > bytes_.capacity()) ++stats_->storage_growths;
        append_code_unit(bytes_, 0, width);
        token.elements = bytes_.size() / width;
        token.data = TextView(bytes_.data(), bytes_.size());
        if (stats_) stats_->encoded_bytes += bytes_.size();
    }
    if (valid) token.kind = token.suffix ? PostTokenKind::user_literal : PostTokenKind::literal;
    return token;
}

PostToken PostTokenCursor::next()
{
    PPToken pp = take();
    PostToken token;
    token.source = pp;
    switch (pp.kind) {
    case PPTokenKind::identifier:
    case PPTokenKind::punctuation:
        token.simple = classify_simple(pp.spelling);
        if (token.simple != TOK_INVALID) token.kind = PostTokenKind::simple;
        else if (pp.kind == PPTokenKind::identifier) {
            token.kind = PostTokenKind::identifier; token.identifier = pp.identifier;
        }
        break;
    case PPTokenKind::number:
        if (stats_) stats_->number_bytes += pp.spelling.size;
        decode_number(token, identifiers_);
        break;
    case PPTokenKind::character:
    case PPTokenKind::user_character:
        if (stats_) stats_->literal_bytes += pp.spelling.size;
        decode_character(token, identifiers_, stats_);
        break;
    case PPTokenKind::string:
    case PPTokenKind::user_string:
        token = string_sequence(pp);
        break;
    case PPTokenKind::eof: token.kind = PostTokenKind::eof; break;
    default: break;
    }
    after_operator_ = token.kind == PostTokenKind::simple && token.simple == KW_OPERATOR;
    if (stats_) {
        ++stats_->tokens;
        stats_->invalid += token.kind == PostTokenKind::invalid;
    }
    return token;
}

}
