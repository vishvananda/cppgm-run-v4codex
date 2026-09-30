struct alignas(32) Pair {
    long left;
    long right;
};

thread_local long total = 3;

long read(const Pair& pair) {
    return pair.left + pair.right;
}

template<int Bias> struct Accumulator {
    Pair value;
    long step(double amount) {
        total += read(value) + (long)amount + Bias;
        return total;
    }
    long dormant() { return missing(*this); }
};

int main(int argc, char**) {
    Accumulator<7> accumulator;
    accumulator.value.left = argc;
    accumulator.value.right = 11;
    long result = accumulator.step(2.5);
    return result != argc + 23 || total != result;
}
