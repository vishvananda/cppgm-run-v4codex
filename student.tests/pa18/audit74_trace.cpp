// Accumulated source/context, correlated alias, callable and access trace.
template<class A,class B> struct Pair {};
template<class...> struct List {};
int effects;
int sum(int a,int b) { return a+b; }
struct Base { protected: operator int() { ++effects; return 3; } };
class Exposed : private Base { public: using Base::operator int; };
template<class...T> struct Processor {
    template<class...U> using Zip = List<Pair<T,U>...>;
    static int leaf(int,int) noexcept;
    template<class...U,class=Zip<U...>>
    auto run(U...u) const noexcept(noexcept(this->leaf(u...)))
        -> decltype((leaf)(u...)) { return __builtin_invoke(sum,u...); }
    int run(...) const { return 99; }
    template<class U> int dormant() { return U::missing; }
};
template<class F,class...A>
auto forward_call(F&&f,A...a)->decltype((f)(a...)) { return (f)(a...); }
int main() {
    Processor<int,long> p;
    Exposed e;
    int value=e;
    static_assert(noexcept(p.run(1,2)), "member exception context");
    return p.run(value,4)!=7 || p.run(1)!=99 ||
        forward_call(sum,2,5)!=7 || effects!=1;
}
