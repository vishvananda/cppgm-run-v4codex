int trace;
struct Guard {
  int value;
  Guard(int n) noexcept : value(n) {}
  ~Guard() noexcept { trace = trace * 10 + value; }
};
template<class T> T calculate(T n) {
  Guard outer(1);
  return (Guard(2), ({
    Guard inner(3);
    if (n < 0) return T(7);
    T x = n + 2;
    ({ x * 3; });
  }));
}
int main(int argc, char**) {
  int a = calculate(argc);
  if (a != 9 || trace != 321) return 1;
  trace = 0;
  long b = calculate(-1L);
  return b == 7 && trace == 321 ? 0 : 1;
}
