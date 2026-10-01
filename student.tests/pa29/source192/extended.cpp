static_assert(sizeof(_Float16)==2 && alignof(_Float16)==2,"half layout");
static_assert(sizeof(__float128)==16 && alignof(__float128)==16,"quad layout");
static_assert(!__is_same(_Float128,__float128),"quad identity");
static_assert(!__is_same(_Float32,float) && !__is_same(_Float64,double) && !__is_same(_Float32x,double),"width identities");
static_assert(!__is_same(_Float16,float) && !__is_same(__float128,long double),"identities");
constexpr __float128 q=0x1.0000000000000000000000000001p0Q;
static_assert(q>1 && q-1==0x1p-112Q,"113 bits");
static_assert((q+0x1p-113Q)-1==0x1p-111Q,"quad ties even");
static_assert(0x1p-16494Q>0 && 0x1p-16495Q==0,"quad subnormals");
static_assert(0x1.8p-16494Q==0x1p-16493Q,"subnormal tie");
static_assert((_Float16)1.00048828125F16==1 && (_Float16)1.00146484375F16==(_Float16)1.001953125F16,"half ties");
static_assert((_Float16)0x1p-24F16>0 && (_Float16)0x1p-25F16==0,"half subnormals");
static_assert(__builtin_bit_cast(unsigned short,-0.0F16)==32768,"half zero bits");
static_assert(__builtin_bit_cast(__uint128_t,q)==((__uint128_t(16383)<<112)|1),"quad bits");
__float128 data[]={q,-0.0Q,0x1p-16494Q,3.5Q};
_Float16 halves[]={1.5F16,-0.0F16,0x1p-24F16};
__float128 add(__float128 x,__float128 y){return x+y;}
__float128 mul(__float128 x,__float128 y){return x*y;}
__float128 div(__float128 x,__float128 y){return x/y;}
_Float16 hadd(_Float16 x,_Float16 y){return x+y;}
_Float16 hmul(_Float16 x,_Float16 y){return x*y;}
int main(int argc,char**){
  volatile __float128 one=argc;
  __float128 x=add(one,0x1p-112Q);
  if(x!=q || mul(x,2)!=2*q || div(x,2)!=q/2) return 1;
  if(data[0]!=q || data[2]!=0x1p-16494Q || data[3]!=3.5Q) return 2;
  if(__builtin_bit_cast(__uint128_t,data[1])!=(__uint128_t(1)<<127)) return 3;
  volatile _Float16 h=halves[0];
  if(hadd(h,0.5F16)!=2 || hmul(h,2)!=3) return 4;
  if((__float128)h!=1.5Q || (_Float16)x!=1 || (int)x!=1) return 5;
  if((long double)x!=1 || (__float128)(long double)x==x) return 6;
  if(__builtin_bit_cast(unsigned short,halves[1])!=32768 || halves[2]!=0x1p-24F16) return 7;
  return 0;
}
