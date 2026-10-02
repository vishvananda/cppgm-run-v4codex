// Source -> demanded template facts -> typed LowIR -> MIR -> ELF audit probe.
volatile int destroyed;
int bias() noexcept { return 3; }
template<class T> struct Pair {
    T first, second;
    explicit Pair(T n) : first(n), second(n + 1) {}
    T sum() const { return first + second; }
    ~Pair() { ++destroyed; }
};
template<class T> T demand(T x) {
    Pair<T> p(x);
    return p.sum() + p.sum() + bias();
}
long use(long x) { return demand(x) + demand(x + 1); }
int main(int argc, char**) {
    long observed = use(argc);
    int other = demand(argc);
    return observed != 8 * argc + 14 || other != 4 * argc + 5 || destroyed != 3;
}
