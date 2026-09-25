// First-signature identity, scalar/reference facts and retained conversion use.
long select(double);
template<class T> auto apply(T, decltype(select(T())) n) -> decltype(select(T()));
int select(int);
template<class U> auto apply(U, decltype(select(U())) n) -> decltype(select(U())) {
    static_assert(sizeof(n) == sizeof(long), "first signature owns parameter type");
    return n;
}
template<int N> struct Scalar {
    static const int value = N;
    operator int() const { return value; }
    auto explicit_value() const -> decltype(operator int()) { return operator int(); }
};
int effects;
Scalar<65543> object;
Scalar<65543>& receiver() { ++effects; return object; }
template<class T> int consume(T& x) {
    const short& value = x;
    return apply(0, value);
}
int main() {
    using S = Scalar<65543>;
    using P = decltype(&S::operator int);
    P p = &S::operator int;
    const short& a = receiver() ? 7 : 9;
    const short& b = receiver() ? 7 : 9;
    return consume(receiver()) != 7 || a != 7 || b != 7 || &a == &b ||
        object.explicit_value() != 65543 || (object.*p)() != 65543 || effects != 3;
}
