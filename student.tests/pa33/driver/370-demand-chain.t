// A deep dependency chain must remain complete when demand work is bounded.
template<int N> __attribute__((noinline)) int step(int value) {
    return step<N - 1>(value) + 1;
}
template<> __attribute__((noinline)) int step<0>(int value) { return value; }
int main() { return step<40>(2) != 42; }
