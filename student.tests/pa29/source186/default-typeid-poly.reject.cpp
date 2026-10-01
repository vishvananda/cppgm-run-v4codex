#include <typeinfo>
struct S{virtual ~S(){}};template<class T> int f(T& p,const std::type_info& n=typeid(p)){return 0;}int main(){S s;return f(s);}
