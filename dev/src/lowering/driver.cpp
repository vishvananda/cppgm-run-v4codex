#include "lowering/procedural.h"
#include "preprocess/preprocessor.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sys/resource.h>
namespace cppgm { namespace lowering {
int emit_lowir(const std::string& output, const std::vector<std::string>& inputs, bool stats, bool audit)
{
    typedef std::chrono::steady_clock Clock;
    const std::time_t now = std::time(0);
    const std::string stamp = std::asctime(std::localtime(&now));
    lowir_model::Program program;
    Linkage linkage(inputs.size() > 1);
    double frontend_ms = 0, lowering_ms = 0;
    std::size_t nodes = 0, static_requests = 0, static_hits = 0;
    std::size_t control_work = 0, discard_work = 0;
    for (const std::string& input : inputs) {
        auto start = Clock::now();
        Preprocessor pp(input, stamp.substr(4, 7) + stamp.substr(20, 4), stamp.substr(11, 8), stats);
        PostTokenCursor post(pp, pp.identifiers(), true);
        syntax::Ast ast(stats);
        syntax::Cursor cursor(post, pp.identifiers(), ast);
        syntax::Parser parser(cursor, ast, pp.identifiers());
        semantic::Analyzer sem(ast, pp.identifiers(), true);
        parser.translation_unit(&sem); sem.finish();
        auto parsed = Clock::now();
        Procedural lower(ast, sem, pp.identifiers(), program, linkage); lower.run();
        frontend_ms += std::chrono::duration<double, std::milli>(parsed-start).count();
        lowering_ms += std::chrono::duration<double, std::milli>(Clock::now()-parsed).count();
        nodes += ast.nodes.size();
        static_requests += sem.static_requests; static_hits += sem.static_hits;
        control_work += lower.control_work; discard_work += lower.discard_work;
        if (stats) {
            std::cerr << "{\"tokens\":" << cursor.produced << ",\"max_pending\":" << cursor.max_pending
                << ",\"node_growths\":" << ast.node_growths << ",\"delimiter_work\":" << cursor.delimiter_work;
            sem.telemetry(std::cerr);
            std::cerr << "}\n";
        }
    }
    if (audit) lowir_model::validate(program);
    std::ofstream out(output.c_str());
    if (!out) throw std::runtime_error("cannot create LowIR output");
    auto start = Clock::now();
    lowir_model::write_program(program, out); out.flush();
    if (!out) throw std::runtime_error("cannot write LowIR output");
    if (stats) {
        struct rusage usage; getrusage(RUSAGE_SELF, &usage);
        std::cerr << "{\"frontend_ms\":" << frontend_ms << ",\"lowering_ms\":" << lowering_ms
            << ",\"write_ms\":" << std::chrono::duration<double, std::milli>(Clock::now()-start).count()
            << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"nodes\":" << nodes
            << ",\"static_requests\":" << static_requests << ",\"static_hits\":" << static_hits
            << ",\"control_work\":" << control_work << ",\"discard_work\":" << discard_work
            << ",\"linkage_requests\":" << linkage.requests << ",\"linkage_hits\":" << linkage.hits
            << ",\"abi_nodes\":" << linkage.abi.size() << ",\"abi_bytes\":" << linkage.abi.storage_bytes()
            << ",\"instructions\":" << program.instructions.size() << ",\"operands\":" << program.operands.size()
            << ",\"ir_pool_growths\":" << program.pool_allocations()
            << ",\"ir_capacity_bytes\":" << program.pool_storage_bytes() << "}\n";
    }
    return 0;
}
} }
