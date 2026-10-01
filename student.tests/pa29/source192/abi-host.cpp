#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <quadmath.h>
#define DECLARE(P,T) T P##add(T,T);T P##sub(T,T);T P##mul(T,T);T P##div(T,T);T P##neg(T);int P##cmp(T,T);int P##class(T);T P##many(T,T,T,T,T,T,T,T,T,T);T P##var(int,...);T P##call(T);
DECLARE(q,__float128) DECLARE(h,_Float16)
#define CONVERT(T,N) __float128 qfrom##N(T);T qto##N(__float128);_Float16 hfrom##N(T);T hto##N(_Float16);
CONVERT(float,f) CONVERT(double,d) CONVERT(long double,l) CONVERT(int,i) CONVERT(unsigned,u) CONVERT(long long,ll) CONVERT(unsigned long long,ull) CONVERT(__int128,si) CONVERT(unsigned __int128,ui)
__float128 qfromh(_Float16);_Float16 hfromq(__float128);
__float128 hostq(__float128 a,__float128 b,__float128 c,__float128 d,__float128 e,__float128 f,__float128 g,__float128 h,__float128 i,__float128 j){return a+b+c+d+e+f+g+h+i+j;}
_Float16 hosth(_Float16 a,_Float16 b,_Float16 c,_Float16 d,_Float16 e,_Float16 f,_Float16 g,_Float16 h,_Float16 i,_Float16 j){return a+b+c+d+e+f+g+h+i+j;}
#define CHECK(x) do{if(!(x)){std::printf("line %d: %s\n",__LINE__,#x);return 1;}}while(0)
template<class T> int cmp(T a,T b){return (a==b)|((a!=b)<<1)|((a<b)<<2)|((a<=b)<<3)|((a>b)<<4)|((a>=b)<<5);}
bool same(__float128 a,__float128 b){return a==b||(isnanq(a)&&isnanq(b));}
bool same(_Float16 a,_Float16 b){return a==b||(std::isnan((float)a)&&std::isnan((float)b));}
_Float32 n32(_Float32); _Float64 n64(_Float64); _Float32x n32x(_Float32x); _Float64x n64x(_Float64x); _Float128 n128(_Float128);
int main(){
 CHECK(n32(2)==3); CHECK(n64(2)==3); CHECK(n32x(2)==3); CHECK(n64x(2)==3); CHECK(n128(2)==3);
 __float128 q[]={0.0Q,-0.0Q,1.0Q,-1.0Q,0x1.0000000000000000000000000001p0Q,0x1p-16494Q,0x1p-16382Q,HUGE_VALQ,-HUGE_VALQ,nanq("")};
 _Float16 h[]={(_Float16)0,(_Float16)-0.0,(_Float16)1,(_Float16)-1,(_Float16)0x1p-24,(_Float16)0x1p-14,(_Float16)INFINITY,(_Float16)-INFINITY,(_Float16)NAN};
 for(auto a:q){CHECK(qclass(a)==((!isinfq(a)&&!isnanq(a))|(isnanq(a)<<1)|(bool(isinfq(a))<<2)|((!isinfq(a)&&!isnanq(a)&&fabsq(a)>=0x1p-16382Q)<<3)|(bool(signbitq(a))<<4)));CHECK(same(qneg(a),-a));for(auto b:q){CHECK(qcmp(a,b)==cmp(a,b));CHECK(same(qadd(a,b),a+b));CHECK(same(qsub(a,b),a-b));CHECK(same(qmul(a,b),a*b));CHECK(same(qdiv(a,b),a/b));}}
 for(auto a:h){CHECK(hclass(a)==(std::isfinite((float)a)|(std::isnan((float)a)<<1)|(std::isinf((float)a)<<2)|((std::isfinite((float)a)&&std::fabs((float)a)>=0x1p-14)<<3)|(std::signbit((float)a)<<4)));CHECK(same(hneg(a),(_Float16)-a));for(auto b:h){CHECK(hcmp(a,b)==cmp(a,b));CHECK(same(hadd(a,b),(_Float16)(a+b)));CHECK(same(hsub(a,b),(_Float16)(a-b)));CHECK(same(hmul(a,b),(_Float16)(a*b)));CHECK(same(hdiv(a,b),(_Float16)(a/b)));}}
 CHECK(qmany(1,2,3,4,5,6,7,8,9,10)==55);CHECK(hmany(1,2,3,4,5,6,7,8,9,10)==55);
 CHECK(qvar(10,1.Q,2.Q,3.Q,4.Q,5.Q,6.Q,7.Q,8.Q,9.Q,10.Q)==55);
 CHECK(hvar(10,(_Float16)1,(_Float16)2,(_Float16)3,(_Float16)4,(_Float16)5,(_Float16)6,(_Float16)7,(_Float16)8,(_Float16)9,(_Float16)10)==55);
 CHECK(qcall(1)==56);CHECK(hcall(1)==56);
#define TEST(N,T,V) CHECK(qfrom##N((T)(V))==(__float128)(T)(V));CHECK(qto##N((__float128)(T)(V))==(T)(V));CHECK(hfrom##N((T)(V))==(_Float16)(T)(V));CHECK(hto##N((_Float16)123)==(T)123)
 TEST(f,float,1.5);TEST(d,double,1.5);TEST(l,long double,1.5);TEST(i,int,-123);TEST(u,unsigned,123);TEST(ll,long long,-123);TEST(ull,unsigned long long,123);TEST(si,__int128,-123);TEST(ui,unsigned __int128,123);
 CHECK(qfromui(((unsigned __int128)1<<112)|1)==q[4]*0x1p112Q);CHECK(qtoull(0x1p63Q)==(1ull<<63));CHECK(qtosi(-0x1p100Q)==-((__int128)1<<100));CHECK(qtoui(0x1p127Q)==((unsigned __int128)1<<127));
 CHECK(qfromh((_Float16)1.5)==1.5Q);CHECK(hfromq(1.0004882812500000000001Q)==(_Float16)1.0009765625);
 return 0;
}
