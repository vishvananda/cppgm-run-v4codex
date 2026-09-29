// End-to-end trace: layout, alias identity, defaulted pack deduction, lazy
// bodies/values/storage, readonly array copying and named-constant forwarding.
template<class T> struct Pair {
    T first;
    long second;
    Pair(T a, long b) : first(a), second(b) {}
    long sum() const { return first + second; }
};
template<class T> struct Source {
    template<class U> using Rebind = Pair<U>;
    template<class U> static constexpr int width = sizeof(T) + sizeof(U);
    template<class U> static int dormant() { return U::missing; }
};
template<template<class> class F, class T> F<T> build(T x) {
    return F<T>(x, Source<T>::template width<long>);
}
template<class A = int, class B = long, class C = char> struct Tuple {};
template<class A, class... B> int count(Tuple<A, B...>) {
    return sizeof...(B);
}
int effects;
template<int N> struct Constant {
    static const int value = N;
    Constant() { ++effects; }
    ~Constant() { effects += 2; }
    operator int() const { return value; }
};
int main() {
    Source<int>::Rebind<short> p = build<Source<int>::Rebind>(short(3));
    int a[] = {1, 2, 3};
    int b[] = {1, 2, 3};
    a[0] = 9;
    int seven = Constant<7>();
    const int* left = &Source<int>::width<long>;
    const int* right = &Source<int>::width<long>;
    return p.sum() != 13 || count(Tuple<>()) != 2 || seven != 7 ||
        effects != 3 || b[0] != 1 || a == b || left != right || *left != 12;
}
