template<class T> struct base {
  int value;
  int read() const { return value; }
};
template<class T> struct derived : base<T> {
  using alias = base<T>;
  void reset(derived& other) { other.alias::value = 17; }
  void set(derived* other) { other->alias::value = 23; }
  int read(const derived& other) { return other.alias::read(); }
  int read(const derived* other) { return other->alias::read(); }
  int inferred(derived& other) {
    static_assert(sizeof(decltype(other.alias::value)) == sizeof(int), "qualified field type");
    return other.alias::value;
  }
};
template<class T> struct derived<T*> : base<T*> {
  typedef base<T*> alias;
  int f(derived& other) { other.alias::value = 29; return other.alias::value; }
};
int main() {
  derived<int> a;
  a.reset(a);
  if (a.read(a) != 17 || a.read(&a) != 17 || a.inferred(a) != 17) return 1;
  a.set(&a);
  derived<int*> b;
  return a.value == 23 && b.f(b) == 29 ? 0 : 2;
}
