#pragma once
#include "lowir/model.h"
namespace lowir_model {
struct EmissionDependency { SymbolId owner, target; };
std::vector<bool> emission_demand(const Program&, const std::vector<EmissionDependency>& = {});
}
