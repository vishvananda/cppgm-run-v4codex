struct A { protected: int x; void f(){} };
struct D:A {
 template<class T,int T::*> struct X {};
 template<class T> static char test(X<T,&T::x>*);
 template<class T> static long test(...);
 static_assert(sizeof(test<A>(0))==sizeof(long),"base-qualified protected address");
 static int D::* good(){return &D::x;}
};
struct R { int& x; };
template<class T> auto ref(int)->decltype(&T::x);
template<class T> long ref(...);
static_assert(sizeof(ref<R>(0))==sizeof(long),"member of reference rejected in substitution");
int main(){return D::good()==nullptr;}
