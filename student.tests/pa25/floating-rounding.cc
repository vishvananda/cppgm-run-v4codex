// Observation only: demonstrate the PA25 oracle's permitted excess precision.
#include <cstdio>
#include <cstdint>
#include <cstring>
int main() {
    std::uint64_t pairs[][2] = {
        {0x000e4a48b30e4a48ULL,0xf46e4bb3ac7e4bb3ULL},
        {0x0009c91ce589c91cULL,0x2ecadde1ca7adde1ULL},
        {0x52d5ffc3f755ffc3ULL,0x1730d5868da0d586ULL},
        {0xc41cd1c7a7ccd1c7ULL,0xd4e76c8e89876c8eULL},
        {0x373d021dbaad021dULL,0xfaed8feca25d8fecULL}
    };
    for (unsigned i=0;i<5;++i) {
        double a,b;std::memcpy(&a,&pairs[i][0],8);std::memcpy(&b,&pairs[i][1],8);
        volatile double x=a,y=b;
        volatile long double extended=i==1 ? (long double)x/y : (long double)x*y;
        double direct=i==1 ? x/y : x*y, two_roundings=extended;
        std::uint64_t single_bits,extended_bits;
        std::memcpy(&single_bits,&direct,8);std::memcpy(&extended_bits,&two_roundings,8);
        std::printf("%016llx %016llx\n",(unsigned long long)single_bits,(unsigned long long)extended_bits);
    }
}
