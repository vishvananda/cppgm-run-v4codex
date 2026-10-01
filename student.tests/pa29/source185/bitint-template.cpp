template<int N> struct holder {
  using signed_type = _BitInt(N);
  using unsigned_type = unsigned _BitInt(N);
  signed_type value;
  constexpr holder(int x):value(static_cast<signed_type>(x)) {}
  int count() const;
  template<class T> int dormant() { return T::missing; }
};
template<int N> int holder<N>::count() const {
  return static_cast<_BitInt(N)>(value)+static_cast<unsigned _BitInt(N)>(2);
}
constexpr holder<9> h(1023);
static_assert(h.value==-1,"dependent width");
template<int N> auto choose(int) -> decltype(sizeof(unsigned _BitInt(N)),char()) { return 1; }
template<int N> long choose(...) { return 2; }
static_assert(sizeof(choose<0>(0))==sizeof(long),"width SFINAE");
static_assert(sizeof(choose<9>(0))==1,"valid width");
int main() { holder<9> x(4); return x.count()==6 ? 0 : 1; }
