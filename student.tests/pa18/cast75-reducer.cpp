struct B {}; struct D : virtual B {};
template<class, class> char test(...);
template<class T, class U, class = decltype((U *)((T *)0))> long test(int);
static_assert(sizeof(test<B,D>(0)) == sizeof(char), "selection");
template<class, class> constexpr bool probe(...) { return true; }
template<class T, class U, class = decltype((U *)((T *)0))> constexpr bool probe(int) { return false; }
int main() { return !probe<B,D>(0); }
