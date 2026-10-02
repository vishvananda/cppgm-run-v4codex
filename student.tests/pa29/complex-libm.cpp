// GNU Library Builtins: typed C-library calls for the three complex precisions.
#define CHECK(T, S, EPS) \
int check_##S(int n) { \
    T x = T(n*3), y = T(n*4); \
    __complex__ T z = __builtin_complex(x,y); \
    if (__builtin_cabs##S(z) != T(n*5)) return 1; \
    __complex__ T conjugate = __builtin_conj##S(z); \
    if (__builtin_creal##S(conjugate) != x || __builtin_cimag##S(conjugate) != -y) return 2; \
    __complex__ T root = __builtin_csqrt##S(z); \
    __complex__ T square = __builtin_cpow##S(root,__builtin_complex(T(2),T(0))); \
    if (__builtin_cabs##S(square-z) > T(EPS)) return 3; \
    __complex__ T projected = __builtin_cproj##S(z); \
    return __builtin_creal##S(projected) != x || __builtin_cimag##S(projected) != y; \
}
CHECK(float, f, .0001)
CHECK(double, , .00000001)
CHECK(long double, l, .00000001)
int main(int argc,char**) { return check_f(argc) || check_(argc) || check_l(argc); }
