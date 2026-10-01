int copied;
struct V { V() {} V(const volatile V&) { ++copied; } };
template<class... T> void discard(T&... objects) { (..., objects); }
int main() { volatile V a; discard(a); return copied == 1 ? 0 : 1; }
