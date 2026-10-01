namespace A {template<class T> struct box {}; box(int)->box<int>;}
namespace B {template<class T> struct box {}; box(int)->box<int>;}
namespace A {box(double)->box<double>;}
int main(){A::box<int> a; B::box<int> b; return sizeof(a)!=sizeof(b);}
