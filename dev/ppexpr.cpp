// (C) 2013 CPPGM Foundation www.cppgm.org. All rights reserved.
// Adapted from the PA3 starter; see NOTICE for attribution.
#include "preprocess/expression.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sys/resource.h>

// PA3's mock uses the first UTF-8 code unit, not the decoded code point.
static bool mock_defined(cppgm::IdentifierId id, void* context)
{
    const cppgm::IdentifierTable& names = *static_cast<cppgm::IdentifierTable*>(context);
    cppgm::TextView text = names.spelling(id);
    return text.size && (static_cast<unsigned char>(text.data[0]) & 1);
}

static void write_result(cppgm::PPExpressionResult result)
{
    if (result.empty) return;
    if (!result.valid) { std::cout << "error\n"; return; }
    cppgm::PPValue value = result.value;
    if (!value.is_unsigned && (value.bits >> 63)) std::cout << '-' << (0 - value.bits);
    else std::cout << value.bits;
    if (value.is_unsigned) std::cout << 'u';
    std::cout << '\n';
}

int main(int argc, char** argv)
{
    try {
        bool report_stats = false;
        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--stats") report_stats = true;
            else throw std::runtime_error("usage: ppexpr [--stats] < source");
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
        cppgm::LexStats lex_stats;
        cppgm::PostStats literal_stats;
        cppgm::PPExpressionStats expr_stats;
        cppgm::IdentifierTable identifiers(report_stats ? &lex_stats : 0);
        cppgm::PPTokenCursor cursor(source, identifiers, report_stats ? &lex_stats : 0);
        cppgm::PPExpressionEvaluator evaluator(identifiers, mock_defined, &identifiers,
            report_stats ? &expr_stats : 0, report_stats ? &literal_stats : 0);
        for (;;) {
            cppgm::PPToken token = cursor.next();
            if (token.kind == cppgm::PPTokenKind::newline || token.kind == cppgm::PPTokenKind::eof)
                write_result(evaluator.finish());
            else evaluator.push(token);
            if (token.kind == cppgm::PPTokenKind::eof) break;
        }
        std::cout << "eof\n";
        std::cout.flush();
        if (!std::cout) throw std::runtime_error("cannot write expressions");
        if (report_stats) {
            Clock::time_point end = Clock::now();
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            std::cerr << "{\"source_bytes\":" << source.bytes.size()
                << ",\"source_storage_bytes\":" << source.bytes.capacity()
                << ",\"read_ms\":" << std::chrono::duration<double, std::milli>(read_end - start).count()
                << ",\"evaluate_emit_ms\":" << std::chrono::duration<double, std::milli>(end - read_end).count()
                << ",\"peak_rss_kib\":" << usage.ru_maxrss
                << ",\"decoded_units\":" << lex_stats.decoded_units
                << ",\"translated_units\":" << lex_stats.translated_units
                << ",\"tokens\":" << lex_stats.tokens
                << ",\"spelling_bytes\":" << lex_stats.spelling_bytes
                << ",\"spelling_storage_bytes\":" << cursor.spelling_storage_bytes()
                << ",\"identifiers\":" << identifiers.size()
                << ",\"identifier_storage_bytes\":" << identifiers.storage_bytes()
                << ",\"intern_probes\":" << lex_stats.intern_probes
                << ",\"rehash_probes\":" << lex_stats.rehash_probes
                << ",\"storage_growths\":" << lex_stats.storage_growths
                << ",\"number_bytes\":" << literal_stats.number_bytes
                << ",\"literal_bytes\":" << literal_stats.literal_bytes
                << ",\"decoded_elements\":" << literal_stats.decoded_elements
                << ",\"expression_tokens\":" << expr_stats.tokens
                << ",\"expression_lines\":" << expr_stats.lines
                << ",\"expression_errors\":" << expr_stats.errors
                << ",\"reductions\":" << expr_stats.reductions
                << ",\"max_values\":" << expr_stats.max_values
                << ",\"max_operators\":" << expr_stats.max_operators
                << ",\"expression_storage_bytes\":" << evaluator.storage_bytes()
                << ",\"expression_storage_growths\":" << expr_stats.storage_growths << "}\n";
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
