#include <typeinfo>
template<class T> int f(T p, int v=decltype(p)(9), const std::type_info& ti=typeid(p)) {
 return v==9 && ti==typeid(T);
}
template<class T> int parens(T p, const std::type_info& ti=typeid((p))) { return ti==typeid(T); }
template<class T> int arrays(T (&p)[3], const std::type_info& ti=typeid(p)) { return ti==typeid(T[3]); }
template<class T> int casts(T p, int v=static_cast<decltype(p)>(11)) { return v; }
template<class T> struct Box { template<class U> static int test(U p, int n=decltype(p)(sizeof(T))) {return n;} };
struct Plain {int v;};
int main(){int a[3];return !f(1)||!f(1L)||!parens(Plain{2})||!arrays(a)||casts(1)!=11||Box<long>::test(1)!=sizeof(long);}
