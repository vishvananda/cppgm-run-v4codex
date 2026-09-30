constexpr double half = 0X.8p+0;
constexpr double tiny = 0x1.8p-4;
static_assert(half == 0.5 && tiny == 0.09375, "hex literal value");
static_assert(0b1011U == 11U, "binary");
#if 0b11 != 3
#error binary controlling expression
#endif
constexpr unsigned long long operator"" _bits(unsigned long long x) { return x; }
static_assert(0b110_bits == 6, "cooked binary UDL");
int main() { return (0x1p4f + half == 16.5 && 0x1.8p+2L == 6.0L) ? 0 : 1; }
