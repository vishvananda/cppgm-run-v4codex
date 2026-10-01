#include <typeinfo>
template<class T> int f(T p,const std::type_info& n=typeid(p)){return n==typeid(T);} int main(){return f(2)!=1;}
