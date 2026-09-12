struct Stream { int sum; };
template<class T> Stream& operator<<(Stream& s, const T& x) { s.sum += x; return s; }
template<class A, class B> long mixed(A a, B b) { return a + b; }
template<class T> T identity(T x) { return x; }
template<class T> T with_default(T a, T b = T(4)) { return a + b; }
template<class T> int unused_default(T, int n = T::missing) { return n; }
int category(int&) { return 1; }
int category(int&&) { return 2; }
template<class T> int forward_category(T&& x) { return category(static_cast<T&&>(x)); }
int main() {
    Stream s = {0};
    s << 3 << 5L;
    if (s.sum != 8 || mixed<long>(3, 4) != 7) return 1;
    int (*f)(int) = identity;
    if (f(9) != 9 || with_default(3) != 7 || with_default(1, 2) != 3) return 2;
    int n = 0;
    if (forward_category(n) != 1 || forward_category(1) != 2) return 3;
    return unused_default(1, 0);
}
