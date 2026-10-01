constexpr bool active() { return __builtin_is_constant_evaluated(); }
struct Box { int value; constexpr Box() : value(active() ? 17 : 29) {} };
constexpr int read() { return Box().value; }
constexpr int a = read();
int main() {
    int folded[] = {read()};
    int direct = read();
    return a == 17 && folded[0] == 29 && direct == 29 ? 0 : 1;
}
