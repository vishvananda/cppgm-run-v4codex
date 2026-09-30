struct Pair { long left; long right; };
long sum(Pair* p, long extra) { return p->left + p->right + extra; }

template<int N> struct Accumulator {
    long value;
    long apply(long x) { return value + x + N; }
    long dormant() { return this->missing; }
};

int main(int argc, char**) {
    Pair pair = {17, 19};
    Accumulator<6> accumulator = {5};
    return accumulator.apply(sum(&pair, argc)) != 47 + argc;
}
