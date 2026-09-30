// PA24 adapter: the typed LowIR boundary is shared with source lowering.
#include "lowir/model.h"
#include "native/encoding.h"
#include "support/tool_help_text.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sys/resource.h>
namespace {
struct Invocation {
    std::vector<std::string> inputs;
    std::string output, dump;
    bool stats = false;
};
Invocation invocation(int argc, char** argv)
{
    Invocation i;
    for (int k = 1; k < argc; ++k) {
        std::string a = argv[k];
        if (a == "--stats") i.stats = true;
        else if (a == "-O0") {}
        else if (a == "-o" || a == "--dump-machine-ir" || a == "--dump-native-plan" || a == "--target") {
            lowir_model::require(++k < argc,"missing option value");
            std::string value = argv[k];
            if (a == "--target") lowir_model::require(value == "linux","unsupported target");
            else {
                auto& target = a == "-o" ? i.output : i.dump;
                lowir_model::require(target.empty(),"repeated option"); target = value;
            }
        } else if (!a.empty() && a[0] == '-') throw lowir_model::ParseError("unknown native option: "+a);
        else i.inputs.push_back(a);
    }
    lowir_model::require(!i.inputs.empty() && (!i.output.empty() || !i.dump.empty()),"expected inputs and native/MIR output");
    return i;
}
}
int main(int argc, char** argv)
{
    try {
        for (int n = 1; n < argc; ++n) if (std::string(argv[n]) == "--help" || std::string(argv[n]) == "-h") {
            std::cout << lowir2native_help_text(); return 0;
        }
        const auto i = invocation(argc,argv);
        auto begin = std::chrono::steady_clock::now();
        lowir_model::Program p = lowir_model::parse_lowir_program_files(i.inputs);
        auto parsed = std::chrono::steady_clock::now();
        std::ofstream mir;
        if (!i.dump.empty()) { mir.open(i.dump); lowir_model::require(bool(mir),"cannot create MIR dump"); }
        native::Statistics stats;
        native::compile(p,i.output,i.dump.empty() ? nullptr : &mir,stats);
        if (!i.dump.empty()) { mir.close(); lowir_model::require(bool(mir),"cannot write MIR dump"); }
        if (i.stats) {
            rusage usage; getrusage(RUSAGE_SELF,&usage);
            std::cerr << "{\"read_validate_ms\":" << std::chrono::duration<double,std::milli>(parsed-begin).count()
                << ",\"selection_ms\":" << stats.selection_ms << ",\"encoding_ms\":" << stats.encoding_ms
                << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"functions\":" << stats.functions
                << ",\"instructions\":" << stats.instructions << ",\"frame_bytes\":" << stats.frame_bytes
                << ",\"text_bytes\":" << stats.text_bytes << ",\"value_visits\":" << stats.value_visits
                << ",\"scratch_carried_reloads\":" << stats.scratch_carried_reloads << "}\n";
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << "ERROR: " << e.what() << '\n'; return 1; }
}
