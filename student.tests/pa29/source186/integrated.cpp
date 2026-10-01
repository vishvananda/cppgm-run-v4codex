#include <typeinfo>
template<int N> struct Box {unsigned _BitInt(N) value;};
template<int N> Box(unsigned _BitInt(N))->Box<N>;
struct Pad {long n;};struct Base {unsigned _BitInt(93) v;};struct Derived:Pad,Base {};
template<int N> int step(const Box<N>& box,int n=decltype(N)(3), const std::type_info& ti=typeid(box)) {
 unsigned _BitInt(N) r=0;bool over=__builtin_add_overflow(box.value,static_cast<unsigned _BitInt(N)>(n),&r);
 return over&&r==2&&ti==typeid(Box<N>);
}
int main(){Box<7> a={127};Box<93> b={static_cast<unsigned _BitInt(93)>(-1)};
 Derived d;d.v=7;const Derived& cd=d; Base& br=(Base&)cd;
 return !step(a)||!step(b)||br.v!=7;}
