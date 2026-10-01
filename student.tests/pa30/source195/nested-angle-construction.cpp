template<class T> struct flag { constexpr operator bool() const { return true; } };
template<class T> struct outer : T {};
template<bool V> struct choice { static const int value = V; };
template<class T> using nested = choice<outer<flag<T>>{}>;
static_assert(nested<int>::value, "nested close is followed by construction");
template<class T> struct holder { int value = 9; };
int main() { holder<flag<int>> x{}; return x.value != 9; }
