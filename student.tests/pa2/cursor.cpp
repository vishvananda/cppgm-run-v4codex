#include "posttoken/cursor.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>
#include <type_traits>

// Count actual allocation calls in the warmed-up streaming hot path.
static std::size_t allocations;
void* operator new(std::size_t n) {
    ++allocations;
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    return p;
}
void operator delete(void* p) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete[](void* p) noexcept { ::operator delete(p); }

using namespace cppgm;

static void identity_and_locations()
{
    SourceBuffer source("alpha al\\\npha 123_π 'x'_π operator\"\"\\u0073v;", 12);
    IdentifierTable names;
    PPTokenCursor pp(source, names, 0, true);
    PostTokenCursor cursor(pp, names, true);
    PostToken a = cursor.next();
    assert(a.kind == PostTokenKind::identifier && a.source.file_id == 12);
    assert(a.source.spelling.data == source.bytes.data());
    PostToken b = cursor.next();
    assert(a.identifier == b.identifier && b.source.begin == 6 && b.source.end == 13);
    assert(b.source.spelling.equals("alpha"));
    PostToken number = cursor.next();
    assert(number.kind == PostTokenKind::user_literal && number.prefix.equals("123"));
    PostToken character = cursor.next();
    assert(character.suffix == number.suffix && character.scalar[0] == 'x');
    assert(cursor.next().simple == KW_OPERATOR);
    PostToken empty = cursor.next();
    assert(empty.kind == PostTokenKind::literal && empty.source.spelling.equals("\"\""));
    assert(empty.elements == 1 && empty.data.size == 1 && !empty.data.data[0]);
    std::size_t suffix_begin = source.bytes.find("\\u0073");
    PostToken name = cursor.next();
    assert(name.kind == PostTokenKind::identifier && names.spelling(name.identifier).equals("sv"));
    assert(name.source.begin == suffix_begin && name.source.end == source.bytes.size() - 1);
    assert(name.source.line == 2 && name.source.column == suffix_begin - source.bytes.find('\n'));
    assert(cursor.next().simple == OP_SEMICOLON);
    assert(cursor.next().kind == PostTokenKind::eof);
    for (int i = 0; i < 5000; ++i) {
        std::string text = "new_name_" + std::to_string(i);
        names.intern(TextView(text.data(), text.size()));
    }
    assert(names.spelling(a.identifier).equals("alpha"));
    assert(names.spelling(number.suffix).equals("_π"));
}

static void demand_and_concatenation()
{
    SourceBuffer source("int x = 4; \"unterminated\n");
    IdentifierTable names;
    PPTokenCursor pp(source, names, 0, true);
    PostTokenCursor cursor(pp, names);
    assert(cursor.next().simple == KW_INT);
    assert(cursor.next().kind == PostTokenKind::identifier);
    assert(cursor.next().simple == OP_ASS);
    assert(cursor.next().scalar[0] == 4);
    assert(cursor.next().simple == OP_SEMICOLON);
    bool failed = false;
    try { cursor.next(); } catch (const std::runtime_error&) { failed = true; }
    assert(failed);

    SourceBuffer strings("\"\\x3c0\" /* next encoding */ u\"𝄞\"_π al\\\npha '' \"end\"");
    PPTokenCursor strings_pp(strings, names, 0, true);
    PostTokenCursor sequence(strings_pp, names); // no joined source view
    PostToken combined = sequence.next();
    assert(combined.kind == PostTokenKind::user_literal && combined.type == FT_CHAR16_T);
    assert(combined.elements == 4 && combined.source.spelling.size == 0);
    const unsigned char expected[] = { 0xc0, 3, 0x34, 0xd8, 0x1e, 0xdd, 0, 0 };
    for (unsigned i = 0; i < sizeof(expected); ++i)
        assert(static_cast<unsigned char>(combined.data.data[i]) == expected[i]);
    assert(sequence.next().source.spelling.equals("alpha")); // transformed lookahead remains live
    assert(sequence.next().kind == PostTokenKind::invalid);
    assert(sequence.next().elements == 4);
    assert(sequence.next().kind == PostTokenKind::eof);
}

static void steady_storage()
{
    const std::string pattern = "name 2147483648 0xffffffffffffffffu 123_tag 'π' u\"x\" \"y\"_tag ; ";
    std::string input;
    for (int i = 0; i < 2000; ++i) input += pattern;
    SourceBuffer source(std::move(input));
    LexStats lex;
    PostStats stats;
    IdentifierTable names(&lex);
    PPTokenCursor pp(source, names, &lex, true);
    PostTokenCursor cursor(pp, names, false, &stats);
    for (int i = 0; i < 7; ++i) assert(cursor.next().kind != PostTokenKind::eof);
    std::size_t before = allocations, storage = cursor.storage_bytes();
    while (cursor.next().kind != PostTokenKind::eof) {}
    assert(before == allocations);
    assert(cursor.storage_bytes() == storage);
    assert(stats.tokens == 14001 && stats.invalid == 0);
    assert(stats.string_parts == 4000 && stats.encoded_bytes == 12000);
}

int main()
{
    static_assert(std::is_trivially_copyable<PostToken>::value, "borrowed views and inline scalar payloads");
    identity_and_locations();
    demand_and_concatenation();
    steady_storage();
    std::cout << "post-token identity, locations, streaming, concatenation and hot-path allocation: passed\n";
}
