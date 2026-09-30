constexpr double q=__builtin_nan("0x123");
constexpr float qf=__builtin_nanf("");
constexpr long double ql=__builtin_nanl("");
static_assert(__builtin_isnan(q),"constant nan");
static_assert(__builtin_isnan(-q+1.0),"propagated nan");
static_assert(__builtin_isinf(-__builtin_infl()),"negative infinity");
static_assert(__builtin_isnan(qf) && __builtin_isnan(ql),"widths");
static_assert(__builtin_isinf(__builtin_inf()),"infinity");
static_assert(!__builtin_isfinite(q) && !__builtin_isnormal(q),"classification");
static_assert(__builtin_fpclassify(10,20,30,40,50,0.0)==50,"zero");
static_assert(noexcept(__builtin_isnan(0.0)),"intrinsic effects");
template<class T> int test(T x) { return __builtin_isnan(x); }
int calls=0;
long double next(){++calls;return __builtin_nanl("9");}
int main(int argc,char**) {
  volatile double value=q;
  volatile float small=qf;
  volatile long double large=ql;
  if(!__builtin_isnan(value) || !__builtin_isnanf(small) || !__builtin_isnanl(large))return 1;
  if(__builtin_isnan((double)argc))return 2;
  if(!test(value) || !test(small) || !test(large))return 3;
  if(!__builtin_isnan(next()) || calls!=1)return 4;
  if(__builtin_fpclassify(1,2,3,4,5,value)!=1)return 5;
  return 0;
}
