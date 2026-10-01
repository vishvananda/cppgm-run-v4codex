using C=_Complex double;
template<class T> struct condition {
  T value;
  constexpr explicit operator bool() const {return __real__ value>0;}
};
constexpr C value=__builtin_complex(3.0,4.0);
static_assert(condition<C>{value},"converted complex assertion");
template<class T> struct box {
  T value;
  explicit(condition<C>{::value}) box(T v):value(v){}
  int read()const {return co_await();}
  int co_await()const {return __imag__ value;}
  T get()const noexcept(condition<C>{::value}) {return value;}
  void unused(){T::missing();}
};
template<class T> T roundtrip(T x) {
  static_assert(condition<C>{value},"shared fixed conversion");
  try{throw x;}catch(const T& y){return y;}
}
int main(int argc,char**) {
  box<C> b(__builtin_complex(double(argc+2),4.0));
  auto c=roundtrip(b.get());
  return __real__ c==3&&__imag__ c==b.read() ? 0:1;
}
