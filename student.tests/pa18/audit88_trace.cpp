// PA18 final audit: canonical result identity, extensible deduction prefix,
// one demanded body, retained ellipsis selection and concrete cleanup.
int copies, dead;
struct Packet {
  int value;
  Packet(int n) : value(n) {}
  Packet(const Packet& p) : value(p.value) { ++copies; }
  ~Packet() { ++dead; }
};
int sum(int a, long b) { return a + b; }
int sink(...) noexcept { return copies - dead; }
template<class... T> int dispatch(T... values) {
  Packet p(sum(values...));
  static_assert(!noexcept(sink(p)), "copy may throw");
  int result = p.value + sink(p);
  return result;
}
template<class T> using Identity = T;
struct Result { int value; Result(int n) : value(n) {} };
template<class T> Identity<T> make(int n) { return T(n); }
int main() {
  int (*call)(int, long) = dispatch<int>;
  Result (*construct)(int) = make<Result>;
  Result result = construct(call(6, 7L));
  return result.value != 14 || copies != 1 || dead != 2;
}
