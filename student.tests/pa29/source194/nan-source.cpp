constexpr unsigned short half_bits = 0xfc01;
constexpr __uint128_t quad_bits = (__uint128_t(0xffff) << 112) | 123;
constexpr _Float16 half = __builtin_bit_cast(_Float16, half_bits);
constexpr __float128 quad = __builtin_bit_cast(__float128, quad_bits);
static_assert(__builtin_bit_cast(unsigned short, half) == half_bits, "half signaling bits");
static_assert(__builtin_bit_cast(__uint128_t, quad) == quad_bits, "quad signaling bits");
_Float16 halves[] = {half, half};
__float128 quads[] = {quad, quad};
template<class T> T identity(T x) { return x; }
int main() {
    if (__builtin_bit_cast(unsigned short, identity(halves[1])) != half_bits) return 1;
    if (__builtin_bit_cast(__uint128_t, identity(quads[1])) != quad_bits) return 2;
    return 0;
}
