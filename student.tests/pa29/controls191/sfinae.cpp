template<class T> auto f(T x)->decltype(__builtin_bit_cast(unsigned,x)){return __builtin_bit_cast(unsigned,x);}
int f(...){return 9;}
static_assert(__is_same(decltype(f(1.0f)),unsigned),"valid");
static_assert(__is_same(decltype(f(1.0)),int),"invalid size substitution");
int main(){return f(1.0)==9 && f(1.0f)==0x3f800000 ? 0:1;}
