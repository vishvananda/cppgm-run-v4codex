#include "abi.h"
extern "C" D host_step(D x){__real__ x+=1;__imag__ x-=1;return x;}
extern "C" L host_wide(L x){__real__ x+=1;__imag__ x-=1;return x;}
int main(){
  F f; __real__ f=1.25f;__imag__ f=-2.5f;
  D d; __real__ d=3.25;__imag__ d=-4.5;
  L l; __real__ l=5.25L;__imag__ l=-6.5L;
  F fr=f_echo(f); if(__real__ fr!=1.25f||__imag__ fr!=-2.5f)return 1;
  D dr=d_many(d,d,d,d,d,d,7,8.5);if(__real__ dr!=26.5||__imag__ dr!=-18.5)return 2;
  L lr=l_echo(l);if(__real__ lr!=5.25L||__imag__ lr!=-6.5L)return 3;
  dr=call_host(d);if(__real__ dr!=4.25||__imag__ dr!=-5.5)return 4;
  lr=call_wide(l);if(__real__ lr!=6.25L||__imag__ lr!=-7.5L)return 5;
  return ignore_wide()?6:0;
}
