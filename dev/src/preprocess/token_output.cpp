#include "preprocess/token_output.h"
#include <ostream>

namespace cppgm {

void write_pp_token(std::ostream& out, const PPToken& token)
{
    out << token_kind_name(token.kind);
    if (token.kind != PPTokenKind::eof) {
        out << ' ' << token.spelling.size << ' ';
        out.write(token.spelling.data, token.spelling.size);
    }
    out << '\n';
}

}
