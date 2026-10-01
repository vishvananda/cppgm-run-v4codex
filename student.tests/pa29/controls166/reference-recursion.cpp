constexpr bool active() { return __builtin_is_constant_evaluated(); }
constexpr int sum(int n) { int&& x = active() ? 17 : 29; int& r = x; if(n) r += sum(n-1); return x; }
static_assert(sum(3)==68, "recursive reference storage");
int main() { int arr[]{sum(3)}; return arr[0] == 68 && sum(3) == 68 ? 0 : 1; }
