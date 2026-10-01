struct V { V() {} V(const volatile V&) noexcept(false) {} };
struct W { W() {} W(const volatile W&) noexcept {} };
template<class... T> constexpr bool test(T&... x) { return noexcept((x, ...)); }
int main() {
    volatile V a,b; volatile W c,d;
    static_assert(!noexcept((a,b)), "ordinary comma copy may throw");
    return !test(a,b) && test(c,d) && test(a) ? 0 : 1;
}
