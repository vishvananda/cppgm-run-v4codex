int deaths = 0;
template<class T, class U = T> struct Pair {
    T first;
    U second;
    Pair(T a, U b) : first(a), second(b) {}
    ~Pair() { ++deaths; }
    T sum() { return first + second; }
    int unused() { return first.no_such_member(); }
    struct Nested { T field; };
};
template<class T> struct Base { T value; T get() { return value; } };
template<class T> struct Derived : Base<T> { T twice() { return this->get() * 2; } };
int main() {
    {
        Pair<int> a(3, 4);
        Pair<long, int> b(5, 6);
        if (a.sum() != 7 || b.sum() != 11) return 1;
        if (sizeof(Pair<int>::Nested) != 4 || sizeof(Pair<long>::Nested) != 8) return 2;
        Derived<int> d;
        d.value = 7;
        if (d.twice() != 14) return 3;
    }
    return deaths == 2 ? 0 : 4;
}
