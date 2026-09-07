// Adapted from the CPPGM PA1 starter; see NOTICE for attribution.
#include "preprocess/token_output.h"

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
            else throw std::runtime_error("usage: pptoken [--stats] < source");
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
        cppgm::PPTokenCursor cursor(source, identifiers, report_stats ? &stats : 0);
        for (;;) {
            cppgm::PPToken token = cursor.next();
            cppgm::write_pp_token(std::cout, token);
            if (token.kind == cppgm::PPTokenKind::eof) break;
        }
        std::cout.flush();
        if (!std::cout) throw std::runtime_error("cannot write tokens");
        if (report_stats) {
            Clock::time_point end = Clock::now();
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            std::cerr << "{\"source_bytes\":" << source.bytes.size()
                << ",\"read_ms\":" << std::chrono::duration<double, std::milli>(read_end - start).count()
                << ",\"scan_emit_ms\":" << std::chrono::duration<double, std::milli>(end - read_end).count()
                << ",\"peak_rss_kib\":" << usage.ru_maxrss
                << ",\"decoded_units\":" << stats.decoded_units
                << ",\"translated_units\":" << stats.translated_units
                << ",\"tokens\":" << stats.tokens
                << ",\"spelling_bytes\":" << stats.spelling_bytes
                << ",\"identifiers\":" << identifiers.size()
                << ",\"identifier_storage_bytes\":" << identifiers.storage_bytes()
                << ",\"intern_probes\":" << stats.intern_probes
                << ",\"storage_growths\":" << stats.storage_growths << "}\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
