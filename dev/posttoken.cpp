// (C) 2013 CPPGM Foundation www.cppgm.org. All rights reserved.
// Adapted from the CPPGM PA2 starter; see NOTICE for attribution.
#include "posttoken/cursor.h"
#include "posttoken/output.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/resource.h>

int main(int argc, char** argv)
{
    try {
        bool report_stats = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--stats") report_stats = true;
            else throw std::runtime_error("usage: posttoken [--stats] < source");
        }
        typedef std::chrono::steady_clock Clock;
        Clock::time_point start = Clock::now();
        std::string input;
        char buffer[65536];
        while (std::cin.read(buffer, sizeof(buffer)) || std::cin.gcount())
            input.append(buffer, static_cast<std::size_t>(std::cin.gcount()));
        if (std::cin.bad()) throw std::runtime_error("cannot read source");
        cppgm::SourceBuffer source(std::move(input));
        Clock::time_point read_end = Clock::now();
        cppgm::LexStats stats;
        cppgm::IdentifierTable identifiers(report_stats ? &stats : 0);
        cppgm::PPTokenCursor pp(source, identifiers, report_stats ? &stats : 0, true);
        cppgm::PostStats post_stats;
        cppgm::PostTokenCursor cursor(pp, identifiers, true, report_stats ? &post_stats : 0);
        for (;;) {
            cppgm::PostToken token = cursor.next();
            cppgm::write_post_token(std::cout, token, identifiers);
            if (token.kind == cppgm::PostTokenKind::eof) break;
        }
        std::cout.flush();
        if (!std::cout) throw std::runtime_error("cannot write tokens");
        if (report_stats) {
            Clock::time_point end = Clock::now();
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            std::cerr << "{\"source_bytes\":" << source.bytes.size()
                << ",\"source_storage_bytes\":" << source.bytes.capacity()
                << ",\"read_ms\":" << std::chrono::duration<double, std::milli>(read_end - start).count()
                << ",\"scan_emit_ms\":" << std::chrono::duration<double, std::milli>(end - read_end).count()
                << ",\"peak_rss_kib\":" << usage.ru_maxrss
                << ",\"decoded_units\":" << stats.decoded_units
                << ",\"translated_units\":" << stats.translated_units
                << ",\"tokens\":" << stats.tokens
                << ",\"spelling_bytes\":" << stats.spelling_bytes
                << ",\"spelling_storage_bytes\":" << pp.spelling_storage_bytes()
                << ",\"identifiers\":" << identifiers.size()
                << ",\"identifier_storage_bytes\":" << identifiers.storage_bytes()
                << ",\"intern_probes\":" << stats.intern_probes
                << ",\"rehash_probes\":" << stats.rehash_probes
                << ",\"storage_growths\":" << stats.storage_growths
                << ",\"post_tokens\":" << post_stats.tokens
                << ",\"invalid_tokens\":" << post_stats.invalid
                << ",\"number_bytes\":" << post_stats.number_bytes
                << ",\"literal_bytes\":" << post_stats.literal_bytes
                << ",\"decoded_elements\":" << post_stats.decoded_elements
                << ",\"encoded_bytes\":" << post_stats.encoded_bytes
                << ",\"string_parts\":" << post_stats.string_parts
                << ",\"post_storage_bytes\":" << cursor.storage_bytes()
                << ",\"post_storage_growths\":" << post_stats.storage_growths << "}\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
