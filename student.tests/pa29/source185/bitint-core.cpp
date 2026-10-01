template<class A, class B> struct same { static const bool value = false; };
template<class A> struct same<A,A> { static const bool value = true; };
using S = _BitInt(7);
using U = unsigned _BitInt(7);
static_assert(sizeof(S)==1 && alignof(S)==1,"layout");
static_assert(sizeof(_BitInt(65))==16 && alignof(_BitInt(65))==8,"wide layout");
static_assert(!same<S,signed char>::value && !same<S,_BitInt(8)>::value,"identity");
static_assert(same<decltype(S(2)+S(3)),S>::value,"no promotion");
static_assert(same<decltype(S(2)+1),int>::value,"ordinary rank");
static_assert(same<decltype((_BitInt(32))2+1),int>::value,"equal-width rank");
static_assert(same<decltype((_BitInt(33))2+1),_BitInt(33)>::value,"precision rank");
static_assert((U(127)+U(1))==0,"wrap");
static_assert(S(127)==-1,"signed truncation");
static_assert((S(-5)/S(2))==-2 && (S(-5)%S(2))==-1,"signed division");
static_assert(__is_integral(S) && __is_signed(S) && __is_unsigned(U),"traits");
U add(U a,U b) { return a+b; }
S sub(S a,S b) { return a-b; }
int main() {
  volatile int seed=125;
  U x=seed;
  S y=-5;
  if (add(x,3)!=0 || sub(y,2)!=-7) return 1;
  if (U(~x)!=2 || S(~y)!=4 || -y!=5) return 2;
  if ((x<<2)!=116 || (y>>1)!=-3) return 3;
  ++x; x+=2; if(x!=0) return 4;
  return 0;
}
