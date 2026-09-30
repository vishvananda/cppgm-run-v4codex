#include "toolchain/host_config.h"
#include "builtin_host_config.h"
#include <sstream>
#include <iostream>
#include <stdexcept>
namespace cppgm { namespace toolchain {
std::vector<std::string> host_library_paths()
{
    std::vector<std::string> result;
    std::istringstream input(cppgm_builtin_host_config::kSearchDirs); std::string line;
    while (std::getline(input,line)) if (line.compare(0,12,"libraries: =") == 0) {
        std::istringstream paths(line.substr(12));
        while (std::getline(paths,line,':')) if (!line.empty()) result.push_back(line);
    }
    return result;
}
void host_environment(std::vector<std::string>& includes, std::vector<std::string>& macros, bool standard_includes, bool cxx_includes)
{
    for (auto path : cppgm_builtin_host_config::kStandardIncludePaths)
        if (path && standard_includes && (cxx_includes || std::string(path).find("c++") == std::string::npos)) includes.push_back(path);
    std::vector<std::string> defaults;
    std::istringstream input(cppgm_builtin_host_config::kHostPredefinedMacros);
    std::string line;
    while (std::getline(input,line)) {
        if (line.compare(0,8,"#define ")) continue;
        auto space = line.find(' ',8);
        auto name = line.substr(8,space-8);
        // Language/feature promises describe this compiler, not its build host.
        if (name == "__cplusplus" || name.compare(0,6,"__cpp_") == 0) continue;
        if (name.compare(0,8,"__BFLT16") == 0 || name.compare(0,7,"__FLT16") == 0 ||
            name.compare(0,7,"__FLT32") == 0 || name.compare(0,7,"__FLT64") == 0 ||
            name.compare(0,8,"__FLT128") == 0 || name.compare(0,5,"__DEC") == 0 ||
            name == "__SIZEOF_FLOAT128__" || name == "__SIZEOF_FLOAT80__") continue;
        defaults.push_back("-D"+name+"="+(space == std::string::npos ? "" : line.substr(space+1)));
    }

    defaults.push_back("-D__GNUC__=4");
    defaults.push_back("-D__GNUC_MINOR__=2");
    defaults.push_back("-D__GNUC_PATCHLEVEL__=1");
    defaults.push_back("-D__GNUG__=4");
    const char* features[] = {"__cpp_ref_qualifiers=200710L", "__cpp_rvalue_references=200610L",
        "__cpp_static_assert=200410L", "__cpp_variadic_templates=200704L", "__cpp_alias_templates=200704L",
        "__cpp_constexpr=200704L", "__cpp_decltype=200707L", "__cpp_lambdas=200907L",
        "__cpp_unicode_literals=200710L", "__cpp_raw_strings=200710L", "__cpp_user_defined_literals=200809L"};
    for (auto feature : features) defaults.push_back(std::string("-D")+feature);
    const char* aliases[] = {"__extension__=", "__restrict=", "__restrict__=",
        "__decltype=decltype", "__inline=inline", "__inline__=inline", "__const=const", "__const__=const",
        "__volatile=volatile", "__volatile__=volatile", "__signed=signed", "__signed__=signed"};
    for (auto alias : aliases) defaults.push_back(std::string("-D")+alias);
    defaults.insert(defaults.end(),macros.begin(),macros.end());
    macros.swap(defaults);
}
void check_stdlib(const std::string& library)
{
    std::string flags = cppgm_builtin_host_config::kStdlibFlags;
    auto selected = flags.find("-stdlib=libc++") != std::string::npos ? "libc++" : "libstdc++";
    if (library != selected) throw std::runtime_error("stdlib differs from the build-time selection");
}
int query(const std::string& flag)
{
    if (flag == "-dumpmachine") std::cout << cppgm_builtin_host_config::kTarget;
    else if (flag == "-dumpversion") std::cout << cppgm_builtin_host_config::kVersion;
    else if (flag == "-print-search-dirs") std::cout << cppgm_builtin_host_config::kSearchDirs;
    else {
        auto& out = flag == "-v" ? std::cerr : std::cout;
        out << "cppgm++ C++11 compiler\nTarget: " << cppgm_builtin_host_config::kTarget;
    }
    return 0;
}
} }
