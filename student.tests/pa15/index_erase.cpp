#include "support/id_index.h"
#include <cstdint>
#include <cstdio>
// Interleaved binding replacement/removal must preserve every unrelated key,
// including probe clusters that wrap and tables that grow between rounds.
int main() {
    cppgm::IdIndex index;
    std::uint32_t expected[513] = {};
    for (unsigned round = 0; round < 8; ++round) {
        for (unsigned key = 1; key <= 512; ++key) {
            expected[key] = key+round+1;
            index.put(key,expected[key]);
        }
        for (unsigned key = 1; key <= 512; ++key) {
            if (key % 3 == round % 3) { expected[key] = 0; index.put(key,0); }
            for (unsigned probe = 1; probe <= 512; ++probe)
                if (index.get(probe) != expected[probe]) {
                    std::fprintf(stderr,"round %u remove %u lost %u\n",round,key,probe); return 1;
                }
        }
    }
    for (unsigned key = 1; key <= 512; ++key) index.put(key,0);
    return !index.empty();
}
