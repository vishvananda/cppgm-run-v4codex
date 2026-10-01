struct Pair{int a,b;Pair(){}explicit Pair(const Pair&) {}};template<class T>int f(Pair p){auto [x,y]=p;return x+y;}int main(){}
