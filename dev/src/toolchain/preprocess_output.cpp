#include "toolchain/preprocess_output.h"
#include "preprocess/preprocessor.h"
#include "posttoken/cursor.h"
#include "posttoken/output.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sys/resource.h>
namespace cppgm { namespace toolchain {
int preprocess_output(const std::vector<std::string>& inputs, const std::string& output,
    const std::vector<std::string>& includes, const std::vector<std::string>& macros, bool stats)
{
    const auto now = std::time(nullptr);
    const std::string stamp = std::asctime(std::localtime(&now));
    std::ofstream file;
    if (!output.empty()) {
        file.open(output,std::ios::binary);
        if (!file) throw std::runtime_error("cannot create preprocessing output");
    }
    auto& out = output.empty() ? std::cout : file;
    out << "preproc " << inputs.size() << '\n';
    for (const auto& input : inputs) {
        auto start = std::chrono::steady_clock::now();
        Preprocessor pp(input,stamp.substr(4,7)+stamp.substr(20,4),stamp.substr(11,8),stats,true);
        pp.include_paths(includes); pp.command_options(macros);
        PostTokenCursor post(pp,pp.identifiers(),true,nullptr,true,true);
        out << "sof " << input << '\n';
        for (;;) {
            auto token = post.next();
            if (token.kind == PostTokenKind::invalid) throw std::runtime_error("invalid phase-7 token");
            write_post_token(out,token,pp.identifiers());
            if (token.kind == PostTokenKind::eof) break;
        }
        if (stats) {
            struct rusage usage; getrusage(RUSAGE_SELF,&usage);
            const auto& s = pp.stats();
            std::cerr << "{\"preprocess_post_emit_ms\":"
                << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
                << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"source_bytes\":" << s.source_bytes
                << ",\"files\":" << s.files << ",\"directives\":" << s.directives
                << ",\"invocations\":" << s.invocations << ",\"output_tokens\":" << s.output_tokens
                << ",\"arena_bytes\":" << s.arena_bytes << ",\"max_pending\":" << s.max_pending << "}\n";
        }
    }
    out.flush();
    if (!out) throw std::runtime_error("cannot write preprocessing output");
    return 0;
}
} }
