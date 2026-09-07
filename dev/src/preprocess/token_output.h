#pragma once

#include "preprocess/token_cursor.h"
#include <iosfwd>

namespace cppgm {
// Explicit PA1 debug view. Production consumers use PPTokenCursor directly.
void write_pp_token(std::ostream& out, const PPToken& token);
}
