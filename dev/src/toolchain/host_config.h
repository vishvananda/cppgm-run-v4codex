#pragma once
#include <string>
#include <vector>
namespace cppgm { namespace toolchain {
void host_environment(std::vector<std::string>& includes, std::vector<std::string>& macros);
std::vector<std::string> host_library_paths();
} }
