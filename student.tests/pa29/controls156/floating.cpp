double static_signaling=__builtin_nans("");
int sign(double x) noexcept {return __builtin_signbit(x);}
int main(int argc,char**) {
  volatile double zero=argc-1;
  if(sign(zero) || !sign(-zero))return 1;
  volatile float f=-__builtin_nanf("");
  volatile long double l=-0.0L;
  if(!__builtin_signbitf(f) || !__builtin_signbitl(l))return 2;
  volatile double inf=__builtin_inf(), nan=__builtin_nan("");
  if(__builtin_isfinite(inf) || __builtin_isnormal(nan) || !__builtin_isnan(nan))return 3;
  double source=__builtin_nans(""); unsigned long long bits=0;
  __builtin_memcpy(&bits,&source,8);
  if(bits!=0x7ff4000000000000ULL)return 4;
  __builtin_memcpy(&bits,&static_signaling,8);
  if(bits!=0x7ff4000000000000ULL)return 5;
  if(__builtin_isgreater(nan,zero) || __builtin_isless(nan,zero) || __builtin_islessgreater(nan,zero))return 6;
  if(!__builtin_isunordered(nan,zero) || !__builtin_isgreaterequal(inf,zero))return 7;
  return 0;
}
constexpr int negative(double x) {return __builtin_signbit(x);}
static_assert(negative(-0.0)==1,"constant activation sign bit");
static_assert(__builtin_isgreater(4,2.0),"mixed comparison");
static_assert(!__builtin_islessgreater(__builtin_nan(""),2.0),"unordered");
