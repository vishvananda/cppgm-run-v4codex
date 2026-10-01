struct V { V() {} V(const volatile V&) = delete; };
template<class... T> void discard(T&... objects) { (objects, ...); }
int main() { volatile V a; discard(a); }
