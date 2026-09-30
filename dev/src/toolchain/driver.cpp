#include "toolchain/object.h"
#include "lowering/procedural.h"
#include <chrono>
#include <iostream>
#include <sys/resource.h>
#include <sys/stat.h>
namespace cppgm { namespace toolchain {
namespace {
bool prefix(const std::string& s, const char* text) { return s.compare(0,std::char_traits<char>::length(text),text) == 0; }
bool object_path(const std::string& s) {
    auto dot = s.rfind('.'); if (dot == std::string::npos) return false;
    auto ext = s.substr(dot); return ext == ".o" || ext == ".obj";
}
std::string argument(const std::vector<std::string>& args, unsigned& i, const char* flag) {
    if (++i == args.size()) throw std::runtime_error(std::string("missing argument after ")+flag);
    return args[i];
}
struct Options {
    bool compile = false, stats = false;
    std::string output;
    std::vector<std::string> inputs, includes, libraries, paths, macros;
};
Options options(const std::vector<std::string>& args)
{
    Options o;
    for (unsigned i = 0; i < args.size(); ++i) {
        const auto& a = args[i];
        if (a == "-c") o.compile = true;
        else if (a == "--stats") o.stats = true;
        else if (a == "-o") o.output = argument(args,i,"-o");
        else if (a == "--target" || prefix(a,"--target=")) {
            auto target = a == "--target" ? argument(args,i,"--target") : a.substr(9);
            if (target != "linux" && target != "x86_64-unknown-linux-gnu" && target != "x86_64-linux-gnu")
                throw std::runtime_error("unsupported target: " + target);
        } else if (prefix(a,"-D") || prefix(a,"-U")) {
            o.macros.push_back(a.size() == 2 ? a + argument(args,i,a.c_str()) : a);
        } else if (prefix(a,"-I") || prefix(a,"-L") || prefix(a,"-l")) {
            auto value = a.size() == 2 ? argument(args,i,a.c_str()) : a.substr(2);
            if (a[1] == 'I') o.includes.push_back(value);
            else if (a[1] == 'L') o.paths.push_back(value);
            else o.libraries.push_back(value);
        } else if (a == "-O0" || a == "-Wall" || prefix(a,"-W") || a == "-w" ||
            a == "-fvisibility=hidden" || a == "-fvisibility-inlines-hidden" || a == "-pedantic" || a == "-pedantic-errors" || a == "-std=c++11" || a == "-std=gnu++11" || a == "-pipe") continue;
        else if (!a.empty() && a[0] == '-') throw std::runtime_error("unsupported driver option: " + a);
        else o.inputs.push_back(a);
    }
    if (o.inputs.empty() || (o.compile && (o.inputs.size() != 1 || object_path(o.inputs[0])))) throw std::runtime_error("invalid compile/link inputs");
    if (o.output.empty()) {
        o.output = "a.out";
        if (o.compile) { auto s = o.inputs[0]; auto slash = s.rfind('/'); s = s.substr(slash == std::string::npos ? 0 : slash+1); o.output = s.substr(0,s.rfind('.')) + ".o"; }
    }
    return o;
}
Object source(const std::string& path, const Options& o, native::Statistics& stats) {
    lowir_model::Program program;
    lowering::build_program(program,{path},o.stats,o.includes,o.macros);
    return compile_object(program,stats);
}
}
int run(const std::vector<std::string>& args)
{
    auto start = std::chrono::steady_clock::now(); auto o = options(args);
    native::Statistics stats; std::size_t text = 0;
    if (o.compile) { auto obj = source(o.inputs[0],o,stats); text = obj.image.code.size(); write_object(obj,o.output); }
    else {
        Linker linker;
        for (const auto& input : o.inputs)
            linker.add(object_path(input) ? read_object(input) : source(input,o,stats));
        for (const auto& library : o.libraries) {
            std::string found;
            for (const auto& path : o.paths) {
                auto file = path + "/lib" + library + ".o"; struct stat info;
                if (!stat(file.c_str(),&info)) { found = file; break; }
            }
            if (found.empty()) throw std::runtime_error("library not found: " + library);
            linker.add(read_object(found));
        }
        text = linker.finish(o.output);
    }
    if (o.stats) {
        struct rusage usage; getrusage(RUSAGE_SELF,&usage);
        std::cerr << "{\"driver_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
            << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"text_bytes\":" << text
            << ",\"native_functions\":" << stats.functions << ",\"native_instructions\":" << stats.instructions
            << ",\"selection_ms\":" << stats.selection_ms << ",\"encoding_ms\":" << stats.encoding_ms << "}\n";
    }
    return 0;
}
} }
