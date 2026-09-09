void* operator new(unsigned long, void* p) noexcept { return p; }
struct Table { unsigned value; };
struct Item { int value; Item(int n) : value(n) {} };
int operator ""_length(const char*, unsigned long n) { return n; }
int main() {
    unsigned storage = 0;
    Table* table = ::new(&storage) Table{7};
    if (table->value != 7) return 1;
    int more = 0;
    Item* item = ::new(&more) Item(9);
    if (item->value != 9 || "hello"_length != 5) return 2;
    volatile long double one = 1.0L, zero = 0.0L;
    long double inf = one / zero;
    long double nan = zero / zero;
    if (__builtin_isfinite(inf) || !__builtin_isinf(inf) || __builtin_isnormal(nan)) return 3;
    if (__builtin_fpclassify(11,22,33,44,55,one) != 33) return 4;
    if (__builtin_fpclassify(11,22,33,44,55,zero) != 55) return 5;
    if (__builtin_fpclassify(11,22,33,44,55,nan) != 11) return 6;
    return __builtin_strlen("done") == 4 ? 0 : 7;
}
