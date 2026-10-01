template<class...T> auto f(int)->decltype((T()+...)){return (T()+...);}
template<class...T> long f(...){return 17;}
struct A{};
static_assert(__is_same(decltype(f<int,int>(0)),int),"good");
static_assert(__is_same(decltype(f<A,A>(0)),long),"bad operands");
static_assert(__is_same(decltype(f<>(0)),long),"empty plus");
int main(){return f<A,A>(0)!=17 || f<>(0)!=17;}
