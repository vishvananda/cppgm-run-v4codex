using I = __int128;
using U = unsigned __int128;
constexpr U high = U(1) << 100;
constexpr I minus = -I(high);
static_assert(sizeof(I) == 16 && alignof(U) == 16, "layout");
static_assert((high | 17) >> 100 == 1, "wide constant");
static_assert((high | 17) % 13 == 7, "constant remainder");
static_assert((high * 3 + 1) / 3 == high, "constant division");
static_assert((~U(0) >> 64) == 18446744073709551615ULL, "complement");
static_assert(I(-1) < I(0) && !(I(-1) < U(1)) && U(1) < I(-1), "mixed compare");
static_assert((minus >> 100) == -1, "signed right shift");
static_assert(U(-1) > U(1), "sign extension");
static_assert((I(1) << 127) < 0, "signed sign bit");
static_assert((I(1) << 126) + (-(I(1) << 126)) == 0, "cancellation");
constexpr long double floating = (long double)(U(1) << 100);
static_assert(U(floating) == high, "float conversion");
constexpr U values[3] = {high, high+1, high+2};
static_assert(values[U(2)] == high+2, "array projection");
int sized[U(3)];
static_assert(sizeof(sized) == 12, "array bound");
enum class E : U { a=high, b };
static_assert(U(E::b) == high+1, "wide enumerator");
template<U N> U selected() {return N;}
template<> U selected<high+1>() {return high+2;}
U global = high+3;
I negative_global = minus;
struct Pair { U x; I y; } pair = {high+7,minus};
U many(int a,U b,int c,U d,int e,U f) {return a+b+c+d+e+f;}
int choose(U v) { switch(v) {case high: return 1;case high+1:return 2;default:return 3;} }
int main(int argc,char**) {
  volatile int n=-argc;
  I v=n;
  if ((U(v)>>64)!=18446744073709551615ULL || v != -1) return 1;
  if (selected<high>()!=high || selected<high+1>()!=high+2) return 2;
  if (global!=high+3 || negative_global!=minus || pair.x!=high+7 || pair.y!=minus) return 3;
  if (many(argc,high,2,high+1,3,high+2)!=high*3+9) return 4;
  if (choose(high)!=1 || choose(high+1)!=2 || choose(high+2)!=3) return 5;
  return 0;
}
