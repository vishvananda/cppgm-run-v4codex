struct character {
  unsigned code;
  character(unsigned c = 0) : code(c) {}
  operator unsigned() const { return code; }
};
template<class T> struct traits;
template<class T> struct probe {
  typedef typename traits<T>::int_type int_type;
  static T to_char(int_type value) { return traits<T>::to_char_type(value); }
  static int_type to_int(T value) { return traits<T>::to_int_type(value); }
  // Class demand must not instantiate this unused definition.
  static int dormant() { return T::missing; }
};
probe<character>* deferred;
template<class T> struct traits {
  typedef unsigned int_type;
  static T to_char_type(int_type value) { return T(value); }
  static int_type to_int_type(T value) { return value; }
};
template<> struct traits<int> {
  typedef long int_type;
  static int to_char_type(long value) { return int(value) + 2; }
  static long to_int_type(int value) { return value - 2; }
};
static_assert(__is_same(probe<character>::int_type, unsigned), "primary");
static_assert(__is_same(probe<int>::int_type, long), "specialization");
int main(int argc, char**) {
  character ch = probe<character>::to_char(argc + 6);
  int i = probe<int>::to_char(argc + 9);
  return probe<character>::to_int(ch) == unsigned(argc + 6) &&
         probe<int>::to_int(i) == argc + 9 ? 0 : 1;
}
