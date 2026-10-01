// Reduced type/representation check for the hosted Q suffix.
static_assert(__is_same(decltype(0x1.0p-16382Q),__float128),"Q selects binary128");
static_assert(!__is_same(decltype(0x1.0p-16382Q),long double),"Q is not L");
constexpr auto bits=__builtin_bit_cast(__uint128_t,0x1.0p-16382Q);
static_assert(bits==(__uint128_t(1)<<112),"binary128 minimum normal exponent");
int main(){return bits!=(__uint128_t(1)<<112);}
