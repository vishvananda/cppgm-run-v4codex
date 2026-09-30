#pragma once
#include <string>
#include <vector>
namespace cppgm { namespace toolchain {
void host_environment(std::vector<std::string>& includes, std::vector<std::string>& macros, bool standard_includes = true, bool cxx_includes = true);
std::vector<std::string> host_library_paths();
void check_stdlib(const std::string&);
int query(const std::string&);
} }
