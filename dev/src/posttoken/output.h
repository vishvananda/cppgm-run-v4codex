#pragma once
#include "posttoken/token.h"
#include <iosfwd>

namespace cppgm {
void write_post_token(std::ostream& out, const PostToken& token, const IdentifierTable& identifiers);
}
