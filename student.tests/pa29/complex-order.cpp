#define CHECK(T, suffix) \
__attribute__((noinline)) __complex__ T add_##suffix(__complex__ T a, __complex__ T b) { return a+b; } \
__attribute__((noinline)) __complex__ T sub_##suffix(__complex__ T a, __complex__ T b) { return a-b; } \
bool check_##suffix(int n) { \
    __complex__ T a{T(n),T(2.5)}, b{T(1.25),T(4)}; \
    auto sum=add_##suffix(a,b), diff=sub_##suffix(a,b); \
    return __real__ sum==T(n)+T(1.25) && __imag__ sum==T(6.5) && \
        __real__ diff==T(n)-T(1.25) && __imag__ diff==T(-1.5); \
}
CHECK(float, f)
CHECK(double, d)
CHECK(long double, l)
int main(int argc,char**) { return !(check_f(argc) && check_d(argc) && check_l(argc)); }
