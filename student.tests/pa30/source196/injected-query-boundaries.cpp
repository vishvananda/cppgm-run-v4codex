template<class A, class B> struct same { static const bool value = false; };
template<class A> struct same<A,A> { static const bool value = true; };
template<class T> struct box {
  T value;
  box() : value(13) {}
  int get() const { return value; }
  box self() { return box(); }
  box<long> different() { return box<long>(); }
  int braced() { return box{}.get(); }
  int qualified() { return box<T>().get(); }
  struct inner {
    int get() { return box().get(); }
  };
};
template<class T> struct outer {
  struct inner {
    int get() const { return 19; }
    int reset() { return inner().get(); }
  };
};
static_assert(same<decltype(box<int>().self()),box<int>>::value, "injected current instantiation");
static_assert(same<decltype(box<int>().different()),box<long>>::value, "explicit specialization");
int main() {
  box<int> b; box<int>::inner i; outer<int>::inner n;
  return b.self().get() == 13 && b.different().get() == 13 &&
    b.braced() == 13 && b.qualified() == 13 && i.get() == 13 && n.reset() == 19 ? 0 : 1;
}
