#include "toolchain/object.h"
#include "toolchain/elf_model.h"
#include "toolchain/host_config.h"
#include "lowering/procedural.h"
#include "toolchain/preprocess_output.h"
#include "lowir/writer.h"
#include "lowir/validator.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/resource.h>
#include <sys/stat.h>
namespace cppgm { namespace toolchain {
namespace {
bool prefix(const std::string& s, const char* text) { return s.compare(0,std::char_traits<char>::length(text),text) == 0; }
bool object_path(const std::string& s) {
    // Output names are not object-format policy. Explicit object input is
    // recognized from its binary contract, including extensionless outputs.
    char magic[8] = {}; std::ifstream input(s,std::ios::binary); input.read(magic,sizeof(magic));
    if (!std::memcmp(magic,"\177ELF",4) || !std::memcmp(magic,"CPPGMOBJ",8)) return true;
    // Retain object input intent for truncated/corrupt named objects so the
    // bounded binary reader diagnoses them instead of parsing binary as C++.
    auto dot = s.rfind('.');
    return dot != std::string::npos && (s.substr(dot) == ".o" || s.substr(dot) == ".obj");
}
std::string argument(const std::vector<std::string>& args, unsigned& i, const char* flag) {
    if (++i == args.size()) throw std::runtime_error(std::string("missing argument after ")+flag);
    return args[i];
}
struct Options {
    bool compile = false, preprocess = false, stats = false, host = false;
    bool standard_includes = true, cxx_includes = true, lowir = false, audit = false;
    std::string output, format;
    std::vector<std::string> inputs, includes, libraries, paths, macros, system_includes;
};
Options options(const std::vector<std::string>& args)
{
    Options o;
    for (unsigned i = 0; i < args.size(); ++i) {
        const auto& a = args[i];
        if (a == "-c") o.compile = true;
        else if (a == "-E") o.preprocess = true;
        else if (a == "--emit-lowir") o.lowir = true;
        else if (a == "--validate-lowir") o.audit = true;
        else if (a == "-nostdinc") o.standard_includes = false;
        else if (a == "-nostdinc++") o.cxx_includes = false;
        else if (prefix(a,"--object-format=")) {
            o.format = a.substr(16);
            if (o.format != "elf" && o.format != "private") throw std::runtime_error("unsupported object format");
        }
        else if (a == "--stats") o.stats = true;
        else if (a == "-o") o.output = argument(args,i,"-o");
        else if (a == "--target" || prefix(a,"--target=")) {
            auto target = a == "--target" ? argument(args,i,"--target") : a.substr(9);
            if (target != "linux" && target != "x86_64-unknown-linux-gnu" && target != "x86_64-linux-gnu")
                throw std::runtime_error("unsupported target: " + target);
        } else if (prefix(a,"-D") || prefix(a,"-U")) {
            o.macros.push_back(a.size() == 2 ? a + argument(args,i,a.c_str()) : a);
        } else if (prefix(a,"-isystem")) {
            o.system_includes.push_back(a.size() == 8 ? argument(args,i,"-isystem") : a.substr(8));
        } else if (a == "-include") {
            o.macros.push_back("-include"); o.macros.push_back(argument(args,i,"-include"));
        } else if (a == "-stdlib" || prefix(a,"-stdlib=")) {
            check_stdlib(a == "-stdlib" ? argument(args,i,"-stdlib") : a.substr(8));
        } else if (a == "-std" || prefix(a,"-std=")) {
            auto standard = a == "-std" ? argument(args,i,"-std") : a.substr(5);
            auto year = standard == "c++11" || standard == "gnu++11" ? "201103L" :
                standard == "c++14" || standard == "gnu++14" ? "201402L" :
                standard == "c++17" || standard == "gnu++17" ? "201703L" : nullptr;
            if (!year) throw std::runtime_error("unsupported language standard");
            o.macros.push_back(std::string("-D__cplusplus=")+year);
            o.macros.push_back(standard.compare(0,3,"gnu") == 0 ? "-U__STRICT_ANSI__" : "-D__STRICT_ANSI__=1");
        } else if (a == "-pthread") o.macros.push_back("-D_REENTRANT=1");
        else if (a == "-fno-exceptions" || a == "-fexceptions") {
            o.macros.push_back(a);
            o.macros.push_back(a == "-fexceptions" ? "-D__EXCEPTIONS=1" : "-U__EXCEPTIONS");
            o.macros.push_back(a == "-fexceptions" ? "-D__cpp_exceptions=199711L" : "-U__cpp_exceptions");
        }
        else if (a == "-MMD" || a == "-MD" || a == "-MP") continue;
        else if (prefix(a,"-MF") || prefix(a,"-MT") || prefix(a,"-MQ")) {
            if (a.size() == 3) argument(args,i,a.c_str());
        } else if (prefix(a,"-I") || prefix(a,"-L") || prefix(a,"-l")) {
            auto value = a.size() == 2 ? argument(args,i,a.c_str()) : a.substr(2);
            if (a[1] == 'I') o.includes.push_back(value);
            else if (a[1] == 'L') o.paths.push_back(value);
            else o.libraries.push_back(value);
        } else if (a == "-g0" || a == "-O0" || a == "-O1" || a == "-O2" || a == "-O3" || a == "-Wall" || prefix(a,"-W") || a == "-w" ||
            a == "-fvisibility=hidden" || a == "-fvisibility-inlines-hidden" || a == "-pedantic" || a == "-pedantic-errors" || a == "-std=c++11" || a == "-std=gnu++11" || a == "-pipe") continue;
        else if (!a.empty() && a[0] == '-') throw std::runtime_error("unsupported driver option: " + a);
        else o.inputs.push_back(a);
    }
    if (o.lowir && !o.compile) throw std::runtime_error("hosted LowIR inspection requires -c");
    if (o.inputs.empty() || ((o.compile || o.preprocess) && !o.output.empty() && o.inputs.size() != 1)) throw std::runtime_error("invalid compile/link inputs");
    if (o.output.empty() && !o.preprocess && !o.compile) o.output = "a.out";
    o.includes.insert(o.includes.end(),o.system_includes.begin(),o.system_includes.end());
    o.host = o.format != "private";
    if (o.host) host_environment(o.includes,o.macros,o.standard_includes,o.cxx_includes);
    return o;
}
void build_source(lowir_model::Program& program, const std::string& path, const Options& o) {
    lowering::build_program(program,{path},o.stats,o.includes,o.macros,false,o.host);
}
Object source(const std::string& path, const Options& o, native::Statistics& stats) {
    lowir_model::Program program; build_source(program,path,o);
    return compile_object(program,stats,o.host);
}
}
int run(const std::vector<std::string>& args)
{
    auto start = std::chrono::steady_clock::now(); auto o = options(args);
    if (o.preprocess) return preprocess_output(o.inputs,o.output,o.includes,o.macros,o.stats);
    native::Statistics stats, runtime_stats; std::size_t text = 0, link_definitions = 0, link_relocations = 0;
    if (o.compile) {
        for (const auto& input : o.inputs) {
            if (object_path(input)) throw std::runtime_error("object input in compile mode");
            auto output = o.output;
            if (output.empty()) {
                auto slash = input.rfind('/'); auto name = input.substr(slash == std::string::npos ? 0 : slash+1);
                output = name.substr(0,name.rfind('.')) + (o.lowir ? ".lowir" : ".o");
            }
            if (o.lowir) {
                lowir_model::Program program; build_source(program,input,o);
                if (o.audit) lowir_model::validate(program);
                std::ofstream out(output);
                if (!out) throw std::runtime_error("cannot create LowIR output");
                lowir_model::write_program(program,out); out.close();
                if (!out) throw std::runtime_error("cannot write LowIR output");
                continue;
            }
            auto obj = source(input,o,stats); text += obj.image.code.size();
            if (o.host) write_host_object(std::move(obj),output); else write_object(obj,output);
        }
    }
    else {
        Linker linker(o.host);
        for (const auto& input : o.inputs) {
            if (object_path(input)) linker.add(read_object(input));
            else { auto obj = source(input,o,stats); linker.add(o.host ? host_link_object(std::move(obj)) : std::move(obj)); }
        }
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
        link_definitions = linker.definition_work; link_relocations = linker.relocation_work;
        runtime_stats = linker.runtime_stats;
    }
    if (o.stats) {
        struct rusage usage; getrusage(RUSAGE_SELF,&usage);
        std::cerr << "{\"driver_ms\":" << std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()
            << ",\"peak_rss_kib\":" << usage.ru_maxrss << ",\"text_bytes\":" << text
            << ",\"native_functions\":" << stats.functions << ",\"native_instructions\":" << stats.instructions
            << ",\"preparation_ms\":" << stats.preparation_ms
            << ",\"prepared_instructions\":" << stats.prepared_instructions << ",\"prepared_operands\":" << stats.prepared_operands
            << ",\"inline_calls\":" << stats.inline_calls << ",\"inline_work\":" << stats.inline_work
            << ",\"inline_declined\":" << stats.inline_declined << ",\"inline_budget_work\":" << stats.inline_budget_work
            << ",\"inline_max_function_work\":" << stats.inline_max_function_work
            << ",\"link_definition_work\":" << link_definitions << ",\"link_relocation_work\":" << link_relocations
            << ",\"runtime_functions\":" << runtime_stats.functions << ",\"runtime_instructions\":" << runtime_stats.instructions
            << ",\"runtime_text_bytes\":" << runtime_stats.text_bytes
            << ",\"selection_ms\":" << stats.selection_ms << ",\"encoding_ms\":" << stats.encoding_ms << "}\n";
    }
    return 0;
}
} }
