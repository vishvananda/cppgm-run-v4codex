struct A{};
int operator+(A,A) noexcept {return 3;}
struct B{};
int operator+(B,B) {return 4;}
template<class... T> constexpr bool safe(){return noexcept((T()+...));}
static_assert(safe<int,int>(),"builtin");
static_assert(safe<A,A>(),"nothrow operator");
static_assert(!safe<B,B>(),"throwing operator");
int main(){return 0;}
