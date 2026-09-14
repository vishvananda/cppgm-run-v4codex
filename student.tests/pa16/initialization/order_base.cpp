// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
struct P{int n;constexpr P(int n):n(n){}};struct D:P{constexpr D(P p):P(p){}};constexpr P p(7);extern const D d;int read(){return d.n;}int seen=read();constexpr D d(p);int main(){return seen==7?0:1;}
