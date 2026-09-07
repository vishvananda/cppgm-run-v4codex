#pragma once
#include "posttoken/token.h"

namespace cppgm {
// Consumers request only the numeric domain they can use. Integral-only
// decoding rejects floats/UD numbers without constructing their unused facts.
enum class NumberDomain { all, integral };
void decode_number(PostToken& token, IdentifierTable& identifiers,
                   NumberDomain domain = NumberDomain::all);
// Decode already validated UTF-8 from PA1; shared with literal/UD conversion.
std::uint32_t next_codepoint(TextView text, std::size_t& position);
bool valid_ud_suffix(TextView text);
}
