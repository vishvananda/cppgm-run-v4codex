// PA17 audit: shared receiver, query, definition and transfer owners.
struct Base { int n; int read() const { return n; } };
struct Left : Base {};
struct Right : Base {};
template<class T> struct Both : Left, T {
    template<class U = int, class V> int add(V v) const {
        return T::read() + sizeof(U) + v;
    }
};
int seven() { return 7; }
template<class T> struct TableOwner { static decltype(&seven) fn; };
template<class T> decltype(&seven) TableOwner<T>::fn = &seven;
template<class T> T&& val();
template<class T, class = void> struct Probe { static const int n = 0; };
template<class T> struct Probe<T, decltype(val<T>().read(), void())> {
    static const int n = 1;
};
struct Empty {};
template<class T> struct Sparse { Empty e; T n; };
int main() {
    Both<Right> x;
    static_cast<Left&>(x).n = 3;
    static_cast<Right&>(x).n = 7;
    Sparse<int> a, b;
    a.n = x.add(2) + TableOwner<Right>::fn();
    b = a;
    return b.n != 20 || x.Left::read() != 3 || Probe<Right>::n != 1;
}
