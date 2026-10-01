template<class... T> constexpr int drop(T&... values) { (values, ...); return 3; }
constexpr int test() { volatile int a=1; int b=2; return drop(a,b); }
static_assert(test() == 3, "discarding a volatile id reads its value");
int main() {}
