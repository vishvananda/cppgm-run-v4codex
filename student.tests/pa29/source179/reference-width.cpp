struct Pair { long a; long& b; };
template<int N> long work(long x) {
    Pair p{(x+N)%97,x}; auto [a,b]=p; b+=N%5; return a+b;
}
int main() {
    long x=4000000000L; Pair p{x,x}; auto& [a,b]=p; b+=2;
    return a!=4000000000L || x!=4000000002L || work<3>(4)!=14;
}
