#pragma once
#include <string>
#include <vector>
namespace cppgm { namespace syntax {
int emit_ast(const std::string& output, const std::vector<std::string>& inputs, bool stats = false, bool types = false, bool calls = false);
} }
