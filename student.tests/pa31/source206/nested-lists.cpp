#include <initializer_list>
int live = 0;
struct Element {
    int value;
    Element(int n) noexcept : value(n) { ++live; }
    ~Element() noexcept { --live; }
};
struct Inner {
    int value;
    Inner(std::initializer_list<Element> xs) noexcept : value(0) {
        for (const Element& x : xs) value += x.value;
    }
};
struct Outer {
    int value;
    Outer(std::initializer_list<Inner> xs) noexcept : value(0) {
        for (const Inner& x : xs) value += x.value;
    }
};
int consume(const Outer& x) noexcept { return x.value; }
int fail() { throw 23; }
static_assert(noexcept(consume({{1,2},{3}})), "nested nonthrowing lists");
static_assert(!noexcept(consume({{1,fail()},{3}})), "nested throwing source");
int main() {
    int result = consume({{1,2},{3}});
    if (result != 6 || live) return 1;
    try { consume({{1,fail()},{3}}); return 2; }
    catch (int n) { return n == 23 && !live ? 0 : 3; }
}
