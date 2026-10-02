#pragma once
#include "lowir/model.h"
namespace lowir_model {
// Invocation-owned, immutable facts for fixed readonly object representations.
// Only the prefix ending at the first NUL is needed; no byte buffer is copied.
class ConstantObjects {
    const Program& p;
    std::vector<std::uint64_t> lengths;
public:
    ConstantObjects(const Program&, std::uint64_t& work);
    bool fold(const Instruction&, const Operand*, Operand&) const;
};
}
