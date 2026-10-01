using U = unsigned _BitInt(93);
using S = _BitInt(93);
constexpr U high = U(1)<<92;
static_assert(high / 2 == U(1)<<91,"wide quotient");
static_assert(S(high)==-S(high-1)-1,"sign extension");
static_assert(U(-1)==(high-1)*2+1,"mask");
U add(U a,U b) { return a+b; }
S divide(S a,S b) { return a/b; }
struct P { char c; U u; char tail; };
static_assert(sizeof(P)==32 && alignof(P)==8,"aggregate layout");
int main() {
 volatile int seed=92;
 U x=U(1)<<seed;
 if (add(x,x)!=0 || divide(S(x),2)!=-S(x/2)) return 1;
 U y=(x-17)/U(9);
 if (y*9+(x-17)%9 != x-17) return 2;
 P p={2,x,3}; if(p.u!=x || p.tail!=3 || p.c!=2) return 3;
 double d = U(12345); if (U(d)!=12345) return 4;
 return 0;
}
