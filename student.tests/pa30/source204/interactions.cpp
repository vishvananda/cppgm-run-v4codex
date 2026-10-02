// Full-stage audit: source identities, deferred template demand, lifetimes,
// vector storage, target facts and native emission meet in one executable.
template<int... I> struct Seq {};
template<int N> struct Build {
    template<class, int... I> using Map = Seq<I...>;
    using type = __make_integer_seq<Map, int, N>;
};
template<class T> struct Left { typedef T value_type; };
template<class T> struct Right { using value_type = T; };
struct Base { protected: static int value() { return 29; } };
template<class T> struct Mid : Base {};
template<class T> struct Child : Mid<T> {
    int get() { return Base::value(); }
};
template<class T> struct Outer {
private:
    static int secret() { return 7; }
public:
    struct Inner {
        friend class Outer<T>;
    private:
        int get() { return Outer<T>::secret(); }
    };
    int get() { return Inner().get(); }
};
struct Complete {
    template<class T> struct Inner { int get() { return Complete().value; } };
    int value;
    Complete() : value(3) {}
};
int destroyed;
struct Guard { Guard(int) {} ~Guard() { ++destroyed; } };
struct Later { Later(int) { throw 13; } };
struct Parts { Guard guard; Later later; };
union Variant { Parts parts; int unused; ~Variant() {} };
struct Aggregate { Variant variant; };
int cleanup() {
    try { Aggregate a = {{{7, 13}}}; }
    catch (int n) { return n; }
    return 0;
}
struct Count { int n; operator int() const { return n; } };
template<class T> auto allocate(Count n) -> decltype(new T[n]) { return new T[n]; }
[[noreturn]] void stop() { throw 5; }
int select(int n) { if (n ? 2 : 0) return 17; stop(); }
int default_lambda(int n = []() { int x = 11; return [=]() { return x; }(); }()) { return n; }
typedef short Shorts __attribute__((vector_size(8)));
typedef char Bytes __attribute__((vector_size(8)));
typedef float Floats __attribute__((vector_size(16)));
typedef int Integers __attribute__((vector_size(16)));
volatile Integers observed;
__attribute__((always_inline)) inline int scale(int x) noexcept { return x * 3 + 1; }
template<class T> struct Pipeline : Left<T>, Right<T> {
    template<class U> static int dormant() { return U::missing; }
    template<int... I> static int count(Seq<I...>) { return sizeof...(I); }
    static int run(int input) {
        typename Pipeline::value_type n = scale(input);
        Shorts a = {(short)n, -129, 128, 32767};
        Bytes packed = __builtin_ia32_packsswb(a, Shorts{-32768, -1, 0, 1});
        Floats f = __builtin_ia32_addss(Floats{(float)packed[0], 2, 3, 4}, Floats{5, 6, 7, 8});
        Integers values = __builtin_convertvector(f, Integers);
        observed = values;
        Integers saved = observed;
        observed[0] = 99;
        return saved[0] + saved[3] + packed[1] + packed[2] + count(Build<5>::type());
    }
};
int main(int argc, char**) {
    Child<int> child;
    Outer<int> outer;
    Complete::Inner<int> inner;
    int* a = allocate<int>(Count{2});
    a[1] = default_lambda();
    auto capture = [&] { return a[1]; };
    int sum = capture() + child.get() + outer.get() + inner.get() + select(1);
    delete[] a;
    int before = destroyed;
    int caught = cleanup();
    int result = Pipeline<int>::run(argc);
    return sum == 67 && caught == 13 && destroyed == before + 1 &&
        result == argc * 3 + 14 && observed[0] == 99 ? 0 : 1;
}
