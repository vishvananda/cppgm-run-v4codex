// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
struct P{int n;constexpr P(int n):n(n){}};constexpr int square(int n){return n*n;}extern const P p;int read(){return p.n;}int seen=read();constexpr P p(square(3));int main(){return seen==9?0:1;}
