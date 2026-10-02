#include <vector>
#include <memory>
#include <functional>
#include <sstream>

int live = 0;
struct Item {
    int value;
    Item(int n) : value(n) { if (n < 0) throw n; ++live; }
    Item(const Item& n) noexcept : value(n.value) { ++live; }
    ~Item() noexcept { --live; }
};
struct Base { int value; Base(int n = 7) noexcept : value(n) {} };
template<class T> struct Inherited : T {
    using T::T;
    Inherited(Inherited&&) = default;
    template<class U> int dormant() { return U::missing; }
};
template<class T> struct Pipeline {
    __attribute__((always_inline)) static int twice(int n) noexcept { return n * 2; }
    __attribute__((noinline)) static int step(int n) noexcept { return twice(n) + 1; }
    static int run(int n) {
        Inherited<Base> initial;
        std::vector<Item> items = {n, n + 1, initial.value};
        std::unique_ptr<int[]> storage(new int[items.size()]);
        std::function<int(int)> call = &step;
        int total = 0;
        for (unsigned i = 0; i < items.size(); ++i) {
            storage[i] = call(items[i].value);
            total += storage[i];
        }
        std::ostringstream out;
        out << total;
        return total + (out.str().empty() ? 1000 : 0);
    }
};
int main(int argc, char** argv) {
    if (argc != 2) return 1;
    int n = argv[1][0] - '0';
    if (Pipeline<int>::run(n) != 4*n + 19 || live) return 2;
    try { std::vector<Item> bad = {n, -1}; return 3; }
    catch (int x) { if (x != -1 || live) return 4; }
    return 0;
}
