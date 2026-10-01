using F = _Complex float;
using D = __complex__ double;
using L = __complex long double;
static_assert(sizeof(F)==8 && alignof(F)==4,"float layout");
static_assert(sizeof(D)==16 && alignof(D)==8,"double layout");
static_assert(sizeof(L)==32 && alignof(L)==16,"extended layout");
static_assert(!__is_same(F,D) && !__is_same(D,L),"distinct types");
constexpr D c = __builtin_complex(2.0,-3.0);
static_assert(__real__ c==2.0 && __imag__ c==-3.0,"components");
template<class T> auto real(T& x) -> decltype((__real__ x)) { return __real__ x; }
template<class T> auto imag(T& x) -> decltype((__imag__ x)) { return __imag__ x; }
template<class T> struct Holder { T x; explicit Holder(T v):x(v){} T get() const {return x;} };
F f(F x) { return x; }
D d(D x) { return x; }
L l(L x) { return x; }
D add(D x,D y) { return x+y; }
D mul(D x,D y) { return x*y; }
D div(D x,D y) { return x/y; }
D global = __builtin_complex(1.0,2.0);
int main(int argc,char**) {
  F x=__builtin_complex(float(argc),2.0f);
  D y=__builtin_complex(3.0,4.0);
  L z=__builtin_complex(5.0L,6.0L);
  Holder<F> h(x);
  if (__real__ f(h.get())!=argc || __imag__ f(x)!=2) return 1;
  if (__real__ d(y)!=3 || __imag__ d(y)!=4) return 2;
  if (__real__ l(z)!=5 || __imag__ l(z)!=6) return 3;
  real(y)=7; imag(y)=8;
  if (__real__ y!=7 || __imag__ y!=8) return 4;
  D a=add(y,global), b=mul(global,global), q=div(b,global);
  if (__real__ a!=8 || __imag__ a!=10) return 5;
  if (__real__ b!=-3 || __imag__ b!=4) return 6;
  if (__real__ q!=1 || __imag__ q!=2) return 7;
  D zero{}; if (zero || !y) return 8;
  D promoted=x; if (__real__ promoted!=argc || __imag__ promoted!=2) return 9;
  return 0;
}
