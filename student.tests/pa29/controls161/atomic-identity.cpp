int overload(int x){return x;}
int overload(_Atomic(int) x){return x+1;}
int distinct(_Atomic(int) x){return x;}
static_assert(__is_same(__remove_cv(const _Atomic(int)),_Atomic(int)),"atomic is not cv");
template<class T> struct Value { enum {which=1}; };
template<class T> struct Value<_Atomic(T)> { enum {which=2}; };
static_assert(Value<int>::which==1 && Value<_Atomic(int)>::which==2,"template identity");
int main(){_Atomic(int) a=2;return overload(3)==3 && distinct(a)==2?0:1;}
