template<class T> struct A { int x; };
struct D:A<int>,A<char> { typedef A<long> owner; typedef int owner::* pointer; };
int main(){ D::owner x; x.x=9; D::pointer p=&D::owner::x; return x.*p!=9; }
