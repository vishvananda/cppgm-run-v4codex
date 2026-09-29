struct A { int x; int f(int v) const { return x+v; } };
template<class T, int T::* P> struct data { static int get(T& t) { return t.*P; } };
template<class T, int T::* P> struct forward : data<T,P> {};
template<class T, int(T::*P)(int)const> int call(T const& t) { return (t.*P)(4); }
template<int A::*P> struct same { static const int value=1; };
template<class T,class U> struct equal { static const bool value=false; };
template<class T> struct equal<T,T> { static const bool value=true; };
static_assert(equal<same<nullptr>,same<static_cast<int A::*>(nullptr)>>::value,"null identity");
static_assert(!equal<same<nullptr>,same<&A::x>>::value,"target identity");
int main() { A a; a.x=3; return forward<A,&A::x>::get(a)!=3 || call<A,&A::f>(a)!=7; }
