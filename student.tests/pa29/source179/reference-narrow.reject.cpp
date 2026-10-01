struct Pair { int a; const int& b; };
template<int N> int work(long x) { Pair p{N,x}; auto [a,b]=p; return a+b; }
int main() { return work<2>(4000000000L); }
