// Pending PA23 virtual-base identity/layout; observed in loop 68, not an exit control.
template<class T>struct A{};struct B:virtual A<int>{};struct C:virtual A<int>{};struct D:B,C{};template<class T>int f(A<T>&){return sizeof(T);}int main(){D d;return f(d)!=sizeof(int);}
