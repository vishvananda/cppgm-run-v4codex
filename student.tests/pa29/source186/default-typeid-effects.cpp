#include <typeinfo>
int calls=0;
template<class T> T side(){++calls;return T();}
template<class T> int f(T p, const std::type_info& ti=typeid(side<T>())) {return ti==typeid(T);}
struct Plain {Plain(){++calls;}};
struct Poly { virtual ~Poly(){} };Poly global;
template<class T> T& dynamic(int){++calls;return global;}
template<class T> int g(T& p,const std::type_info& ti=typeid(dynamic<T>(sizeof(p)))) {return ti==typeid(T);}
int main(){if(!f(1)||calls!=0)return 1;return !g(global)||calls!=1;}
