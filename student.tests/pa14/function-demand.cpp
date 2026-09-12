template<class T> T add(T a, T b) { return a + b; }
template<class T> T factorial(T n) { return n < 2 ? T(1) : n * factorial<T>(n - 1); }
template<class T> T& reference(T& n) { return n; }
template<class T> int local_size() { struct Local { T value; }; return sizeof(Local); }
template<class T> int unused(T t) { return t.missing(); }
int main() {
    int n = 4;
    reference(n) = 7;
    if (n != 7 || add(3, 4) != 7 || add<long>(8, 9) != 17) return 1;
    if (factorial<int>(6) != 720 || factorial<long>(5) != 120) return 2;
    if (local_size<int>() != 4 || local_size<long>() != 8 || local_size<int>() != 4) return 3;
    return 0;
}
