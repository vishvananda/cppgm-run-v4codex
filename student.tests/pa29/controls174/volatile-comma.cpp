int copied, destroyed;
struct V {
    V() {}
    V(const volatile V&) { ++copied; }
    ~V() { ++destroyed; }
};
template<class... T> void discard(T&... objects) { (objects, ...); }
int main() {
    volatile V a, b, c;
    discard(a, b, c);
    return copied == 3 && destroyed == 3 ? 0 : 1;
}
