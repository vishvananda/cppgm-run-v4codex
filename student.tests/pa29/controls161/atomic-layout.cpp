struct Pair { unsigned long a,b; };
struct Small { _Atomic(bool) flag; constexpr Small(bool v):flag(v) {} };
template<class T> struct Box { _Atomic(T) value; };
static_assert(alignof(Box<Pair>)==16 && sizeof(Box<Pair>)==16,"wide alignment");
static_assert(__is_literal_type(Small),"atomic literal subobject");
static_assert(__is_same(decltype(*(_Atomic(int)*)0),_Atomic(int)&),"typed storage");
int choose(int*){return 1;} int choose(_Atomic(int)*){return 2;}
int main(){Small a(true);Box<int> b{};int plain=0;b.value=4;
 int& alias=plain;alias=6;
 return a.flag && b.value==4 && choose(&b.value)==2 && choose(&plain)==1?0:1;}
