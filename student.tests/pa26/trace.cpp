int destroyed;
struct Guard { ~Guard() { ++destroyed; } };
template<int N> int calculate(int input) {
    try {
        Guard guard;
        if (input) throw N;
        return 0;
    } catch (int value) { return value + destroyed; }
}
int main(int argc, char**) { return calculate<11>(argc) == 12 ? 0 : 1; }
