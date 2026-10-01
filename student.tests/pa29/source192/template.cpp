static_assert(__is_same(__float80,long double)&&!__is_same(_Float64x,long double),"x87 aliases");
static_assert(__is_same(decltype(1.0W),long double),"x87 suffix");
constexpr __uint128_t wide=(__uint128_t(1)<<127)+(__uint128_t(1)<<103)+1;
static_assert((float)wide>0x1p127F,"direct integer to binary32 rounding");
constexpr __float128 q=0x1.0000000000000000000000000001p0Q;
template<class T> constexpr T twice(T x){return x+x;}
template<class T> struct Box { T a; T b; constexpr Box(T x,T y):a(x),b(y){} constexpr T sum()const{return a+b;} };
constexpr Box<__float128> b(q,0x1p-112Q);
static_assert(b.sum()==1+0x1p-111Q,"template precision");
static_assert(twice((_Float16)1.5)==3,"half template");
constexpr long double exact{1.0Q};
constexpr _Float16 h{1.5Q};
__float128 a(){const __float128 x[]={0x1p-80Q};return x[0];}
__float128 c(){const __float128 x[]={0x1p-90Q};return x[0];}
__float128 d(){const __uint128_t x[]={__uint128_t(1)<<112};return (__float128)x[0];}
__float128 e(){const __uint128_t x[]={__uint128_t(1)<<113};return (__float128)x[0];}
int main(int argc,char**){
 __float128 x=argc;Box<__float128> r(x,0x1p-112Q);
 if(r.sum()!=q||twice(r.sum())!=2*q)return 1;
 if(a()==c()||a()!=0x1p-80Q||c()!=0x1p-90Q)return 2;
 if(d()==e()||d()!=0x1p112Q||e()!=0x1p113Q)return 3;
 auto fn=&twice<__float128>; if(fn(q)!=2*q)return 4;
 return 0;
}
