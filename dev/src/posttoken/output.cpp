// PA2 output vocabulary inherited from the CPPGM starter; see NOTICE.
#include "posttoken/output.h"
#include <ostream>

namespace cppgm {

static void write_text(std::ostream& out, TextView text) { out.write(text.data, text.size); }

static void write_hex(std::ostream& out, TextView bytes)
{
    static const char digits[] = "0123456789ABCDEF";
    char buffer[4096];
    std::size_t p = 0;
    while (p < bytes.size) {
        std::size_t used = 0;
        while (p < bytes.size && used < sizeof(buffer)) {
            unsigned char c = bytes.data[p++];
            buffer[used++] = digits[c >> 4]; buffer[used++] = digits[c & 15];
        }
        out.write(buffer, used);
    }
}

void write_post_token(std::ostream& out, const PostToken& token, const IdentifierTable& identifiers)
{
    static const char* const kinds[] = { "invalid", "simple", "identifier", "literal", "user-defined-literal", "eof" };
    out << kinds[static_cast<unsigned>(token.kind)];
    if (token.kind != PostTokenKind::eof) { out << ' '; write_text(out, token.source.spelling); }
    if (token.kind == PostTokenKind::simple) out << ' ' << simple_name(token.simple);
    bool literal = token.kind == PostTokenKind::literal || token.kind == PostTokenKind::user_literal;
    bool user_number = false;
    if (token.kind == PostTokenKind::user_literal) {
        out << ' '; write_text(out, identifiers.spelling(token.suffix));
        static const char* const categories[] = { "integer", "floating", "character", "string" };
        out << ' ' << categories[static_cast<unsigned>(token.literal)];
        user_number = token.literal == LiteralKind::integer || token.literal == LiteralKind::floating;
    }
    if (user_number) { out << ' '; write_text(out, token.prefix); }
    else if (literal) {
        if (token.literal == LiteralKind::string) out << " array of " << token.elements;
        out << ' ' << fundamental_name(token.type) << ' ';
        write_hex(out, token.literal == LiteralKind::string ? token.data :
                  TextView(token.scalar.data(), fundamental_width(token.type)));
    }
    out << '\n';
}

}
