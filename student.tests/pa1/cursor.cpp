#include "preprocess/token_cursor.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <type_traits>

using namespace cppgm;

static PPToken nonspace(PPTokenCursor& cursor)
{
    PPToken token;
    do { token = cursor.next(); }
    while (token.kind == PPTokenKind::whitespace || token.kind == PPTokenKind::newline);
    return token;
}

static void locations_and_identity()
{
    SourceBuffer source("alpha al\\\npha \\u03C0 π\nR\"(x\ny)\"_tag z", 17);
    LexStats stats;
    IdentifierTable names(&stats);
    PPTokenCursor cursor(source, names, &stats);
    PPToken first = nonspace(cursor);
    assert(first.kind == PPTokenKind::identifier && first.spelling.equals("alpha"));
    assert(first.spelling.data == source.bytes.data());
    assert(first.begin == 0 && first.end == 5 && first.line == 1 && first.column == 1);
    assert(first.file_id == 17);
    PPToken spliced = nonspace(cursor);
    assert(spliced.identifier == first.identifier && spliced.spelling.equals("alpha"));
    assert(spliced.begin == 6 && spliced.end == 13 && spliced.line == 1 && spliced.column == 7);
    PPToken ucn = nonspace(cursor);
    assert(ucn.spelling.equals("π") && ucn.line == 2 && ucn.column == 5);
    PPToken utf8 = nonspace(cursor);
    assert(ucn.identifier == utf8.identifier && utf8.spelling.equals("π"));
    assert(utf8.spelling.data == source.bytes.data() + utf8.begin);
    PPToken raw = nonspace(cursor);
    assert(raw.kind == PPTokenKind::user_string && raw.line == 3 && raw.column == 1);
    assert(raw.spelling.equals("R\"(x\ny)\"_tag") && names.spelling(raw.suffix).equals("_tag"));
    assert(raw.spelling.data == source.bytes.data() + raw.begin);
    PPToken last = nonspace(cursor);
    assert(last.spelling.equals("z") && last.line == 4 && last.column == 9);
    assert(nonspace(cursor).kind == PPTokenKind::eof);
    assert(nonspace(cursor).kind == PPTokenKind::eof);
    assert(stats.spelling_bytes == 7); // only alpha's splice and the UCN copied

    // Reallocation and subsequent source lifetimes cannot change name identity.
    for (int i = 0; i < 20000; ++i) {
        std::string name = "name_" + std::to_string(i);
        IdentifierId id = names.intern(TextView(name.data(), name.size()));
        assert(names.spelling(id).equals(name.c_str()));
    }
    assert(names.spelling(first.identifier).equals("alpha"));
    assert(names.spelling(ucn.identifier).equals("π"));
    assert(names.spelling(raw.suffix).equals("_tag"));
    for (int i = 19999; i >= 0; --i) {
        std::string name = "name_" + std::to_string(i);
        assert(names.intern(TextView(name.data(), name.size())) == static_cast<IdentifierId>(i + 5));
    }
    SourceBuffer other("alpha π", 18);
    PPTokenCursor again(other, names);
    assert(nonspace(again).identifier == first.identifier);
    assert(nonspace(again).identifier == ucn.identifier);
}

static void streaming_and_modes()
{
    SourceBuffer source("first\n\"unterminated\n");
    IdentifierTable names;
    PPTokenCursor cursor(source, names);
    assert(cursor.next().spelling.equals("first"));
    assert(cursor.next().kind == PPTokenKind::newline);
    bool failed = false;
    try { cursor.next(); } catch (const std::runtime_error&) { failed = true; }
    assert(failed); // No eager tokenization/validation of the whole file.

    SourceBuffer raw("R\"abcdefghijklmnop()abcdefghijklmno)abcdefghijklmnop\"\\u03C0 next");
    PPTokenCursor raw_cursor(raw, names);
    PPToken token = raw_cursor.next();
    assert(token.kind == PPTokenKind::user_string);
    assert(names.spelling(token.suffix).equals("π"));
    assert(token.spelling.equals("R\"abcdefghijklmnop()abcdefghijklmno)abcdefghijklmnop\"π"));
    assert(nonspace(raw_cursor).spelling.equals("next"));
}

static void completed_name_lookup()
{
    LexStats stats;
    IdentifierTable names(&stats);
    for (int i = 0; i < 8; ++i) {
        std::string name = "name_" + std::to_string(i);
        names.intern(TextView(name.data(), name.size()));
    }
    const std::size_t storage = names.storage_bytes();
    const std::size_t growths = stats.storage_growths;
    for (int i = 0; i < 1000; ++i)
        assert(names.intern(TextView("name_7", 6)) == 8);
    assert(names.storage_bytes() == storage);
    assert(stats.storage_growths == growths);
    assert(names.intern(TextView("next", 4)) == 9);
    assert(names.spelling(8).equals("name_7"));
}

int main()
{
    static_assert(std::is_trivially_copyable<PPToken>::value, "tokens must not own storage");
    locations_and_identity();
    streaming_and_modes();
    completed_name_lookup();
    std::cout << "cursor ownership, identity, locations and streaming: passed\n";
}
