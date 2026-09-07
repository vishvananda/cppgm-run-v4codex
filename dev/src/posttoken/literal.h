#pragma once
#include "posttoken/token.h"

namespace cppgm {

enum class Encoding : unsigned char { ordinary, utf8, utf16, utf32, wide };

// Numeric escapes remain distinct from Unicode scalar values until the final
// string encoding is known. Overflow is sticky; long hex escapes cannot wrap.
struct LiteralElement {
    std::uint32_t value = 0;
    bool numeric = false, valid = true;
};

class LiteralReader {
public:
    LiteralReader(TextView spelling, std::size_t suffix_bytes);
    Encoding encoding() const { return encoding_; }
    bool next(LiteralElement& element);
private:
    TextView text_;
    Encoding encoding_ = Encoding::ordinary;
    std::size_t position_ = 0, end_ = 0;
    bool raw_ = false;
};

EFundamentalType encoding_type(Encoding encoding);
void append_code_unit(std::string& bytes, std::uint32_t value, unsigned width);
bool append_string_element(std::string& bytes, LiteralElement element, Encoding encoding);
void decode_character(PostToken& token, const IdentifierTable& identifiers, PostStats* stats);

}
