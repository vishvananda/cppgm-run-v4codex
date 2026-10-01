struct Pair{int a,b;Pair(){}Pair(const Pair&)=delete;};template<class T>int unused(Pair p){auto [a,b]=p;return a+b;}int main(){}
