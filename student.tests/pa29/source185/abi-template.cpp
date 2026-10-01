template<int N> _BitInt(N) twice(_BitInt(N) x, _BitInt(N) y) { return x+y; }
template<int N> unsigned _BitInt(N+1) widened(unsigned _BitInt(N) x) { return x; }
template<_BitInt(7) V> int value() { return V; }
int main() { return twice<7>(2,3)==5 && widened<7>(127)==127 && value<-3>()==-3 ? 0 : 1; }
