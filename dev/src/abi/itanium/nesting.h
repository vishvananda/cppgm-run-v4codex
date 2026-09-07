#pragma once
#include <stdexcept>

namespace abi_mangle {
// Shared across every recursive grammar edge, including isolated external
// substitution tables. Linear modifier chains do not consume stack depth.
struct Nesting {
    unsigned& depth;
    explicit Nesting(unsigned& value) : depth(value) {
        if (depth >= 1024) throw std::runtime_error("ABI nesting limit exceeded");
        ++depth;
    }
    ~Nesting() { --depth; }
};
} // namespace abi_mangle
