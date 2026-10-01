#include "abi.h"
extern "C" F f_echo(F x){return x;}
extern "C" D d_many(D a,D b,D c,D d,D e,D f,int i,double j) {
  return __builtin_complex(__real__ a+__real__ b+__real__ c+__real__ d+__real__ e+__real__ f+i,
                           __imag__ a+__imag__ b+__imag__ c+__imag__ d+__imag__ e+__imag__ f+j);
}
extern "C" L l_echo(L x){return x;}
extern "C" D call_host(D x){return host_step(x);}
extern "C" L call_wide(L x){return host_wide(x);}
extern "C" int ignore_wide(){
  for(int i=0;i<20;++i) host_wide(__builtin_complex(1.0L,2.0L));
  L x=host_wide(__builtin_complex(3.0L,4.0L));
  return __real__ x==4 && __imag__ x==3 ? 0 : 1;
}
