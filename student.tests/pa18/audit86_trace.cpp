// PA18 audit: one declaration and demanded template through shared fact owners.
int copies, destructors, markers;
struct Marker { ~Marker() { ++markers; } };
struct Value {
  Value() {}
  Value(volatile Value&) { ++copies; }
  ~Value() { ++destructors; }
};
template<class T> struct Base {
  T value;
  Base() : value(7) {}
  Base(int) : Base() {}
};
template<class T> struct Derived : Base<T> { Derived() : Base<T>(1) {} };
template<class T, bool Quiet = noexcept(void(*static_cast<T*>(0)))>
struct Effects { static const bool quiet = Quiet; };
template<int... N> int run(volatile Value& value) {
  long fixed[][2] = { N... };
  int observed[] = { (Marker(), 3), ((void)value, 5) };
  Derived<int> object;
  static_assert(!Effects<volatile Value>::quiet, "copy can throw");
  return sizeof(fixed) != 4*sizeof(long) || fixed[1][1] != 11 ||
    observed[0] != 3 || observed[1] != 5 || object.value != 7;
}
int main() {
  volatile Value value;
  return run<2,3,5,11>(value) || copies != 1 || destructors != 1 || markers != 1;
}
