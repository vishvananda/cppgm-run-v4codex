namespace traits {
template<class T> struct supported { static const bool value = true; };
}
template<bool B, class T = int> struct condition { typedef T type; };
template<class T> using enabled = typename condition<traits::supported<const T&>::value>::type;
template<class T> struct callable { typedef T (signature)(const T&); };

long select(double);
template<class X> struct member {
  template<class T> int value(T, const decltype(select(T())));
};
int select(int);
template<class X> template<class T>
int member<X>::value(T, decltype(select(T())) n) {
  static_assert(sizeof(n) == sizeof(long), "first signature owns lookup");
  n += sizeof(X);
  return n;
}

struct provider { template<class T> T get(int n) { return n; } };
struct int_arg { typedef int type; };
struct long_arg { typedef long type; };
int sum(int x, long y) { return x + y; }
template<class... A> int expanded(provider& p, int n) {
  return sum(p.get<typename A::type>(n)...);
}

long fixed(double);
template<class T> auto result(T x) -> decltype(fixed(sizeof(T))) { return x; }
int fixed(unsigned long);
template<class T> auto result(T x) -> decltype(fixed(sizeof(T))) { return x + 1; }

template<class T> enabled<T> indirect(const T& n) { return n; }
int main() {
  member<char> m;
  provider p;
  callable<int>::signature* fp = indirect<int>;
  long (*a)(int) = result;
  int (*b)(int) = result;
  volatile int n = 7;
  return m.value(0,n) != 8 || expanded<int_arg,long_arg>(p,n) != 14 ||
      fp(7) != 7 || a(n) != 7 || b(n) != 8;
}
