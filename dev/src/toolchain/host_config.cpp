#include "toolchain/host_config.h"
#include "builtin_host_config.h"
#include <sstream>
namespace cppgm { namespace toolchain {
void host_environment(std::vector<std::string>& includes, std::vector<std::string>& macros)
{
    for (auto path : cppgm_builtin_host_config::kStandardIncludePaths)
        if (path) includes.push_back(path);
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
    defaults.push_back("-D__has_builtin(x)=0");
    defaults.push_back("-D__GNUC__=4");
    defaults.push_back("-D__GNUC_MINOR__=2");
    defaults.push_back("-D__GNUC_PATCHLEVEL__=1");
    defaults.push_back("-D__GNUG__=4");
    const char* aliases[] = {"__extension__=", "__restrict=", "__restrict__=",
        "__decltype=decltype", "__inline=inline", "__inline__=inline", "__const=const", "__const__=const",
        "__volatile=volatile", "__volatile__=volatile", "__signed=signed", "__signed__=signed"};
    for (auto alias : aliases) defaults.push_back(std::string("-D")+alias);
    defaults.insert(defaults.end(),macros.begin(),macros.end());
    macros.swap(defaults);
}
} }
