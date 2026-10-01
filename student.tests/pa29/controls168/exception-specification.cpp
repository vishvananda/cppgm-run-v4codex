struct T{T(int) noexcept(false){}};
struct N{N(int) noexcept{}};
struct A{int tag;T member;};struct B{int tag;N member;};
static_assert(!noexcept((A){.member=1}),"selected throwing conversion");
static_assert(noexcept((B){.member=1}),"selected nonthrowing conversion");
template<class S> constexpr bool check(){return noexcept((S){.member=1});}
static_assert(!check<A>() && check<B>(),"dependent exception demand");
int main(){return 0;}
