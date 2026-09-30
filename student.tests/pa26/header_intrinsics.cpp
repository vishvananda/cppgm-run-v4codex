extern "C" int strcmp(const char*,const char*);
const char* ordinary(){static_assert(sizeof(__func__)==9,"name length");static_assert(__func__[0]=='o',"constant name");return __func__;}
template<class T> const char* named(T){return __func__;}
struct Address { int value; int* operator&(){return 0;} };
template<class T> T* raw_address(T& v){return __builtin_addressof(v);}
template<class T> auto expected(T x) -> decltype(__builtin_expect(T(),1)) {return __builtin_expect(x,1);}
static_assert(__is_same(decltype(__builtin_expect(1,1)),long),"builtin query result");
static_assert(noexcept(__builtin_expect(1,1)),"builtin query exception");
int may_throw();
static_assert(noexcept(__builtin_constant_p(may_throw())),"constant query is unevaluated");
static_assert(noexcept(__builtin_abort()),"abort cannot throw");
static_assert(noexcept(__builtin_unreachable()),"unreachable cannot throw");
static_assert(__is_same(decltype(__builtin_constant_p(1)),int),"constant query type");
template<int N> struct Expected {static const long value=__builtin_expect(N,1);};
static_assert(Expected<7>::value==7,"constant builtin query");
int main(){Address v;v.value=7; const Address& c=v;
static_assert(__is_same(decltype(__builtin_addressof(c)),const Address*),"address cv");
return strcmp(ordinary(),"ordinary") || strcmp(named(1),"named") || strcmp(named(1.),"named") ||
    raw_address(v)->value!=7 || expected(7)!=7 || __builtin_labs(-4000000000L)!=4000000000L || __builtin_llabs(-5000000000LL)!=5000000000LL || __builtin_fabs(-2.5)!=2.5 || __builtin_fabsf(-3.5f)!=3.5f || __builtin_fabsl(-4.5L)!=4.5L;}
