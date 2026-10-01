template<class T> struct incomplete;
incomplete<int>* p = nullptr;
template<class T> struct owner {
  static int dormant() { return incomplete<T>::value; }
  static int live(int n) { return n + 3; }
};
template<class T> auto choose(T*, int) -> typename T::value_type { return 7; }
template<class T> int choose(T*, ...) { return 9; }
struct present { typedef int value_type; };
struct absent {};
struct hidden { private: typedef int value_type; };
static_assert(sizeof(owner<int>) == 1, "declarations only");
int main(int argc, char**) {
  present x; absent y; hidden z;
  return owner<int>::live(argc) == argc + 3 && choose(&x, 0) == 7 &&
         choose(&y, 0) == 9 && choose(&z, 0) == 9 ? 0 : 1;
}
