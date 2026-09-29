template<class T> struct A {};
struct D:A<int>,A<char> { typedef A owner; };
int main(){return 0;}
