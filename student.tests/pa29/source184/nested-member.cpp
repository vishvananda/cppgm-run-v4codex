struct A {int x;};
constexpr const int A::*member=&A::x;
constexpr const int A::* const *p=&member;
constexpr int A::*const *q=(int A::*const*)p;
constexpr A a={31};
static_assert(a.**q==31,"mixed member/object pointers");
int main(){A b={5}; b.**q=19; return b.x==19?0:1;}
