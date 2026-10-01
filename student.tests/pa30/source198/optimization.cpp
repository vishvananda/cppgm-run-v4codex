volatile int observations;
__attribute__((always_inline)) inline int scale(int x) noexcept { return x*3+1; }
__attribute__((noinline)) int offset(int x) { ++observations; return x+2; }
template<class T> struct left { typedef T value_type; };
template<class T> struct right { using value_type = T; };
template<class T> struct combine : left<T>,right<T> {
    static int apply(int x) {
        typename combine::value_type y = scale(x);
        return y+offset(x);
    }
    template<class U> static int dormant() { return U::missing; }
};
int main(int argc,char**) {
    int sum=0;
    for (int i=0;i<argc*16;++i) sum+=combine<int>::apply(i);
    return sum==528 && observations==16 ? 0 : 1;
}
