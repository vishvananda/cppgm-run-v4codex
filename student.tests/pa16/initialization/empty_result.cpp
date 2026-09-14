// N3485: 12.4 [class.dtor]/5; Itanium ABI 3.1.3.1.
template<class T>struct G{G(){}~G(){}};template<class T>constexpr G<T>make(T){return G<T>();}int main(){make(1);}
