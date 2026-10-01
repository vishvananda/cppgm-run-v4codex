using C = _Complex double;
constexpr C z=__builtin_complex(2.0,3.0);
constexpr C w=__builtin_complex(4.0,-5.0);
constexpr C sum=z+w;
constexpr C product=z*w;
constexpr C quotient=product/w;
static_assert(__real__ sum==6 && __imag__ sum==-2,"sum");
static_assert(__real__ product==23 && __imag__ product==2,"product");
static_assert(__real__ quotient==2 && __imag__ quotient==3,"quotient");
static_assert(__real__ (-z)==-2 && __imag__ (~z)==-3,"negation and conjugation");
template<class T> constexpr auto re(T x)->decltype(+__real__ x){return __real__ x;}
static_assert(re(z)==2,"dependent constant component");
constexpr const double* p=&__imag__ z;
static_assert(*p==3,"constant component address");
struct S { C x; };
constexpr S s={z};
static_assert(__imag__ s.x==3,"nested constant");
C array[2]={z,w};
S stored={product};
int main(){return __real__ stored.x==23 && __imag__ array[1]==-5 && *p==3 ? 0:1;}
