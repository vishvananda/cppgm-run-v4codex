#pragma once
#include <string>
#include <vector>
namespace cppgm { namespace toolchain {
int preprocess_output(const std::vector<std::string>& inputs, const std::string& output,
    const std::vector<std::string>& includes, const std::vector<std::string>& macros, bool stats);
} }
