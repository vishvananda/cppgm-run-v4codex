int copied;
struct V { V() {} V(const volatile V&) { ++copied; } };
template<class... T> volatile V& last(T&... objects) { return (..., objects); }
int main() {
    volatile V a,b,c;
    volatile V& result = last(a,b,c);
    return &result == &c && copied == 2 ? 0 : 1;
}
