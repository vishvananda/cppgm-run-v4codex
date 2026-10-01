struct Z{int a[0];};constexpr Z z={};template<class T> constexpr unsigned n(){return sizeof(T);}static_assert(n<Z>()==0,"query");int main(){return sizeof(z);}
