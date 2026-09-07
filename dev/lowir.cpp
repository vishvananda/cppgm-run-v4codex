// PA8 command-line adapter; shared typed IR lives under src/lowir/.
#include "lowir/model.h"
#include "support/tool_help_text.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/resource.h>

namespace {
struct Invocation {
    std::string output, exercise;
    std::vector<std::string> inputs;
    bool stats = false;
};
Invocation invocation(int argc, char** argv)
{
    Invocation i;
    for (int n = 1; n < argc; ++n) {
        std::string a = argv[n];
        if (a == "-o" || a == "--exercise") {
            lowir_model::require(++n < argc, "missing option value");
            std::string& value = a == "-o" ? i.output : i.exercise;
            lowir_model::require(value.empty(), "repeated option");
            value = argv[n];
        } else if (a == "--stats") i.stats = true;
        else if (!a.empty() && a[0] == '-') throw lowir_model::ParseError("unknown option " + a);
        else i.inputs.push_back(a);
    }
    lowir_model::require(!i.output.empty() && (i.inputs.empty() != i.exercise.empty()), "expected -o and inputs or one exercise");
    return i;
}
using Clock = std::chrono::steady_clock;
double milliseconds(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b-a).count(); }
}
int main(int argc, char** argv)
{
    try {
        for (int n = 1; n < argc; ++n) if (std::string(argv[n]) == "--help" || std::string(argv[n]) == "-h") {
            std::cout << lowir_help_text() << "  --stats  report phase times, peak RSS and IR work counts to stderr\n";
            return 0;
        }
        Invocation i = invocation(argc, argv);
        auto start = Clock::now();
        lowir_model::Program p;
        if (!i.exercise.empty()) p = lowir_model::construct_exercise(i.exercise);
        else for (const auto& path : i.inputs) {
            std::ifstream in(path, std::ios::binary);
            lowir_model::require(bool(in), "cannot open input");
            std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            lowir_model::read_program(p, source, path);
        }
        auto read = Clock::now();
        if (i.exercise.empty()) lowir_model::validate(p);
        auto validated = Clock::now();
        std::ofstream out(i.output, std::ios::binary);
        lowir_model::require(bool(out), "cannot open output");
        lowir_model::write_program(p, out);
        out.close();
        lowir_model::require(bool(out), "cannot write output");
        auto written = Clock::now();
        if (i.stats) {
            rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            std::cerr << "{\"read_ms\":" << milliseconds(start, read) << ",\"validate_ms\":" << milliseconds(read, validated)
                << ",\"write_ms\":" << milliseconds(validated, written) << ",\"peak_rss_kib\":" << usage.ru_maxrss
                << ",\"source_bytes\":" << p.stats.source_bytes << ",\"tokens\":" << p.stats.tokens
                << ",\"symbols\":" << p.symbols.size() << ",\"values\":" << p.values.size()
                << ",\"instructions\":" << p.instructions.size() << ",\"operands\":" << p.operands.size()
                << ",\"validated_instructions\":" << p.stats.validated_instructions << ",\"cfg_edges\":" << p.stats.cfg_edges
                << ",\"interned_names\":" << p.names.size() << ",\"name_storage_bytes\":" << p.names.storage_bytes()
                << ",\"ir_pool_allocations\":" << p.pool_allocations() << ",\"ir_pool_capacity_bytes\":" << p.pool_storage_bytes() << "}\n";
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << "ERROR: " << e.what() << '\n'; return 1; }
}
