int target() { return 7; }
struct Descriptor {
  typedef int (*Fn)();
  static constexpr Fn pointer = &target;
};
constexpr Descriptor::Fn Descriptor::pointer;

template<class T, int N> struct Box {
  template<class D> struct Inner {
    static decltype(D::pointer) pointer;
    template<int K> static int run() {
      struct Table { int (*call)(); };
      static const Table table = { &target };
      return table.call() + K;
    }
  };
};
template<class T, int N> template<class D>
decltype(D::pointer) Box<T, N>::Inner<D>::pointer = D::pointer;

int main() {
  return Box<char, 1>::Inner<Descriptor>::pointer() != 7 ||
      Box<char, 1>::Inner<Descriptor>::run<3>() != 10 ||
      Box<long, 2>::Inner<Descriptor>::run<5>() != 12;
}
