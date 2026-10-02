// Demand, lifecycle, mixed/stack call ABI, builtin facts and debug provenance.
volatile int destroyed;
__attribute__((noinline)) long step(long a, double x, long b, double y,
    long c, long d, long e, long f, long g, long h) noexcept {
    return a+b+c+d+e+f+g+h+static_cast<long>(x+y);
}
template<class T> struct Packet {
    T value;
    explicit Packet(T n) : value(n) {}
    T read() const { return value; }
    void unused() { typename T::missing invalid; }
    ~Packet() { ++destroyed; }
};
template<class T> __attribute__((noinline)) long demand(T seed, int n) {
    Packet<T> p(seed);
    long result = p.read();
    for (int i=0; i<n; ++i)
        result = step(result,1.25,i,2.75,2,3,4,5,6,7);
    return result;
}
__attribute__((noinline)) unsigned long length(const char* s) {
    return __builtin_strlen(s);
}
__attribute__((noinline)) void copy(char* d, const char* s, unsigned long n) {
    __builtin_memcpy(d,s,n);
}
int main(int argc, char**) {
    long a = demand(static_cast<long>(argc),argc);
    long b = demand(argc,argc+1);
    char from[32] = "machine audit", to[32] = {};
    copy(to,from,32);
    long expected_a = argc + 31L*argc + argc*(argc-1)/2;
    long expected_b = argc + 31L*(argc+1) + argc*(argc+1)/2;
    return a!=expected_a || b!=expected_b || destroyed!=2 || length(to)!=13;
}
