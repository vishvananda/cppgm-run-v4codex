#pragma once
#include <cstdint>
namespace native {
// These forms touch only reserved rax/xmm15 scratch. Selection and encoding
// share the same size policy so incoming argument carriers stay valid.
inline bool direct_copy_bytes(std::uint64_t bytes, unsigned alignment) {
    return bytes <= 32 || (bytes <= 64 && alignment >= 8);
}
}
