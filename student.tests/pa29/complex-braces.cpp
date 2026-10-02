// GNU complex component lists used by the selected hosted <complex> constructors.
typedef __complex__ double Number;
Number global = {3.0, 4.0};
constexpr Number fixed = {5.0, 6.0};
static_assert(__real__ fixed == 5.0 && __imag__ fixed == 6.0, "both constant components");
struct Box {
    Number value;
    constexpr Box(double r, double i):value{r,i} {}
};
constexpr Box fixed_box(7.0,8.0);
static_assert(__imag__ fixed_box.value == 8.0, "constexpr member components");
template<class T> Number pair(T r,T i) { Number z{r,i}; return z; }
Number make(double r,double i) { return {r,i}; }
double sum(Number z) { return __real__ z + __imag__ z; }
int order;
double component(int expected) { if (++order != expected) return -1000.0; return expected; }
int main(int argc,char**) {
    Box local(argc,argc+1);
    Number z{component(1),component(2)};
    if (sum(z) != 3.0 || order != 2) return 1;
    if (sum(global) != 7.0 || sum(fixed) != 11.0 || sum(fixed_box.value) != 15.0) return 2;
    if (sum(local.value) != argc*2+1) return 3;
    if (sum(pair(double(argc),double(argc+1))) != argc*2+1) return 4;
    if (sum(make(argc,argc+1)) != argc*2+1 || sum({1.0,2.0}) != 3.0) return 5;
    const Number& ref{double(argc),2.0};
    Number copied{ref}, empty{}, one{3.0};
    if (sum(copied) != argc+2 || sum(empty) != 0 || sum(one) != 3.0) return 6;
    Number array[] = {{1.0,2.0},{3.0,4.0}};
    if (sum(array[0])+sum(array[1]) != 10.0) return 7;
    __complex__ float narrow{float(argc),-0.0f};
    __complex__ long double wide{(long double)argc,4.0L};
    return __real__ narrow != argc || !__builtin_signbit(__imag__ narrow) ||
        __real__ wide != argc || __imag__ wide != 4.0L;
}
