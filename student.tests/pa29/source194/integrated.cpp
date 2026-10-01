#include <cmath>
#if !__has_builtin(__builtin_bit_cast) || !__has_builtin(__builtin_convertvector)
#error missing implemented builtin probe
#endif
typedef int V __attribute__((vector_size(16)));
typedef float F __attribute__((vector_size(16)));
constexpr __float128 precise = 0x1.0000000000000000000000000001p0Q;
constexpr _Float16 tiny = 0x1p-24F16;
static_assert(precise > 1 && tiny > 0, "retained representation");
template<int N> struct __attribute__((aligned(N))) Box {
    struct Inner {
        __attribute__((abi_tag("declaration"))) static __float128 apply(V);
        template<class Unused> static int dormant() { return Unused::missing; }
    };
};
template<int N> __float128 Box<N>::Inner::apply(V x) {
    F y = __builtin_convertvector(x, F);
    V bits = __builtin_bit_cast(V, y);
    if (__builtin_reduce_or(bits) == 0) return 0;
    return __float128(__builtin_reduce_or(__builtin_convertvector(y, V))) + precise + __float128(tiny);
}
__attribute__((always_inline)) inline __float128 twice(__float128 x) noexcept {
    return x * 2;
}
static_assert(alignof(Box<32>) == 32, "dependent alignment");
int main(int argc, char**) {
    V v = {argc, 2, 3, 4};
    __float128 expected = __float128(argc | 7) + precise + __float128(tiny);
    auto p = &Box<32>::Inner::apply;
    auto x = p(v);
    if (x != expected || twice(x) != expected * 2) return 1;
    if (!__builtin_isnormal(x) || !__builtin_signbit(-0.0Q)) return 2;
    return std::ceil(double(x)) == (argc | 7) + 2 ? 0 : 3;
}
