struct A{int n;constexpr int get(int x) const {return n+x;}};
template<class...T> constexpr int read(const A& a,T...p){return (a.*... .*p)(2);}
static_assert(read(A{3},&A::get)==5,"constexpr fold callable");
int main(){A a={7};return read(a,&A::get)-9;}
