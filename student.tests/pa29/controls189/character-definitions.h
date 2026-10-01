namespace std {
template<class T> struct char_traits {
  typedef unsigned int_type;
  static T to_char_type(int_type value) { return T(value); }
  static int_type to_int_type(T value) { return value; }
};
}
