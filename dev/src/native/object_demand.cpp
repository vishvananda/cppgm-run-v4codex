#include "lowir/emission_demand.h"
namespace native {
// Source ABI retention and native selection use the same typed demand graph.
std::vector<bool> object_demand(const lowir_model::Program& p)
{
    return lowir_model::emission_demand(p);
}
} // namespace native
