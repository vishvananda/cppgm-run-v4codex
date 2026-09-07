#include "syntax/driver.h"
#include "syntax/parser.h"
#include "semantic/analyzer.h"
#include "preprocess/preprocessor.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/resource.h>

namespace cppgm { namespace syntax {

int emit_ast(const std::string& output, const std::vector<std::string>& inputs, bool stats, bool types)
{
    const std::time_t now = std::time(0);
    const std::string stamp = std::asctime(std::localtime(&now));
    const std::string date = stamp.substr(4, 7) + stamp.substr(20, 4);
    const std::string time = stamp.substr(11, 8);
    std::ofstream out(output.c_str(), std::ios::binary);
    if (!out) throw std::runtime_error("cannot create AST output");
    out << inputs.size() << " translation units\n";
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        typedef std::chrono::steady_clock Clock;
        Clock::time_point start = Clock::now();
        Preprocessor pp(inputs[i], date, time, stats);
        PostTokenCursor post(pp, pp.identifiers(), true);
        Ast ast(stats);
        Cursor cursor(post, pp.identifiers(), ast);
        Parser parser(cursor, ast, pp.identifiers());
        semantic::Analyzer semantics(ast, pp.identifiers());
        NodeId root = parser.translation_unit(types ? &semantics : 0);
        if (types) semantics.finish();
        Clock::time_point parsed = Clock::now();
        out << "start translation unit " << i + 1 << '\n';
        if (types) semantics.write(out);
        else write_ast(out, ast, root, pp.identifiers());
        out << "end translation unit\n";
        out.flush();
        if (!out) throw std::runtime_error("cannot write AST output");
        if (stats) {
            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            std::cerr << "{\"frontend_ms\":" << std::chrono::duration<double, std::milli>(parsed - start).count()
                << ",\"emit_ms\":" << std::chrono::duration<double, std::milli>(Clock::now() - parsed).count()
                << ",\"peak_rss_kib\":" << usage.ru_maxrss
                << ",\"source_bytes\":" << pp.stats().source_bytes
                << ",\"tokens\":" << cursor.produced << ",\"max_pending\":" << cursor.max_pending
                << ",\"nodes\":" << ast.nodes.size() - 1
                << ",\"node_capacity\":" << ast.nodes.capacity()
                << ",\"node_growths\":" << ast.node_growths
                << ",\"location_growths\":" << ast.location_growths
                << ",\"literal_growths\":" << ast.literal_growths
                << ",\"node_bytes\":" << sizeof(Node)
                << ",\"locations\":" << ast.locations.size() - 1
                << ",\"location_capacity\":" << ast.locations.capacity()
                << ",\"delimiter_work\":" << cursor.delimiter_work
                << ",\"angle_work\":" << parser.angle_work << ",\"angle_hits\":" << parser.angle_hits
                << ",\"hint_bytes\":" << parser.hint_bytes
                << ",\"scopes\":" << parser.name_categories().scope_count()
                << ",\"name_probes\":" << parser.name_categories().probes
                << ",\"lookup_scopes\":" << parser.name_categories().lookup_scopes
                << ",\"syntax_decisions\":" << parser.decisions;
            if (types) semantics.telemetry(std::cerr);
            std::cerr << "}\n";
        }
    }
    return 0;
}

} }
