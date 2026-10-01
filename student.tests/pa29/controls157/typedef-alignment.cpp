typedef int Over __attribute__((aligned(32)));
typedef long Under __attribute__((aligned(1)));
typedef int Array[8] __attribute__((aligned(32)));
struct S {char c; Under x;};
struct T {char c; Over x;};
static_assert(__is_same(Over,int) && __is_same(Under,long),"attributes do not change semantic identity");
static_assert(alignof(Over)==32 && sizeof(Over)==4,"type alignment is not size");
static_assert(alignof(Under)==1 && sizeof(Under)==8,"decreased typedef alignment");
static_assert(alignof(Array)==32 && sizeof(Array)==32,"array alignment");
static_assert(sizeof(S)==9 && __builtin_offsetof(S,x)==1,"reduced member alignment");
static_assert(sizeof(T)==64 && __builtin_offsetof(T,x)==32,"increased member alignment");
Over global=7;
static_assert(__alignof__(global)==32,"expression object alignment");
template<class U> struct Holder { typedef U Lane __attribute__((aligned(32))); Lane value; };
static_assert(alignof(Holder<int>)==32,"dependent typedef alignment");
template<class U, int N> struct Dependent {
 typedef U Lane __attribute__((aligned(N),aligned(2)));
 Lane value;
};
static_assert(alignof(Dependent<int,64>)==64, "dependent alignment expression");
static_assert(alignof(Dependent<long,1>)==2, "combined dependent minimum");
template<class U> struct Same { static const int n=1; };
static_assert(__is_same(Same<Over>,Same<int>),"canonical specialization key");
int main(){Over local=8; S x={'a',0x12345678}; Array array={};
 return ((unsigned long)&local%32)||((unsigned long)&global%32)||((unsigned long)&array%32)||x.x!=0x12345678;}
