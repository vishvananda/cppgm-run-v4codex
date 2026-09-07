#pragma once
#include "posttoken/token.h"

namespace cppgm {
void decode_number(PostToken& token, IdentifierTable& identifiers);
// Decode already validated UTF-8 from PA1; shared with literal/UD conversion.
std::uint32_t next_codepoint(TextView text, std::size_t& position);
bool valid_ud_suffix(TextView text);
}
