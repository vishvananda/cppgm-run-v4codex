// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
struct P{int n;constexpr P(int n):n(n){}constexpr P operator-()const{return P(-n);}};constexpr P p(3);extern const P q;int read(){return q.n;}int seen=read();constexpr P q=-p;int main(){return seen==-3?0:1;}
