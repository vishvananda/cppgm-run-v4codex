template<class T> T&& move(T& t) { return static_cast<T&&>(t); }
template<class T> struct pointer {
  T value;
  pointer() : value(7) {}
  pointer(pointer&& other) : value(other.value) { other.value = 0; }
  void swap(pointer& other) { T x = value; value = other.value; other.value = x; }
  void reset() { pointer().swap(*this); }
  pointer& operator=(pointer&& other) {
    pointer(move(other)).swap(*this);
    return *this;
  }
};
template<class T> struct pointer<T*> {
  int value;
  pointer() : value(11) {}
  void swap(pointer& other) { int x = value; value = other.value; other.value = x; }
  void reset() { pointer().swap(*this); }
};
int main() {
  pointer<int> a, b;
  a.value = 3; b.value = 5;
  a = move(b);
  if (a.value != 5 || b.value != 0) return 1;
  a.reset();
  pointer<int*> c; c.value = 1; c.reset();
  return a.value == 7 && c.value == 11 ? 0 : 2;
}
