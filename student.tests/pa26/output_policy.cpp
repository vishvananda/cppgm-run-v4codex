int destroyed;
struct Guard { ~Guard() { ++destroyed; } };
template<int N> int checked() {
    try { Guard guard; throw N; }
    catch (int value) { return value + destroyed; }
}
int main() { return checked<11>() == 12 ? 0 : 1; }
