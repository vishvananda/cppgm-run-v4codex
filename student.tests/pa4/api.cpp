// Direct structured-cursor ownership/location/work checks, independent of dumps.
#include "preprocess/preprocessor.h"
#include "posttoken/cursor.h"
#include <cassert>
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
    assert(argc == 2);
    std::string path = argv[1];
    {
        std::ofstream out(path.c_str());
        out << "#define ID(x) x\n#line 71 \"logical.cc\"\nID(name) name\n";
    }
    cppgm::Preprocessor pp(path, "Sep  7 2026", "12:00:00", true);
    cppgm::PostTokenCursor cursor(pp, pp.identifiers());
    cppgm::PostToken first = cursor.next();
    assert(first.kind == cppgm::PostTokenKind::identifier && first.source.line == 71);
    assert(first.source.file_id == 1 && first.source.begin > 0);
    assert(pp.identifiers().spelling(first.source.presumed_file).equals("logical.cc"));
    cppgm::IdentifierId id = first.identifier;
    assert(cursor.next().identifier == id);
    assert(cursor.next().kind == cppgm::PostTokenKind::eof);
    {
        std::ofstream out(path.c_str());
        out << "#define SUFFIX \"\"_custom\n#line 80 \"logical.cc\"\noperator SUFFIX\noperator \"\"_direct\n";
    }
    cppgm::Preprocessor suffixes(path, "Sep  7 2026", "12:00:00");
    cppgm::PostTokenCursor suffix_cursor(suffixes, suffixes.identifiers());
    for (std::size_t line = 80; line <= 81; ++line) {
        assert(suffix_cursor.next().kind == cppgm::PostTokenKind::simple);
        assert(suffix_cursor.next().kind == cppgm::PostTokenKind::literal);
        cppgm::PostToken suffix = suffix_cursor.next();
        assert(suffix.kind == cppgm::PostTokenKind::identifier);
        assert(suffix.source.line == line);
        assert(suffixes.identifiers().spelling(suffix.source.presumed_file).equals("logical.cc"));
    }
    {
        std::ofstream out(path.c_str());
        out << "#define I(x) x\n";
        for (int repeat = 0; repeat < 3; ++repeat) {
            for (int i = 0; i < 20000; ++i) out << "I(";
            out << "42";
            for (int i = 0; i < 20000; ++i) out << ')';
            out << '\n';
        }
    }
    cppgm::Preprocessor nested(path, "Sep  7 2026", "12:00:00", true);
    assert(nested.next().spelling.equals("42"));
    assert(nested.next().spelling.equals("42"));
    cppgm::PreprocessStats warm = nested.stats();
    assert(nested.next().spelling.equals("42"));
    assert(nested.next().kind == cppgm::PPTokenKind::eof);
    assert(nested.stats().captured_tokens == 180000);
    assert(nested.stats().borrowed_arguments == 59997);
    assert(nested.stats().argument_prescans == 60000);
    assert(nested.stats().max_prescan_depth == 20000);
    assert(nested.stats().task_slabs == 625);
    assert(nested.stats().argument_growths == 20000);
    assert(nested.stats().prescan_output_growths <= 40000);
    assert(nested.stats().task_slabs == warm.task_slabs);
    assert(nested.stats().argument_growths == warm.argument_growths);
    assert(nested.stats().prescan_output_growths == warm.prescan_output_growths);
    {
        std::ofstream out(path.c_str());
        for (int i = 0; i < 100000; ++i) out << "__COUNTER__ ";
    }
    cppgm::Preprocessor counters(path, "Sep  7 2026", "12:00:00", true);
    for (int i = 0; i < 100000; ++i) {
        cppgm::PPToken token = counters.next();
        assert(token.spelling.equals(std::to_string(i).c_str()));
    }
    assert(counters.next().kind == cppgm::PPTokenKind::eof);
    assert(counters.stats().arena_bytes <= 131072);
    std::cout << "PA4 API checks passed: locations, identities, 3x20000 pooled prescans, bounded generated spelling\n";
}
