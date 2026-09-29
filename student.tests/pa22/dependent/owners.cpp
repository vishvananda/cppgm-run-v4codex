struct A { int x; int f(int v) const { return x+v; } };
struct B { int x; int f(int v) const { return x-v; } };
template<class T> struct owner { typedef T type; };
template<class T> struct nested { typedef int owner<T>::type::* type; };
template<class R,class T> int apply(T& t, R T::*p) { return t.*p; }
template<class R,class T,class... Args> R invoke(T const& t,R(T::*p)(Args...)const,Args... args) { return (t.*p)(args...); }
template<class T> struct select { static const int value=0; };
template<class R,class T> struct select<R T::*> { static const int value=1; };
template<class R,class T,class...Args> struct select<R(T::*)(Args...)const> { static const int value=2; };
static_assert(select<int A::*>::value==1,"data");
static_assert(select<int(A::*)(int)const>::value==2,"function");
template<class... T> int owners(int T::*... p) { return sizeof...(T); }
int main(){ A a; a.x=5; B b; b.x=8; nested<A>::type p=&A::x; return owners(&A::x,&B::x)!=2 || apply(a,p)!=5 || apply(b,&B::x)!=8 || invoke(a,&A::f,3)!=8; }
