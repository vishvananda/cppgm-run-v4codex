#include <cstring>
#include <cstdio>
extern "C" _Float16 f16_snan(), f16_nan();
extern "C" float f32_snan(), f32_nan();
extern "C" double f64_snan(), f64_nan();
extern "C" long double f80_snan(), f80_nan();
extern "C" __float128 f128_snan(), f128_nan(), quad_suffixed();
template<class T> bool check(T v,unsigned quiet_byte,unsigned quiet_mask,bool signal,bool negative=false) {
 unsigned char bytes[sizeof(T)];std::memcpy(bytes,&v,sizeof v);
 bool quiet=bytes[quiet_byte]&quiet_mask;
 bool payload=bytes[quiet_byte]&(quiet_mask-1);
 for(unsigned i=0;i<quiet_byte;++i)payload|=bytes[i]!=0;
 bool sign=bytes[sizeof(T)==16 && quiet_byte==7?9:sizeof(T)-1]&128;
 return quiet!=signal && (quiet||payload) && sign==negative;
}
#define GLOBAL(T,K) extern "C" T global_##K##_snan, global_##K##_nan;
GLOBAL(_Float16,f16) GLOBAL(float,f32) GLOBAL(double,f64) GLOBAL(long double,f80) GLOBAL(__float128,f128)
int main(){int failures=0;
#define CHECK(T,B,M) failures+=!check(T##_snan(),B,M,true);failures+=!check(T##_nan(),B,M,false);
 CHECK(f16,1,2) CHECK(f32,2,64) CHECK(f64,6,8) CHECK(f80,7,64) CHECK(f128,13,128)
#undef CHECK
#define CHECK(T,B,M) failures+=!check(global_##T##_snan,B,M,true);failures+=!check(global_##T##_nan,B,M,false);
 CHECK(f16,1,2) CHECK(f32,2,64) CHECK(f64,6,8) CHECK(f80,7,64) CHECK(f128,13,128)
 failures+=!check(quad_suffixed(),13,128,true,true);
 std::printf("failures=%d\n",failures);return failures;
}
