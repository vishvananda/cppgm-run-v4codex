namespace library {
template<class T> T identity(T);
template<class U> U identity(U x) { return x; }
template<class V> V identity(V);
template<class T> int category(const T&) { return sizeof(T); }
}
template<class T> int choose(T) { return 1; }
int choose(int) { return 2; }
struct Callable { int operator()() { return 3; } int operator()() const { return 4; } };
template<class T> int invoke(T value) { return value(); }
int main() {
    Callable f;
    if (invoke<Callable>(f) != 3 || invoke<const Callable>(f) != 4) return 1;
    if (library::identity(5) != 5 || library::identity<long>(6) != 6) return 2;
    if (choose(1) != 2 || choose(1L) != 1) return 3;
    return 0;
}
