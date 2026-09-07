#include "posttoken/literal.h"
#include "posttoken/number.h"
#include <cstring>
#include <limits>

namespace cppgm {

LiteralReader::LiteralReader(TextView spelling, std::size_t suffix_bytes) : text_(spelling)
{
    if (text_.data[0] == 'u') {
        position_ = 1; encoding_ = Encoding::utf16;
        if (text_.data[1] == '8') { position_ = 2; encoding_ = Encoding::utf8; }
    } else if (text_.data[0] == 'U') { position_ = 1; encoding_ = Encoding::utf32; }
    else if (text_.data[0] == 'L') { position_ = 1; encoding_ = Encoding::wide; }
    raw_ = text_.data[position_] == 'R';
    position_ += raw_ ? 2 : 1; // opening quote
    end_ = text_.size - suffix_bytes - 1; // closing quote
    if (raw_) {
        std::size_t delimiter = position_;
        while (text_.data[position_] != '(') ++position_;
        end_ -= position_ - delimiter + 1; // ')' and delimiter
        ++position_;
    }
}

bool LiteralReader::next(LiteralElement& element)
{
    if (position_ == end_) return false;
    element = LiteralElement();
    element.value = next_codepoint(text_, position_);
    if (raw_ || element.value != '\\') return true;
    unsigned char c = text_.data[position_++];
    bool octal = c >= '0' && c <= '7';
    if (octal || c == 'x') {
        element.numeric = true;
        unsigned base = octal ? 8 : 16;
        unsigned limit = octal ? 2 : std::numeric_limits<unsigned>::max();
        element.value = octal ? c - '0' : 0;
        while (position_ < end_ && limit) {
            int digit = hex_value(text_.data[position_]);
            if (digit < 0 || static_cast<unsigned>(digit) >= base) break;
            if (element.value > (0xffffffffu - digit) / base) element.valid = false;
            element.value = element.value * base + digit;
            ++position_;
            if (octal) --limit;
        }
        return true;
    }
    const char* keys = "'\"?\\abfnrtv";
    const char* values = "'\"?\\\a\b\f\n\r\t\v";
    const char* found = std::strchr(keys, c);
    // PA1 has already checked escape syntax (and decoded real UCNs).
    element.valid = found != 0;
    element.value = found ? static_cast<unsigned char>(values[found - keys]) : 0;
    return true;
}

EFundamentalType encoding_type(Encoding encoding)
{
    switch (encoding) {
    case Encoding::utf16: return FT_CHAR16_T;
    case Encoding::utf32: return FT_CHAR32_T;
    case Encoding::wide: return FT_WCHAR_T;
    default: return FT_CHAR;
    }
}

void append_code_unit(std::string& bytes, std::uint32_t value, unsigned width)
{
    for (unsigned i = 0; i < width; ++i) bytes.push_back(static_cast<char>(value >> (8 * i)));
}

bool append_string_element(std::string& bytes, LiteralElement element, Encoding encoding)
{
    if (!element.valid) return false;
    std::uint32_t value = element.value;
    unsigned width = fundamental_width(encoding_type(encoding));
    if (element.numeric) {
        if ((width == 1 && value > 255) || (width == 2 && value > 65535)) return false;
        append_code_unit(bytes, value, width);
    } else if (width == 1) append_utf8(bytes, value);
    else if (width == 2 && value > 65535) {
        value -= 0x10000;
        append_code_unit(bytes, 0xd800 + (value >> 10), 2);
        append_code_unit(bytes, 0xdc00 + (value & 1023), 2);
    } else append_code_unit(bytes, value, width);
    return true;
}

void decode_character(PostToken& token, const IdentifierTable& identifiers, PostStats* stats)
{
    TextView suffix = token.source.suffix ? identifiers.spelling(token.source.suffix) : TextView();
    token.suffix = token.source.suffix;
    if (suffix.size && !valid_ud_suffix(suffix)) return;
    LiteralReader reader(token.source.spelling, suffix.size);
    LiteralElement element, extra;
    if (!reader.next(element)) return;
    if (stats) ++stats->decoded_elements;
    if (reader.next(extra)) {
        if (stats) ++stats->decoded_elements;
        return;
    }
    if (!element.valid || element.value > 0x10ffff ||
        (element.value >= 0xd800 && element.value <= 0xdfff)) return;
    token.type = encoding_type(reader.encoding());
    if (token.type == FT_CHAR && element.value > 127) token.type = FT_INT;
    if (token.type == FT_CHAR16_T && element.value > 65535) return;
    token.literal = LiteralKind::character;
    token.kind = suffix.size ? PostTokenKind::user_literal : PostTokenKind::literal;
    for (unsigned i = 0; i < fundamental_width(token.type); ++i)
        token.scalar[i] = static_cast<char>(element.value >> (8 * i));
}

}
