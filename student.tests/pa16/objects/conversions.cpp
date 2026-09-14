struct C {
 int n;
 constexpr C(int v):n(v){}
 friend constexpr C operator+(C const&a,C const&b){return C(a.n+b.n);}
};
constexpr C a(1);
constexpr C b=2;
static_assert(a.n==1, "direct");
static_assert(b.n==2, "conversion");
constexpr C add(C const&a,C const&b){return C(a.n+b.n);}
static_assert(add(a,b).n==3,"free");
static_assert((a+b).n==3,"operator");
static_assert((a+2).n==3,"reference conversion");
int main(){return 0;}
