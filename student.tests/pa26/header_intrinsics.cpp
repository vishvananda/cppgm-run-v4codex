extern "C" int strcmp(const char*,const char*);
const char* ordinary(){static_assert(sizeof(__func__)==9,"name length");static_assert(__func__[0]=='o',"constant name");return __func__;}
template<class T> const char* named(T){return __func__;}
struct Address { int value; int* operator&(){return 0;} };
template<class T> T* raw_address(T& v){return __builtin_addressof(v);}
int main(){Address v;v.value=7; const Address& c=v;
static_assert(__is_same(decltype(__builtin_addressof(c)),const Address*),"address cv");
return strcmp(ordinary(),"ordinary") || strcmp(named(1),"named") || strcmp(named(1.),"named") ||
    raw_address(v)->value!=7 || __builtin_fabs(-2.5)!=2.5 || __builtin_fabsf(-3.5f)!=3.5f || __builtin_fabsl(-4.5L)!=4.5L;}
