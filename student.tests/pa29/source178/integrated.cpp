int calls, dead;
constexpr int line(int n=__builtin_LINE()) { return n; }
template<class T> int initialize(int n=line()) { ++calls; return n; }
template<class T> struct Box {
#line 40 "member.hpp"
  inline static int data[] __attribute__((exclude_from_explicit_instantiation)) = {initialize<T>()};
  static int read() __attribute__((exclude_from_explicit_instantiation)) { return data[0]; }
};
extern template struct Box<int>;
static_assert(sizeof(Box<int>::data)==sizeof(int), "array definition");
struct Guard { ~Guard(){++dead;} };
template<class T> int work() {
  if constexpr(sizeof(T)==sizeof(int)) {
    if (Guard g; Box<T>::read()==40) return line(7);
    return 2;
  } else return T::missing;
}
int main() { return work<int>()==7 && calls==1 && dead==1 ? 0 : 1; }
