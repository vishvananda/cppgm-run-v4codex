template<int N> int run(int& x, long& y) {
  auto outer = [&]() {
    auto inner = [&]() { x += N; y += N; return x + y; };
    auto copied = inner;
    return copied();
  };
  return outer();
}
struct Holder {
  int n;
  int read(int& x) {
    auto closure = [this, &x]() { return n + x; };
    return closure();
  }
};
int main() {
  int x = 1; long y = 2;
  if (run<2>(x,y) != 7 || run<2>(x,y) != 11 || run<3>(x,y) != 17) return 1;
  Holder object = {3};
  return object.read(x) != 11 || x != 8 || y != 9;
}
