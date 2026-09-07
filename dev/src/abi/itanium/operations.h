#pragma once
#include "abi/itanium/graph.h"
namespace abi_mangle {
// Compact operation identity; textual mnemonics are classified by the adapter.
Id operation(const std::string& code);
const char* operation_code(Id op);
}
