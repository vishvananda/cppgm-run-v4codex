constexpr __float128 subnormal = 0x0.ffffffffffffffffffffffffffffp-16382Q;
constexpr __float128 normal = 0x1p-16382Q;
static_assert(!__builtin_isnormal(subnormal), "quad max subnormal");
static_assert(__builtin_isnormal(normal), "quad min normal");
int main() {
    volatile __float128 a = subnormal, b = normal;
    if (__builtin_isnormal(a) || !__builtin_isnormal(b)) return 1;
    if (__builtin_fpclassify(11, 12, 13, 14, 15, a) != 14) return 2;
    return __builtin_fpclassify(11, 12, 13, 14, 15, b) == 13 ? 0 : 3;
}
