// Final PA17 trace: partial owner -> member definition -> closure effect -> LowIR.
template<class T> struct Selected { static const int value = 1; };
template<class T> struct Selected<T*> { static const int value = sizeof(T); };
template<class T> using Alias = Selected<T*>;
template<class T> struct Owner { template<class U> int run(U); };
int cleanups;
struct Local { ~Local() noexcept { ++cleanups; } };
template<class X> template<class Y> int Owner<X>::run(Y n) {
    Local live;
    auto f = [](const X x) noexcept(sizeof(x) == sizeof(X)) {
        return x + Alias<X>::value;
    };
    static_assert(noexcept(f(1)), "selected exception fact");
    X (*p)(X) = f;
    return f(n) + p(n);
}
struct Base { int n; constexpr Base(int v):n(v){} constexpr int get()const{return n;} };
struct Left:Base { constexpr Left(int n):Base(n){} };
struct Right:Base { constexpr Right(int n):Base(n){} };
template<int N> struct Both:Left,Right { constexpr Both():Left(N),Right(N+7){} };
template<int N> struct Value { static const int value=N; };
static_assert(Value<(Both<0>().Right::get())>::value==7,"qualified constant key");
int main() {
    Owner<int> a; Owner<long> b;
    return a.run(3)!=14 || b.run(3L)!=22 || cleanups!=2;
}
