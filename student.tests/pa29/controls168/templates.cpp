struct S { int a; int b; };
template<class T> struct Pair { T a; T b; };
template<class T> Pair<T> make(T x){return (Pair<T>){.b=x};}
template<int N> constexpr S fixed(){return (S){.b=N};}
template<class T> S ordinary(T x){S s={.b=x};return s;}
template<class T> S retained(){return (S){.b=7};}
static_assert(fixed<19>().a==0 && fixed<19>().b==19,"dependent value");
int main(){auto a=make(5);return a.a!=0 || a.b!=5 || ordinary(8).b!=8 || retained<int>().b!=7 || retained<long>().b!=7;}
