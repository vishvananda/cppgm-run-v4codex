int copied, destroyed;
struct V {
    V() {}
    V(const volatile V&) { if (++copied == 2) throw 7; }
    ~V() { ++destroyed; }
};
template<class... T> void discard(T&... objects) { (..., objects); }
int main() {
    volatile V a,b,c;
    try { discard(a,b,c); } catch (int n) {
        return n == 7 && copied == 2 && destroyed == 1 ? 0 : 1;
    }
    return 2;
}
