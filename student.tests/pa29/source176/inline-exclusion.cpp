#define EX __attribute__((exclude_from_explicit_instantiation))
template<class T> struct Members {
  EX inline static int used = 31;
  EX inline static int dormant = T::missing;
  inline static int supplied = 9;
};
extern template struct Members<int>;
template struct Members<long>;
int main() { return Members<int>::used + Members<int>::supplied - 40; }
