constexpr bool active() { return __builtin_is_constant_evaluated(); }
struct Pair { int first, second; constexpr Pair() : first(11), second(active() ? 17 : 29) {} };
int main() {
    const int& ref = Pair().second;
    const int& alias = ref;
    return ref == 17 && alias == 17 && &ref == &alias ? 0 : 1;
}
