int main() {
    const int a[2] = {3, 4};
    auto [x,y] = a;
    static_assert(__is_same(decltype(x),const int), "array cv must be preserved");
    volatile int b[2] = {5,6};
    auto [p,q] = b;
    static_assert(__is_same(decltype(p),volatile int), "volatile element copy");
    return x+y+p+q != 18;
}
