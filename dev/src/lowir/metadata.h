#pragma once
#include "lowir/model.h"
namespace lowir_model {
const char* role_name(SymbolRole role);
void validate_signature(const Program& p, const Signature& signature, bool indirect);
}
