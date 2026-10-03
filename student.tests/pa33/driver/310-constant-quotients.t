// Optimization-boundary coverage, not a pre-existing correctness regression.
// Compare constant quotients with independent runtime hardware-divisor paths.
template<unsigned long D> __attribute__((noinline)) unsigned long uq(unsigned long n) { return n / D; }
template<long D> __attribute__((noinline)) long sq(long n) { return n / D; }
__attribute__((noinline)) unsigned long ur(unsigned long n, unsigned long d) { return n / d; }
__attribute__((noinline)) long sr(long n, long d) { return n / d; }
volatile unsigned long unsigned_divisor;
volatile long signed_divisor;
unsigned long next(unsigned long& state) { state ^= state << 13; state ^= state >> 7; state ^= state << 17; return state; }
template<unsigned long D> bool check_unsigned() {
    unsigned_divisor = D;
    unsigned long values[] = {0,1,D-1,D,D+1,~0ul,~0ul-1,1ul<<63,(1ul<<63)-1};
    for (unsigned i=0; i<sizeof(values)/sizeof(values[0]); ++i)
        if (uq<D>(values[i]) != ur(values[i],unsigned_divisor)) return false;
    unsigned long state=17;
    for(unsigned i=0;i<4096;++i) {
        unsigned long n=next(state);
        if(uq<D>(n)!=ur(n,unsigned_divisor)) return false;
    }
    return true;
}
template<long D> bool check_signed() {
    signed_divisor = D;
    long values[] = {0,1,-1,23,24,25,-23,-24,-25,static_cast<long>(1ul<<63),static_cast<long>((1ul<<63)-1)};
    for (unsigned i=0;i<sizeof(values)/sizeof(values[0]);++i) {
        long n=values[i];
        if(D==-1 && n==static_cast<long>(1ul<<63)) continue;
        if(sq<D>(n)!=sr(n,signed_divisor)) return false;
    }
    unsigned long state=53;
    for(unsigned i=0;i<4096;++i) {
        long n=static_cast<long>(next(state));
        if(D==-1 && n==static_cast<long>(1ul<<63)) continue;
        if(sq<D>(n)!=sr(n,signed_divisor)) return false;
    }
    return true;
}
int main() {
#define U(d) if(!check_unsigned<d>()) return 1
#define S(d) if(!check_signed<d>()) return 2
    U(1); U(2); U(3); U(5); U(7); U(10); U(24); U(40); U(63); U(64); U(65);
    U(127); U(255); U(257); U(65535); U(65537); U(4294967295ul); U(4294967297ul);
    U((1ul<<63)-1); U(1ul<<63); U((1ul<<63)+1); U(~0ul);
    S(1); S(-1); S(2); S(-2); S(3); S(-3); S(7); S(-7); S(24); S(-24);
    S(40); S(-40); S(64); S(-64); S(127); S(-127); S(65537); S(-65537);
    S(4294967297l); S(-4294967297l); S(9223372036854775807l); S(-9223372036854775807l-1);
    return 0;
}
