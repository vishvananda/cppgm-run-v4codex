template<class T> struct Box {
  Box() { T::missing(); }
  ~Box() { T::missing(); }
  int call() { return T::missing; }
  static int value;
  friend int call(Box&) { return T::missing; }
};
template<class T> int Box<T>::value=T::missing;
int main() {
  if constexpr(false) { Box<int> b; b.call(); call(b); return Box<int>::value; }
  return 0;
}
