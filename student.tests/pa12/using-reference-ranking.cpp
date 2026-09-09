struct Base {
  int pick(int x) & { return x + 1; }
  int pick(int x) && { return x + 2; }
};
struct Derived : Base {
  using Base::pick;
  int pick(double x) & { return (int)x + 3; }
};
struct Further : Derived {};
int main() {
  Further value;
  if (value.pick(4) != 5 || value.pick(4.0) != 7) return 1;
  return static_cast<Further&&>(value).pick(4) == 6 ? 0 : 2;
}
