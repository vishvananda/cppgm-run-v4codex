#include "syntax/ast.h"
#include <iostream>
struct EntryToken {
    cppgm::IdentifierId text = 0;
    std::uint32_t location = 0, literal = 0;
    std::size_t delimiter_end = 0, angle_end = 0;
    cppgm::ETokenType op = cppgm::TOK_INVALID;
    cppgm::PostTokenKind kind = cppgm::PostTokenKind::eof;
    unsigned char packing = 0;
};
int main() { std::cout << sizeof(EntryToken) << ' ' << sizeof(cppgm::syntax::Token) << '\n'; }
