template<class T, T V> struct constant {
  static constexpr T value = V;
  operator T() const { return value; }
};
using yes = constant<bool, true>;
int effects;
yes object;
yes& receiver() { ++effects; return object; }
struct box {
  int value;
  explicit box(int n) : value(n) { ++effects; }
  ~box() { --effects; }
};
template<class C> box choose(C& c, int n) { return c ? box(n) : box(n + 99); }
int main() {
  {
    box result = choose(receiver(), 7);
    if (result.value != 7 || effects != 2) return 1;
    bool (yes::*p)() const = &yes::operator bool;
    if (!(object.*p)() || !object.operator bool()) return 2;
  }
  return effects != 1;
}
