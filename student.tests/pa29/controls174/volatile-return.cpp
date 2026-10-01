int copied;
struct V { V() {} V(const volatile V&) { ++copied; } };
volatile V& ref(volatile V& value) { return value; }
template<class... T> void discard(T&... objects) { (ref(objects), ...); }
int main() { volatile V a,b; discard(a,b); return copied; }
