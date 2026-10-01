__float128 qadd(__float128 a,__float128 b){return a+b;}
__float128 qsub(__float128 a,__float128 b){return a-b;}
__float128 qmul(__float128 a,__float128 b){return a*b;}
__float128 qdiv(__float128 a,__float128 b){return a/b;}
__float128 qneg(__float128 a){return -a;}
int qcmp(__float128 a,__float128 b){return (a==b)|((a!=b)<<1)|((a<b)<<2)|((a<=b)<<3)|((a>b)<<4)|((a>=b)<<5);}
int qclass(__float128 a){return __builtin_isfinite(a)|(__builtin_isnan(a)<<1)|(__builtin_isinf(a)<<2)|(__builtin_isnormal(a)<<3)|(__builtin_signbit(a)<<4);}
_Float16 hneg(_Float16 a){return -a;}
_Float16 hadd(_Float16 a,_Float16 b){return a+b;}
_Float16 hsub(_Float16 a,_Float16 b){return a-b;}
_Float16 hmul(_Float16 a,_Float16 b){return a*b;}
_Float16 hdiv(_Float16 a,_Float16 b){return a/b;}
int hcmp(_Float16 a,_Float16 b){return (a==b)|((a!=b)<<1)|((a<b)<<2)|((a<=b)<<3)|((a>b)<<4)|((a>=b)<<5);}
int hclass(_Float16 a){return __builtin_isfinite(a)|(__builtin_isnan(a)<<1)|(__builtin_isinf(a)<<2)|(__builtin_isnormal(a)<<3)|(__builtin_signbit(a)<<4);}
#define CONVERT(T,N) __float128 qfrom##N(T a){return a;} T qto##N(__float128 a){return a;} _Float16 hfrom##N(T a){return a;} T hto##N(_Float16 a){return a;}
CONVERT(float,f) CONVERT(double,d) CONVERT(long double,l) CONVERT(int,i) CONVERT(unsigned,u) CONVERT(long long,ll) CONVERT(unsigned long long,ull) CONVERT(__int128,si) CONVERT(unsigned __int128,ui)
__float128 qfromh(_Float16 a){return a;} _Float16 hfromq(__float128 a){return a;}
__float128 qmany(__float128 a,__float128 b,__float128 c,__float128 d,__float128 e,__float128 f,__float128 g,__float128 h,__float128 i,__float128 j){return a+b+c+d+e+f+g+h+i+j;}
_Float16 hmany(_Float16 a,_Float16 b,_Float16 c,_Float16 d,_Float16 e,_Float16 f,_Float16 g,_Float16 h,_Float16 i,_Float16 j){return a+b+c+d+e+f+g+h+i+j;}
__float128 qvar(int count,...){__builtin_va_list a;__builtin_va_start(a,count);__float128 r=0;for(int i=0;i<count;++i)r+=__builtin_va_arg(a,__float128);__builtin_va_end(a);return r;}
_Float16 hvar(int count,...){__builtin_va_list a;__builtin_va_start(a,count);_Float16 r=0;for(int i=0;i<count;++i)r+=__builtin_va_arg(a,_Float16);__builtin_va_end(a);return r;}
extern __float128 hostq(__float128,__float128,__float128,__float128,__float128,__float128,__float128,__float128,__float128,__float128);
extern _Float16 hosth(_Float16,_Float16,_Float16,_Float16,_Float16,_Float16,_Float16,_Float16,_Float16,_Float16);
__float128 qcall(__float128 a){return hostq(a,2,3,4,5,6,7,8,9,10)+a;}
_Float16 hcall(_Float16 a){return hosth(a,2,3,4,5,6,7,8,9,10)+a;}
_Float32 n32(_Float32 x){return x+1.0F32;}
_Float64 n64(_Float64 x){return x+1.0F64;}
_Float32x n32x(_Float32x x){return x+1.0F32x;}
_Float64x n64x(_Float64x x){return x+1.0F64x;}
_Float128 n128(_Float128 x){return x+1.0F128;}
