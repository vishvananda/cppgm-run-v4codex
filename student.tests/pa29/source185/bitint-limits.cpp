using B = unsigned _BitInt(1);
using U = unsigned _BitInt(128);
using S = _BitInt(128);
static_assert(sizeof(B)==1 && sizeof(S)==16 && alignof(S)==8,"limits layout");
static_assert(B(3)==1 && B(1)+B(1)==0,"one bit wrap");
static_assert(U(-1)+U(1)==0,"128 bit wrap");
static_assert(__is_same(__make_unsigned(_BitInt(19)),unsigned _BitInt(19)),"unsigned transform");
static_assert(__is_same(__make_signed(unsigned _BitInt(19)),_BitInt(19)),"signed transform");
static_assert(__is_same(decltype((_BitInt(7))1+(unsigned _BitInt(7))1),unsigned _BitInt(7)),"same precision");
static_assert(__is_same(decltype((_BitInt(9))1+(unsigned _BitInt(7))1),_BitInt(9)),"larger signed");
static_assert(__is_same(decltype((_BitInt(32))1+1u),unsigned int),"ordinary unsigned");
static_assert(__is_same(decltype((_BitInt(64))1+1L),long),"ordinary signed");
U wrap(U x) { return x+1; }
int main() { volatile int i=1; B b=i; ++b; if(b!=0)return 1; return wrap(U(-1))==0 && S(-1)<0 ? 0 : 2; }
