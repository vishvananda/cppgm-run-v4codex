#include "abi.h"
extern "C" C hc(C x){return x;}
extern "C" B hb(B x){return x;}
extern "C" S hs(S x){return x;}
extern "C" I hi(I x){return x;}
int main(){C c{3}; B b{true,false,true,false}; S s{1,2,3,4}; I i{5,6,7,8};
 return native_test() && __builtin_reduce_or(nc(c)!=c)==0 &&
 __builtin_bit_cast(unsigned char,nb(b))==5 &&
 __builtin_reduce_or(ns(s)!=s)==0 && __builtin_reduce_or(ni(i)!=i)==0 ? 0:1;}
