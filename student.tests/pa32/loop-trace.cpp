// Demanded template -> affine phi/step -> bounded unroll -> native ELF.
volatile long observed;
struct Seed { long value; explicit Seed(long n) : value(n) {} };
template<int N> long accumulate(long x) {
    Seed state(x);
    for (int i = 0; i < N; ++i) {
        state.value += i;
        observed = state.value;
    }
    return state.value;
}
int main(int argc, char**) {
    long a = accumulate<4>(argc);
    long b = accumulate<4>(argc + 2);
    return a != argc + 6 || b != argc + 8 || observed != b;
}
