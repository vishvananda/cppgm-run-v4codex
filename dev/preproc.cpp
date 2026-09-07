// (C) 2013 CPPGM Foundation www.cppgm.org. All rights reserved.
// Adapted from the PA4 scaffold; see NOTICE for attribution.
#include "preprocess/preprocessor.h"
#include "posttoken/cursor.h"
#include "posttoken/output.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/resource.h>

int main(int argc, char** argv)
{
    try {
        const std::time_t now = std::time(0);
        const std::string stamp = std::asctime(std::localtime(&now));
        const std::string date = stamp.substr(4, 7) + stamp.substr(20, 4);
        const std::string time = stamp.substr(11, 8);
        bool stats = argc > 1 && std::string(argv[1]) == "--stats";
        int first = stats ? 2 : 1;
        if (argc < first + 3 || std::string(argv[first]) != "-o")
            throw std::runtime_error("usage: preproc [--stats] -o output source...");
        std::ofstream out(argv[first + 1], std::ios::binary);
        if (!out) throw std::runtime_error("cannot create output file");
        out << "preproc " << argc - first - 2 << '\n';
        for (int i = first + 2; i < argc; ++i) {
            typedef std::chrono::steady_clock Clock;
            Clock::time_point start = Clock::now();
            cppgm::Preprocessor pp(argv[i], date, time, stats);
            cppgm::PostTokenCursor cursor(pp, pp.identifiers(), true);
            out << "sof " << argv[i] << '\n';
            for (;;) {
                cppgm::PostToken token = cursor.next();
                if (token.kind == cppgm::PostTokenKind::invalid) throw std::runtime_error("invalid phase-7 token");
                cppgm::write_post_token(out, token, pp.identifiers());
                if (token.kind == cppgm::PostTokenKind::eof) break;
            }
            out.flush();
            if (!out) throw std::runtime_error("cannot write output");
            if (stats) {
                struct rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                const cppgm::PreprocessStats& s = pp.stats();
                std::cerr << "{\"preprocess_post_emit_ms\":"
                    << std::chrono::duration<double, std::milli>(Clock::now() - start).count()
                    << ",\"peak_rss_kib\":" << usage.ru_maxrss
                    << ",\"source_bytes\":" << s.source_bytes << ",\"files\":" << s.files
                    << ",\"directives\":" << s.directives << ",\"invocations\":" << s.invocations
                    << ",\"argument_prescans\":" << s.argument_prescans
                    << ",\"replacement_tokens\":" << s.replacement_tokens
                    << ",\"output_tokens\":" << s.output_tokens << ",\"paste_bytes\":" << s.paste_bytes
                    << ",\"arena_bytes\":" << s.arena_bytes << ",\"context_nodes\":" << s.context_nodes
                    << ",\"max_pending\":" << s.max_pending << ",\"lex_tokens\":" << pp.lex_stats().tokens
                    << ",\"captured_tokens\":" << s.captured_tokens
                    << ",\"borrowed_arguments\":" << s.borrowed_arguments
                    << ",\"max_prescan_depth\":" << s.max_prescan_depth
                    << ",\"max_context_nodes\":" << s.max_context_nodes
                    << ",\"scratch_growths\":" << s.scratch_growths
                    << ",\"identifiers\":" << pp.identifiers().size()
                    << ",\"identifier_storage_bytes\":" << pp.identifiers().storage_bytes() << "}\n";
            }
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "ERROR: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
